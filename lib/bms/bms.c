// Header
#include "bms.h"

// Includes
#include "cjson/cjson_util.h"

// C Standard Library
#include <errno.h>
#include <stdlib.h>
#include <math.h>

/**
 * @brief Prints an index into an existing string.
 * @param index The index to print
 * @param name The string to print into.
 * @return The number of characters written.
 */
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
	// Create list to contain signal names to storing redundant information.
	// REVIEW(Barach): Because these are local variables and dynamically allocated, they must be deallocated by the end of
	//   this function (no other way prevent a memory leak). While there is quite a bit of memory in here that gets allocated
	//   it is all stored in the bms object so that it can be deallocated correctly later.
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

	for (uint16_t segmentIndex = 0; segmentIndex < bms->segmentCount; ++segmentIndex)
	{
		for (uint16_t ltcIndex = 0; ltcIndex < bms->ltcsPerSegment; ++ltcIndex)
		{
			for (uint16_t senseLineIndex = 0; senseLineIndex < bms->senseLinesPerLtc; ++senseLineIndex)
			{
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

	// Get bms status index and message
	bms->bmsStatusMessageIndex = canDatabaseFindMessage (database, "BMS_STATUS");
	canMessage_t* bmsStatusMessage = canDatabaseGetMessage (database, bms->bmsStatusMessageIndex);

	// Used to store the bms status signals and bms status indices
	bms->bmsStatusSignalsCount = 0;
	bms->bmsStatusSignals = malloc (sizeof (canSignal_t*) * bmsStatusMessage->signalCount);
	bms->bmsStatusSignalIndices = malloc (sizeof (size_t) * bmsStatusMessage->signalCount);

	for (size_t signalIndex = 0; signalIndex < bmsStatusMessage->signalCount; signalIndex++) {
		// Get signal using the signal index
		ssize_t signalGlobalIndex = canDatabaseGetGlobalIndex (database, bms->bmsStatusMessageIndex, signalIndex);
		canSignal_t* bmsStatusSignal = canDatabaseGetSignal (database, signalGlobalIndex);

		// Check that the signal has not been retreived previously
		char* signalName = bmsStatusSignal->name;
		if (checkSignalRedundancy (signalName, &signalNames, &signalCount)) {
			// Append signal to the bms status signals and index to the bms status signals indices
			bms->bmsStatusSignals[bms->bmsStatusSignalsCount] = bmsStatusSignal;
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
	return canDatabaseGetFloat (database, globalIndex, value);
}

void bmsDealloc (bms_t* bms)
{
	// TODO(Barach): Init doesn't deallocate correctly on failure.
	// Deallocate all dynamically allocated memory.
	free (bms->bmsStatusSignals);
	free (bms->bmsStatusSignalIndices);
	free (bms->ltcTemperatureIndices);
	free (bms->ltcSelfTestFaultIndices);
	free (bms->ltcIsoSpiFaultIndices);
	free (bms->senseLinesOpenIndices);
	free (bms->senseLineTemperatureIndices);
	free (bms->cellsDischargingIndices);
	free (bms->cellVoltageIndices);
}

// REVIEW(Barach): Because you are storing strings, this function is a bit complex. You can just store the global index of
//   the signal instead (signal names don't change, so the two are interchangable).
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
		// REVIEW(Barach): Library functions should not print error messages, rather, they should return the error code that
		// triggered the message.
		printf ("Error: couldn't allocate memory for list \n");
	}

	// REVIEW(Barach): If the memory allocation above failed, accessing the array will cause undefined behavior, likely
	// crashing the program. This function needs a way to gracefully fail. Ex. return int error code and write result to a
	// bool* parameter.

	// REVIEW(Barach): It is not obvious what this is doing. While it consolidates code, doing multiple operations in a single
	// line like this really isn't good practice. Preferrable to do:
	//   *signalNames [*signalCount] = ...
	//   ++(*signalCount)

	(*signalNames)[(*signalCount)++] = strdup (signalName);

	return true;
}