// Header
#include "bms.h"

// Includes
#include "cjson/cjson_util.h"

// C Standard Library
#include <errno.h>
#include <stdlib.h>
#include <math.h>

// TODO(Barach): Replace with list lib?

/**
 * @brief Checks if signal has already been retreived to minimize signal redundancy. Dyanmically adds the signal's index to a list of signal indices if not.
 * @param index The global index of the signal
 * @param indices A list used to store the indices of previously retreived signals
 * @param signalCount The number of signal incices in the signal indices list
 * @param result The result of the function (true indicates that the signal has not been previously retreived / false indicates that the signal has been previously retreived)
 */
static int checkSignalRedundancy (ssize_t index, size_t** indices, size_t* signalCount, bool* result) {
	// return false if signal has previously been retreived
	for (size_t i = 0; i < *signalCount; i++)
	{
		// TODO(Barach): Warn
		if ((*indices)[i] == index)
		{
			*result = false;
			return 0;
		}
	}

	// add signal name to the list
	*indices = realloc (*indices, (*signalCount + 1) * sizeof (size_t));
	if (! (*indices)) {
		// Set errno to indicate that the program failed to allocate the requested memory
		errno = ENOMEM;
		return -1;
	}

	(*indices)[*signalCount] = index;
	++(*signalCount);
	*result = true;
	return 0;
}

// TODO(Barach): Can replace with a snprintf
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

// TODO(Barach): Clean this up majorly
static size_t printSenseLineIndex (bms_t* bms, uint16_t segmentIndex, uint16_t ltcIndex, uint16_t senseLineIndex, char* name)
{
	// The index to write within the text, note this uses the number of cells per LTC, not the number of sense lines per LTC.
	uint16_t renderIndex = bms->cellsPerLtc * (segmentIndex * bms->ltcsPerSegment + ltcIndex) + senseLineIndex;

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
	bool result;
	size_t signalCount = 0;
	size_t* signalIndices = NULL;

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
		// Format the cell voltage signal name
		char voltName [] = "CELL_VOLTAGE_###";
		size_t offset = printIndex (index, voltName + 13);
		voltName [offset + 13] = '\0';

		// Get the cell voltage global index & append it to the indicies + the signal redundancy list
		ssize_t cellVoltageIndex = canDatabaseFindSignal (database, voltName);
		bms->cellVoltageIndices [index] = cellVoltageIndex;
		checkSignalRedundancy (cellVoltageIndex, &signalIndices, &signalCount, &result);
		if (bms->cellVoltageIndices [index] < 0)
			return errno;

		// Format the cell balancing signal name
		char disName [] = "CELL_BALANCING_###";
		offset = printIndex (index, disName + 15);
		disName [offset + 15] = '\0';

		// Get the cell balancing global index & append it to the indicies + the signal redundancy list
		ssize_t cellsDischargingIndex = canDatabaseFindSignal (database, disName);
		bms->cellsDischargingIndices [index] = cellsDischargingIndex;
		checkSignalRedundancy (cellsDischargingIndex, &signalIndices, &signalCount, &result);
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

				// Format the sense line temperature signal name
				char tempName [] = "SENSE_LINE_###_##_TEMPERATURE";
				uint16_t offset = printSenseLineIndex (bms, segmentIndex, ltcIndex, senseLineIndex, tempName + 11);
				snprintf (tempName + 11 + offset, 13, "_TEMPERATURE");

				// Get the sense line temperature global index & append it to the indicies + the signal redundancy list
				ssize_t senseLineTemperatureIndex = canDatabaseFindSignal (database, tempName);
				bms->senseLineTemperatureIndices [index] = senseLineTemperatureIndex;
				checkSignalRedundancy (senseLineTemperatureIndex, &signalIndices, &signalCount, &result);

				// Format the sense line open signal name
				char openName [] = "SENSE_LINE_###_##_OPEN";
				offset = printSenseLineIndex (bms, segmentIndex, ltcIndex, senseLineIndex, openName + 11);
				snprintf (openName + 11 + offset, 6, "_OPEN");

				// Get the sense line open global index & append it to the indicies + the signal redundancy list
				ssize_t senseLinesOpenIndex = canDatabaseFindSignal (database, openName);
				bms->senseLinesOpenIndices [index] = senseLinesOpenIndex;
				checkSignalRedundancy (senseLinesOpenIndex, &signalIndices, &signalCount, &result);
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

			// Format the LTC ISOSPI fault signal name
			char isoSpiName [] = "BMS_LTC_###_ISOSPI_FAULT";
			uint16_t offset = printIndex (index, isoSpiName + 8);
			snprintf (isoSpiName + 8 + offset, 14, "_ISOSPI_FAULT");

			// Get the LTC ISOSPI fault global index & append it to the indicies + the signal redundancy list
			ssize_t ltcIsoSpiFaultIndices = canDatabaseFindSignal (database, isoSpiName);
			bms->ltcIsoSpiFaultIndices [index] = ltcIsoSpiFaultIndices;
			checkSignalRedundancy (ltcIsoSpiFaultIndices, &signalIndices, &signalCount, &result);

			// Format the LTC ISOSPI test fault signal name
			char selfTestName [] = "BMS_LTC_###_SELF_TEST_FAULT";
			offset = printIndex (index, selfTestName + 8);
			snprintf (selfTestName + 8 + offset, 17, "_SELF_TEST_FAULT");

			// Get the LTC ISOSPI test fault global index & append it to the indicies + the signal redundancy list
			ssize_t ltcSelfTestFaultIndex = canDatabaseFindSignal (database, selfTestName);
			bms->ltcSelfTestFaultIndices [index] = ltcSelfTestFaultIndex;
			checkSignalRedundancy (ltcSelfTestFaultIndex, &signalIndices, &signalCount, &result);

			// Format the LTC temperature signal name
			char temperatureName [] = "BMS_LTC_###_TEMPERATURE";
			offset = printIndex (index, temperatureName + 8);
			snprintf (temperatureName + 8 + offset, 13, "_TEMPERATURE");

			// Get the LTC temperature global index & append it to the indicies + the signal redundancy list
			ssize_t ltcTemperatureIndex = canDatabaseFindSignal (database, temperatureName);
			bms->ltcTemperatureIndices [index] = ltcTemperatureIndex;
			checkSignalRedundancy (ltcTemperatureIndex, &signalIndices, &signalCount, &result);
		}
	}

	// Get the pack voltage global index & append it to the indicies + the signal redundancy list
	ssize_t packVoltageIndex = canDatabaseFindSignal (database, "PACK_VOLTAGE");
	bms->packVoltageIndex = packVoltageIndex;
	checkSignalRedundancy (packVoltageIndex, &signalIndices, &signalCount, &result);
	if (bms->packVoltageIndex < 0)
		return errno;

	// Get the pack current global index & append it to the indicies + the signal redundancy list
	size_t packCurrentIndex = canDatabaseFindSignal (database, "PACK_CURRENT");
	bms->packCurrentIndex = packCurrentIndex;
	checkSignalRedundancy (packCurrentIndex, &signalIndices, &signalCount, &result);

	if (bms->packCurrentIndex < 0)
		return errno;

	// Get bms status index and message
	ssize_t bmsStatusMessageIndex = canDatabaseFindMessage (database, "BMS_STATUS");
	canMessage_t* bmsStatusMessage = canDatabaseGetMessage (database, bmsStatusMessageIndex);

	// Used to store the bms status signals and bms status indices
	bms->statusSignalsCount = 0;
	bms->statusSignalIndices = malloc (sizeof (ssize_t) * bmsStatusMessage->signalCount);

	for (size_t signalIndex = 0; signalIndex < bmsStatusMessage->signalCount; signalIndex++) {
		// Get bms status signal & its corresponding global index
		ssize_t bmsStatusSignalGlobalIndex = canDatabaseGetGlobalIndex (database, bmsStatusMessageIndex, signalIndex);

		// Check that the signal has not been retreived previously
		if (checkSignalRedundancy (bmsStatusSignalGlobalIndex, &signalIndices, &signalCount, &result))
			// Check if memory allocation failed
			return -1;

		if (result)
		{
			// Append signal to the bms status signals and index to the bms status signals indices
			bms->statusSignalIndices[bms->statusSignalsCount++] = bmsStatusSignalGlobalIndex;
		}
	}

	// Deallocate the memory allocated to the list storing the previously retreived signals
	free (signalIndices);

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

void bmsDealloc (bms_t* bms)
{
	// TODO(Barach): Init doesn't deallocate correctly on failure.
	// Deallocate all dynamically allocated memory.
	free (bms->statusSignalIndices);
	free (bms->ltcTemperatureIndices);
	free (bms->ltcSelfTestFaultIndices);
	free (bms->ltcIsoSpiFaultIndices);
	free (bms->senseLinesOpenIndices);
	free (bms->senseLineTemperatureIndices);
	free (bms->cellsDischargingIndices);
	free (bms->cellVoltageIndices);
}