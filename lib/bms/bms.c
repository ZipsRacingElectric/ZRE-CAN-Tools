// Header
#include "bms.h"

// Includes
#include "cjson/cjson_util.h"

// C Standard Library
#include <errno.h>
#include <stdlib.h>
#include <math.h>

// Calculate the length of the index (primarily used to calculate offset for signal names)
static size_t printIndex (uint16_t index, char* name)
{
	snprintf (name, 4, "%-3i", index);

	if (index < 10)
		return 1;
	else if (index < 100)
		return 2;
	else
		return 3;
}

static size_t printSenseLineIndex (bms_t* bms, uint16_t segmentIndex, uint16_t ltcIndex, uint16_t senseLineIndex, char* name)
{
	// The index to write within the text, note this uses the number of cells per LTC, not the number of sense lines per LTC.
	uint16_t renderIndex = bms->cellsPerLtc * (segmentIndex * bms->ltcsPerSegment + ltcIndex) +  senseLineIndex;

	snprintf (name, 4, "%-3i", renderIndex);

	uint16_t offset;
	if (renderIndex < 10)
		offset = 1;
	else if (renderIndex < 100)
		offset = 2;
	else
		offset = 3;

	// HI suffix
	if (senseLineIndex == 0)
	{
		snprintf (name + offset, 4, "_HI");
		return offset + 3;
	}

	// LO suffix
	if (senseLineIndex == bms->senseLinesPerLtc - 1)
	{
		snprintf (name + offset, 4, "_LO");
		return offset + 3;
	}

	return offset;
}

int bmsInit (bms_t* bms, cJSON* config, canDatabase_t* database)
{
	// Create list to contain signal names to avoid displaying signals redundantly in the bms tui
	size_t signalCount = 0;
	char** signalNames = NULL;
	
	bms->database = database;

	// Get JSON config values

	if (jsonGetUint16_t (config, "segmentCount", &bms->segmentCount) != 0)
		return errno;

	if (jsonGetUint16_t (config, "ltcsPerSegment", &bms->ltcsPerSegment) != 0)
		return errno;

	if (jsonGetUint16_t (config, "cellsPerLtc", &bms->cellsPerLtc) != 0)
		return errno;

	if (jsonGetFloat (config, "minCellVoltage", &bms->minCellVoltage) != 0)
		return errno;

	if (jsonGetFloat (config, "maxCellVoltage", &bms->maxCellVoltage) != 0)
		return errno;

	if (jsonGetFloat (config, "minTemperature", &bms->minTemperature) != 0)
		return errno;

	if (jsonGetFloat (config, "maxTemperature", &bms->maxTemperature) != 0)
		return errno;

	if (jsonGetFloat (config, "maxLtcTemperature", &bms->maxLtcTemperature) != 0)
		return errno;

	bms->senseLinesPerLtc = bms->cellsPerLtc + 1;
	bms->cellCount = bms->segmentCount * bms->ltcsPerSegment * bms->cellsPerLtc;
	bms->senseLineCount = bms->segmentCount * bms->ltcsPerSegment * bms->senseLinesPerLtc;

	// Get database references

	bms->cellVoltageIndices = malloc (sizeof (ssize_t) * bms->cellCount);
	bms->cellsDischargingIndices = malloc (sizeof (ssize_t) * bms->cellCount);

	// Iterate over each cell index in the bms
	for (uint16_t index = 0; index < bms->cellCount; ++index)
	{
		// Get cell voltage signal index
		// DiBacco: signal name is used to locate data from the dbc file
		char voltName [] = "CELL_VOLTAGE_###";
		size_t offset = printIndex (index, voltName + 13);
		voltName [offset + 13] = '\0'; 
		checkSignalRedundancy (voltName, &signalNames, &signalCount); 
		// DiBacco: offset is used to modify the signal name to contain one '#' per digit in the index
		// index < 10 = CELL_VOLTAGE_# | index < 100 = CELL_VOLTAGE_## | etc

		// DiBacco: retreives the index of the specified from the signals array from the database
		bms->cellVoltageIndices [index] = canDatabaseFindSignal (database, voltName);
		if (bms->cellVoltageIndices [index] < 0) // DiBacco: canDatabaseFindSignal returns -1 if not found (return errno which is set in canDatabaseFindSignal)
			return errno;

		// Get cell balancing signal index
		char disName [] = "CELL_BALANCING_###";
		offset = printIndex (index, disName + 15);
		disName [offset + 15] = '\0';
		checkSignalRedundancy (disName, &signalNames, &signalCount); 

		bms->cellsDischargingIndices [index] = canDatabaseFindSignal (database, disName);
		if (bms->cellsDischargingIndices [index] < 0)
			return errno;
	}

	bms->senseLineTemperatureIndices = malloc (sizeof (ssize_t) * bms->senseLineCount);
	bms->senseLinesOpenIndices = malloc (sizeof (ssize_t) * bms->senseLineCount);

	// DiBacco: iterate over each segment in the module
	for (uint16_t segmentIndex = 0; segmentIndex < bms->segmentCount; ++segmentIndex)
	{
		// DiBacco: iterate over each LTC chip (batter monitor IC) 
		for (uint16_t ltcIndex = 0; ltcIndex < bms->ltcsPerSegment; ++ltcIndex)
		{
			// DiBacco: iterate over each sense line (voltage measurement line)
			for (uint16_t senseLineIndex = 0; senseLineIndex < bms->senseLinesPerLtc; ++senseLineIndex)
			{
				// DiBacco: get index of the sense line in reference to the entire bms
				uint16_t index = SENSE_LINE_INDEX_LOCAL_TO_GLOBAL (bms, segmentIndex, ltcIndex, senseLineIndex);
				
				// Get sense line temperature signal index
				char tempName [] = "SENSE_LINE_###_##_TEMPERATURE";
				uint16_t offset = printSenseLineIndex (bms, segmentIndex, ltcIndex, senseLineIndex, tempName + 11);
				snprintf (tempName + 11 + offset, 13, "_TEMPERATURE");
				checkSignalRedundancy (tempName, &signalNames, &signalCount); 

				bms->senseLineTemperatureIndices [index] = canDatabaseFindSignal (database, tempName);

				char openName [] = "SENSE_LINE_###_##_OPEN";
				offset = printSenseLineIndex (bms, segmentIndex, ltcIndex, senseLineIndex, openName + 11);
				snprintf (openName + 11 + offset, 6, "_OPEN");
				checkSignalRedundancy (openName, &signalNames, &signalCount); 

				bms->senseLinesOpenIndices [index] = canDatabaseFindSignal (database, openName);
				if (bms->senseLinesOpenIndices [index] < 0)
					return errno;
			}
		}
	}

	bms->ltcIsoSpiFaultIndices = malloc (sizeof (ssize_t) * bms->ltcsPerSegment * bms->segmentCount);
	bms->ltcSelfTestFaultIndices = malloc (sizeof (ssize_t) * bms->ltcsPerSegment * bms->segmentCount);
	bms->ltcTemperatureIndices = malloc (sizeof (ssize_t) * bms->ltcsPerSegment * bms->segmentCount);

	for (uint16_t segmentIndex = 0; segmentIndex < bms->segmentCount; ++segmentIndex)
	{
		for (uint16_t ltcIndex = 0; ltcIndex < bms->ltcsPerSegment; ++ltcIndex)
		{
			uint16_t index = ltcIndex + bms->ltcsPerSegment * segmentIndex;

			char isoSpiName [] = "BMS_LTC_###_ISOSPI_FAULT";
			uint16_t offset = printIndex (index, isoSpiName + 8);
			snprintf (isoSpiName + 8 + offset, 14, "_ISOSPI_FAULT");
			checkSignalRedundancy (isoSpiName, &signalNames, &signalCount); 

			bms->ltcIsoSpiFaultIndices [index] = canDatabaseFindSignal (database, isoSpiName);

			char selfTestName [] = "BMS_LTC_###_SELF_TEST_FAULT";
			offset = printIndex (index, selfTestName + 8);
			snprintf (selfTestName + 8 + offset, 17, "_SELF_TEST_FAULT");
			checkSignalRedundancy (selfTestName, &signalNames, &signalCount); 

			bms->ltcSelfTestFaultIndices [index] = canDatabaseFindSignal (database, selfTestName);

			char temperatureName [] = "BMS_LTC_###_TEMPERATURE";
			offset = printIndex (index, temperatureName + 8);
			snprintf (temperatureName + 8 + offset, 13, "_TEMPERATURE");
			checkSignalRedundancy (temperatureName, &signalNames, &signalCount);

			bms->ltcTemperatureIndices [index] = canDatabaseFindSignal (database, temperatureName);
		}
	}

	bms->packVoltageIndex = canDatabaseFindSignal (database, "PACK_VOLTAGE");
	checkSignalRedundancy ("PACK_VOLTAGE", &signalNames, &signalCount); 
	if (bms->packVoltageIndex < 0)
		return errno;

	bms->packCurrentIndex = canDatabaseFindSignal (database, "PACK_CURRENT");
	checkSignalRedundancy ("PACK_CURRENT", &signalNames, &signalCount); 
	if (bms->packCurrentIndex < 0)
		return errno;

	/*
	TODO(DiBacco): remove reduntant signals in the bms status panel 
		Begin with: 
			- Glory: ISO-SPI, Fault_Test

		Solution: use list to store names, which will be use to compare against the names of the messages in the panel
	*/

	// TODO(DiBacco): store canSignal_t* instead of indexes in the bmsStatusSignalIndices attribute of the bms 
	// otherwise canDatabaseGetMessage and canDatabaseGetSignal will be called in bms.c and bms_tui/main.c
	// (canSignal_t** bmsStatusSignals [BMS_STATUS_SIGNAL_COUNT_MAX])

	// Get bms status index and message
	bms->bmsStatusMessageIndex = canDatabaseFindMessage (database, "BMS_STATUS");
	canMessage_t* bmsStatusMessage = canDatabaseGetMessage (database, bms->bmsStatusMessageIndex);

	// Used to store the bms status signal indexes
	bms->bmsStatusSignalsCount = 0;
	bms->bmsStatusSignalIndices = malloc (sizeof (ssize_t) * bmsStatusMessage->signalCount);

	for (size_t signalIndex = 0; signalIndex < bmsStatusMessage->signalCount; signalIndex++) {
		// Get signal using the signal index
		ssize_t signalGlobalIndex = canDatabaseGetGlobalIndex (database, bmsStatusMessageIndex, signalIndex); 
		canSignal_t* bmsStatusSignal = canDatabaseGetSignal (database, signalGlobalIndex);

		// Check that the signal has not been retreived previously
		char* signalName = bmsStatusSignal->name;
		if (checkSignalRedundancy (signalName, &signalNames, &signalCount)) {
			// Append local index to the bms status signal indices
			bms->bmsStatusSignalIndices[bms->bmsStatusSignalsCount++] = signalIndex;  
		} 
	}

	return 0;
}

canDatabaseSignalState_t bmsGetPackVoltage (bms_t* bms, float* voltage)
{
	return canDatabaseGetFloat (bms->database, bms->packVoltageIndex, voltage);
}

canDatabaseSignalState_t bmsGetPackCurrent (bms_t* bms, float* current)
{
	return canDatabaseGetFloat (bms->database, bms->packCurrentIndex, current);
}

canDatabaseSignalState_t bmsGetCellVoltage (bms_t* bms, size_t index, float* voltage)
{
	return canDatabaseGetFloat (bms->database, bms->cellVoltageIndices [index], voltage);
}

canDatabaseSignalState_t bmsGetCellDischarging (bms_t* bms, size_t index, bool* discharging)
{
	return canDatabaseGetBool (bms->database, bms->cellsDischargingIndices [index], discharging);
}

canDatabaseSignalState_t bmsGetSenseLineTemperature (bms_t* bms, size_t index, float* temperature)
{
	return canDatabaseGetFloat (bms->database, bms->senseLineTemperatureIndices [index], temperature);
}

canDatabaseSignalState_t bmsGetSenseLineOpen (bms_t* bms, size_t index, bool* open)
{
	return canDatabaseGetBool (bms->database, bms->senseLinesOpenIndices [index], open);
}

bmsLtcState_t bmsGetLtcState (bms_t* bms, size_t index)
{
	bool isoSpiFault;
	bool selfTestFault;

	// TODO(Barach): How to manage this?

	// If either fault signal has timed out, return timeout
	if (canDatabaseGetBool (bms->database, bms->ltcIsoSpiFaultIndices [index], &isoSpiFault) != CAN_DATABASE_VALID)
		return BMS_LTC_STATE_TIMEOUT;
	if (canDatabaseGetBool (bms->database, bms->ltcSelfTestFaultIndices [index], &selfTestFault) != CAN_DATABASE_VALID)
		return BMS_LTC_STATE_TIMEOUT;

	// IsoSPI fault is higher importance than self-test fault.
	if (isoSpiFault)
		return BMS_LTC_STATE_ISOSPI_FAULT;
	if (selfTestFault)
		return BMS_LTC_STATE_SELF_TEST_FAULT;

	// No faults
	return BMS_LTC_STATE_OKAY;
}

canDatabaseSignalState_t bmsGetLtcTemperature (bms_t* bms, size_t index, float* temperature)
{
	return canDatabaseGetFloat (bms->database, bms->ltcTemperatureIndices [index], temperature);
}

bool bmsGetCellVoltageStats (bms_t* bms, float* min, float* max, float* avg)
{
	bool valid = false;
	size_t count = 0;

	if (min != NULL)
		*min = INFINITY;

	if (max != NULL)
		*max = -INFINITY;

	if (avg != NULL)
		*avg = 0;

	for (size_t index = 0; index < bms->cellsPerLtc * bms->ltcsPerSegment * bms->segmentCount; ++index)
	{
		float cell;
		if (canDatabaseGetFloat (bms->database, bms->cellVoltageIndices [index], &cell) != CAN_DATABASE_VALID)
			continue;

		valid = true;
		++count;

		if (min != NULL && cell < *min)
			*min = cell;

		if (max != NULL && cell > *max)
			*max = cell;

		if (avg != NULL)
			*avg += cell;
	}

	if (avg != NULL)
		*avg /= count;

	return valid;
}

bool bmsGetCellDeltaStats (bms_t* bms, float* max, float* avg)
{
	size_t count = 0;

	if (max != NULL)
		*max = -INFINITY;

	if (avg != NULL)
		*avg = 0;

	float min;
	bool valid = bmsGetCellVoltageStats (bms, &min, NULL, NULL);
	if (!valid)
		return false;
	valid = false;

	for (size_t index = 0; index < bms->cellsPerLtc * bms->ltcsPerSegment * bms->segmentCount; ++index)
	{
		float cell;
		if (canDatabaseGetFloat (bms->database, bms->cellVoltageIndices [index], &cell) != CAN_DATABASE_VALID)
			continue;

		float delta = cell - min;
		valid = true;
		++count;

		if (max != NULL && delta > *max)
			*max = delta;

		if (avg != NULL)
			*avg += delta;
	}

	if (avg != NULL)
		*avg /= count;

	return valid;
}

bool bmsGetTemperatureStats (bms_t* bms, float* min, float* max, float* avg)
{
	bool valid = false;
	size_t count = 0;

	if (min != NULL)
		*min = INFINITY;

	if (max != NULL)
		*max = -INFINITY;

	if (avg != NULL)
		*avg = 0;

	for (size_t index = 0; index < bms->senseLinesPerLtc * bms->ltcsPerSegment * bms->segmentCount; ++index)
	{
		float temp;
		if (canDatabaseGetFloat (bms->database, bms->senseLineTemperatureIndices [index], &temp) != CAN_DATABASE_VALID)
			continue;

		valid = true;
		++count;

		if (min != NULL && temp < *min)
			*min = temp;

		if (max != NULL && temp > *max)
			*max = temp;

		if (avg != NULL)
			*avg += temp;
	}

	if (avg != NULL)
		*avg /= count;

	return valid;
}

canDatabaseSignalState_t bmsGetSignalValue (canDatabase_t* database, size_t messageIndex, size_t signalIndex, float* value) {
	// Convert index within the message to index within the databaseS
	ssize_t globalIndex = canDatabaseGetGlobalIndex (database, messageIndex, signalIndex); 

	// Get the value of the message associated with the global index
	// Timeout in canDatabaseGetFloat is result of no data in the BUS
	return canDatabaseGetFloat (database, globalIndex, value);
}

bool checkSignalRedundancy (char* signalName, char*** signalNames, size_t* signalCount) {
	// return false if signal has previously been retreived
	for (size_t signalIndex = 0; signalIndex < *signalCount; signalIndex++) {
		if (strcmp (signalName, (*signalNames)[signalIndex]) == 0) {
			return false;
		}
	}

	// add signal name to the list
	*signalNames = realloc (*signalNames, (*signalCount + 1) * sizeof (char*)); 
	if (! (*signalName)) {
		printf ("Error: couldn't allocate memory for list\n");
	}

	(*signalNames)[(*signalCount)++] = strdup (signalName);

	return true;
}