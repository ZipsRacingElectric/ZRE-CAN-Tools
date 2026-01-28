// Header
#include "bms.h"

// Includes
#include "array.h"
#include "cjson/cjson_util.h"
#include "debug.h"
#include "list.h"

// C Standard Library
#include <errno.h>
#include <stdlib.h>
#include <math.h>

listDefine (ssize_t);
arrayDefine (ssize_t);

/**
 * @brief Prints an index into an existing string.
 * @param index The index to print
 * @param name The string to print into.
 * @param n The maximum number of characters to write (including terminator).
 * @return The number of characters written, or, 0 on error.
 */
static size_t printIndex (size_t index, char* name, size_t n)
{
	int code = snprintf (name, n, "%lu", (long unsigned) index);
	if (code < 0 || (size_t) code >= n)
		return 0;

	return code;
}

size_t bmsSnprintSenseLineIndex (bms_t* bms, size_t index, char* str, size_t n)
{
	size_t textIndex = (index + 1) * bms->cellsPerLtc / bms->senseLinesPerLtc;

	// Print the text index and its suffix
	int code;
	if (index % bms->senseLinesPerLtc == 0)
	{
		// The first sense line in an LTC should have the HI suffix, as it belongs to the higher potential LTC.
		code = snprintf (str, n, "%lu_HI", (long unsigned) textIndex);
	}
	else if (index % bms->senseLinesPerLtc == bms->cellsPerLtc)
	{
		// The last sense line in an LTC should have the LO suffix, as it belongs to the lower potential LTC.
		code = snprintf (str, n, "%lu_LO", (long unsigned) textIndex);
	}
	else
	{
		// No suffix
		code = snprintf (str, n, "%lu", (long unsigned) textIndex);
	}
	if (code < 0 || (size_t) code >= n)
		return 0;

	return code;
}

int bmsInit (bms_t* bms, cJSON* config, canDatabase_t* database)
{
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
	bms->ltcCount = bms->segmentCount * bms->ltcsPerSegment;
	bms->cellCount = bms->ltcCount * bms->cellsPerLtc;
	bms->senseLineCount = bms->ltcCount * bms->senseLinesPerLtc;

	// Create a list to track the signals that have been used. This is so we can omit said signals from the array of status
	// signals.
	list_t (ssize_t) usedSignals;
	if (listInit (ssize_t) (&usedSignals, 512) != 0)
		return errno;

	// Create a list for the logical temperature indices.
	list_t (ssize_t) logicalTemperatureIndices;
	if (listInit (ssize_t) (&logicalTemperatureIndices, 64) != 0)
		return errno;

	bms->cellVoltageIndices = malloc (sizeof (ssize_t) * bms->cellCount);
	if (bms->cellVoltageIndices == NULL)
		return errno;

	bms->cellsDischargingIndices = malloc (sizeof (ssize_t) * bms->cellCount);
	if (bms->cellsDischargingIndices == NULL)
		return errno;

	// Traverse each cell in the BMS
	for (uint16_t index = 0; index < bms->cellCount; ++index)
	{
		// Get the cell voltage global index
		// - All cell voltage signals are required, fail if one is missing.

		char cellVoltageName [] = "CELL_VOLTAGE_###";
		size_t offset = printIndex (index, cellVoltageName + strlen ("CELL_VOLTAGE_"), strlen ("###") + 1);
		cellVoltageName [offset + strlen ("CELL_VOLTAGE_")] = '\0';

		bms->cellVoltageIndices [index] = canDatabaseFindSignal (database, cellVoltageName);
		if (bms->cellVoltageIndices [index] < 0)
			return errno;
		if (listAppend (ssize_t) (&usedSignals, bms->cellVoltageIndices [index]) != 0)
			return errno;

		// Get the cell discharging global index
		// - All cell discharging signals are required, fail if one is missing.

		char cellDischargingName [] = "CELL_BALANCING_###";
		offset = printIndex (index, cellDischargingName + strlen ("CELL_BALANCING_"), strlen ("###") + 1);
		cellDischargingName [offset + strlen ("CELL_BALANCING_")] = '\0';

		bms->cellsDischargingIndices [index] = canDatabaseFindSignal (database, cellDischargingName);
		if (bms->cellsDischargingIndices [index] < 0)
			return errno;
		if (listAppend (ssize_t) (&usedSignals, bms->cellsDischargingIndices [index]) != 0)
			return errno;
	}

	bms->senseLineTemperatureIndices = malloc (sizeof (ssize_t) * bms->senseLineCount);
	if (bms->senseLineTemperatureIndices == NULL)
		return errno;

	bms->senseLinesOpenIndices = malloc (sizeof (ssize_t) * bms->senseLineCount);
	if (bms->senseLinesOpenIndices == NULL)
		return errno;

	// Traverse each sense line in the BMS
	for (uint16_t index = 0; index < bms->senseLineCount; ++index)
	{
		// Get the sense line temperature global index
		// - Not all sense line temperature signals are required, -1 is used to indicate a signal is not present.

		char senseLineTemperatureName [] = "SENSE_LINE_###_##_TEMPERATURE";
		size_t offset = bmsSnprintSenseLineIndex (bms, index, senseLineTemperatureName + strlen ("SENSE_LINE_"), strlen ("###_##") + 1);
		snprintf (senseLineTemperatureName + strlen ("SENSE_LINE_") + offset, strlen ("_TEMPERATURE") + 1, "_TEMPERATURE");

		bms->senseLineTemperatureIndices [index] = canDatabaseFindSignal (database, senseLineTemperatureName);
		if (listAppend (ssize_t) (&usedSignals, bms->senseLineTemperatureIndices [index]) != 0)
			return errno;

		// If the temperature exists, add it to the logical indices.
		if (bms->senseLineTemperatureIndices [index] > 0)
		{
			if (listAppend (ssize_t) (&logicalTemperatureIndices, bms->senseLineTemperatureIndices [index]) != 0)
				return errno;
		}

		// Get the sense line open global index
		// - All sense line status signals are required, fail if one is missing.

		char senseLineOpenName [] = "SENSE_LINE_###_##_OPEN";
		offset = bmsSnprintSenseLineIndex (bms, index, senseLineOpenName + strlen ("SENSE_LINE_"), strlen ("###_##") + 1);
		snprintf (senseLineOpenName + strlen ("SENSE_LINE_") + offset, strlen ("_OPEN") + 1, "_OPEN");

		bms->senseLinesOpenIndices [index] = canDatabaseFindSignal (database, senseLineOpenName);
		if (bms->senseLinesOpenIndices [index] < 0)
			return errno;
		if (listAppend (ssize_t) (&usedSignals, bms->senseLinesOpenIndices [index]) != 0)
			return errno;
	}

	bms->ltcIsoSpiFaultIndices = malloc (sizeof (ssize_t) * bms->ltcsPerSegment * bms->segmentCount);
	if (bms->ltcIsoSpiFaultIndices == NULL)
		return errno;

	bms->ltcSelfTestFaultIndices = malloc (sizeof (ssize_t) * bms->ltcsPerSegment * bms->segmentCount);
	if (bms->ltcSelfTestFaultIndices == NULL)
		return errno;

	bms->ltcTemperatureIndices = malloc (sizeof (ssize_t) * bms->ltcsPerSegment * bms->segmentCount);
	if (bms->ltcTemperatureIndices == NULL)
		return errno;

	// Traverse each LTC in the BMS
	for (uint16_t index = 0; index < bms->ltcCount; ++index)
	{
		// Get the LTC's IsoSPI fault global index
		// - Not all IsoSPI fault signals are required, -1 is used to indicate a signal is not present.

		char ltcIsoSpiFaultName [] = "BMS_LTC_###_ISOSPI_FAULT";
		size_t offset = printIndex (index, ltcIsoSpiFaultName + strlen ("BMS_LTC_"), strlen ("###") + 1);
		snprintf (ltcIsoSpiFaultName + strlen ("BMS_LTC_") + offset, strlen ("_ISOSPI_FAULT") + 1, "_ISOSPI_FAULT");

		bms->ltcIsoSpiFaultIndices [index] = canDatabaseFindSignal (database, ltcIsoSpiFaultName);
		if (listAppend (ssize_t) (&usedSignals, bms->ltcIsoSpiFaultIndices [index]) != 0)
			return errno;

		// Get the LTC's self test fault global index
		// - Not all self test fault signals are required, -1 is used to indicate a signal is not present.

		char ltcSelfTestFaultName [] = "BMS_LTC_###_SELF_TEST_FAULT";
		offset = printIndex (index, ltcSelfTestFaultName + strlen ("BMS_LTC_"), strlen ("###") + 1);
		snprintf (ltcSelfTestFaultName + strlen ("BMS_LTC_") + offset, strlen ("_SELF_TEST_FAULT") + 1, "_SELF_TEST_FAULT");

		bms->ltcSelfTestFaultIndices [index] = canDatabaseFindSignal (database, ltcSelfTestFaultName);
		if (listAppend (ssize_t) (&usedSignals, bms->ltcSelfTestFaultIndices [index]) != 0)
			return errno;

		// Get the LTC temperature global index
		// - Not all temperature signals are required, -1 is used to indicate a signal is not present.

		char ltcTemperatureName [] = "BMS_LTC_###_TEMPERATURE";
		offset = printIndex (index, ltcTemperatureName + strlen ("BMS_LTC_"), strlen ("###") + 1);
		snprintf (ltcTemperatureName + strlen ("BMS_LTC_") + offset, strlen ("_TEMPERATURE") + 1, "_TEMPERATURE");

		bms->ltcTemperatureIndices [index] = canDatabaseFindSignal (database, ltcTemperatureName);
		if (listAppend (ssize_t) (&usedSignals, bms->ltcTemperatureIndices [index]) != 0)
			return errno;
	}

	// Get the pack voltage global index

	bms->packVoltageIndex = canDatabaseFindSignal (database, "PACK_VOLTAGE");
	if (bms->packVoltageIndex < 0)
		return errno;

	if (listAppend (ssize_t) (&usedSignals, bms->packVoltageIndex) != 0)
		return errno;

	// Get the pack current global index & append it to the indicies
	bms->packCurrentIndex = canDatabaseFindSignal (database, "PACK_CURRENT");
	if (bms->packCurrentIndex < 0)
		return errno;

	if (listAppend (ssize_t) (&usedSignals, bms->packCurrentIndex) != 0)
		return errno;

	// Get the status message and its signals
	// - Note, all signals in usedSignals are omitted, as they'd be redundant.

	ssize_t statusMessageIndex = canDatabaseFindMessage (database, "BMS_STATUS");
	if (statusMessageIndex < 0)
		return errno;

	canMessage_t* statusMessage = canDatabaseGetMessage (database, statusMessageIndex);
	bms->statusSignalsCount = 0;
	bms->statusSignalIndices = malloc (sizeof (ssize_t) * statusMessage->signalCount);
	if (bms->statusSignalIndices == NULL)
		return errno;

	for (size_t signalIndex = 0; signalIndex < statusMessage->signalCount; ++signalIndex)
	{
		// Get the global index of the signal. Skip if we have already used the signal.
		ssize_t globalIndex = canDatabaseGetGlobalIndex (database, statusMessageIndex, signalIndex);
		if (arrayContains (ssize_t) (listArray (ssize_t) (&usedSignals), globalIndex, listSize (ssize_t) (&usedSignals)))
		{
			canSignal_t* signal = canDatabaseGetSignal (database, globalIndex);
			if (signal != NULL)
				debugPrintf ("Ignoring redundant BMS status signal '%s'\n", signal->name);

			continue;
		}

		// If the signal is unique, add it to the status signals.
		bms->statusSignalIndices [bms->statusSignalsCount] = globalIndex;
		++bms->statusSignalsCount;
	}

	// Deallocate the usedSignals list, as we are done with it.
	listDealloc (ssize_t) (&usedSignals);

	// Convert the logical temperature indices list into an array.
	bms->logicalTemperatureIndices = listDestroy (ssize_t) (&logicalTemperatureIndices, &bms->logicalTemperatureCount);
	if (bms->logicalTemperatureIndices == NULL)
		return errno;

	// Load the BMS's fault signals
	cJSON* faults;
	if (jsonGetObject (config, "faults", &faults) == 0)
	{
		bms->faults = faultSignalsLoad (faults, &bms->faultCount, database);
		if (bms->faults == NULL)
			return errno;
	}
	else
	{
		debugPrintf ("Warning: BMS config file is missing 'faults' array.\n");
		bms->faults = NULL;
		bms->faultCount = 0;
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

canDatabaseSignalState_t bmsGetPackPower (bms_t* bms, float* power)
{
	float voltage;
	canDatabaseSignalState_t voltageState = bmsGetPackVoltage (bms, &voltage);
	float current;
	canDatabaseSignalState_t currentState = bmsGetPackCurrent (bms, &current);

	if (voltageState == CAN_DATABASE_MISSING || currentState == CAN_DATABASE_MISSING)
		return CAN_DATABASE_MISSING;

	if (voltageState == CAN_DATABASE_TIMEOUT || currentState == CAN_DATABASE_TIMEOUT)
		return CAN_DATABASE_TIMEOUT;

	*power = voltage * current;
	return CAN_DATABASE_VALID;
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

canDatabaseSignalState_t bmsGetLogicalTemperature (bms_t* bms, size_t index, float* temperature)
{
	return canDatabaseGetFloat (bms->database, bms->logicalTemperatureIndices [index], temperature);
}

bmsLtcState_t bmsGetLtcState (bms_t* bms, size_t index)
{
	// IsoSPI fault is higher importance than self-test fault, test that first.
	bool isoSpiFaultMissing = false;
	bool isoSpiFault;
	switch (canDatabaseGetBool (bms->database, bms->ltcIsoSpiFaultIndices [index], &isoSpiFault))
	{
	// If there is no signal, we can't assume anything.
	case CAN_DATABASE_MISSING:
		isoSpiFault = false;
		isoSpiFaultMissing = true;
		break;

	// If either signal is timed out, the LTC is considered timed out.
	case CAN_DATABASE_TIMEOUT:
		return BMS_LTC_STATE_TIMEOUT;

	// If an IsoSPI fault is present, return that state.
	case CAN_DATABASE_VALID:
		if (isoSpiFault)
			return BMS_LTC_STATE_ISOSPI_FAULT;
	}

	bool selfTestFaultMissing = false;
	bool selfTestFault;
	switch (canDatabaseGetBool (bms->database, bms->ltcSelfTestFaultIndices [index], &selfTestFault))
	{
	// If there is no signal, we can't assume anything.
	case CAN_DATABASE_MISSING:
		selfTestFault = false;
		selfTestFaultMissing = true;
		break;

	// If either signal is timed out, the LTC is considered timed out
	case CAN_DATABASE_TIMEOUT:
		return BMS_LTC_STATE_TIMEOUT;

	// If a self test fault is present, return that state.
	case CAN_DATABASE_VALID:
		if (selfTestFault)
			return BMS_LTC_STATE_SELF_TEST_FAULT;
	}

	// If we have no information about either signal, the state is considered missing.
	if (selfTestFaultMissing && isoSpiFaultMissing)
		return BMS_LTC_STATE_MISSING;

	// Otherwise, no faults
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
	free (bms->faults);
	free (bms->statusSignalIndices);
	free (bms->ltcTemperatureIndices);
	free (bms->ltcSelfTestFaultIndices);
	free (bms->ltcIsoSpiFaultIndices);
	free (bms->senseLinesOpenIndices);
	free (bms->senseLineTemperatureIndices);
	free (bms->cellsDischargingIndices);
	free (bms->cellVoltageIndices);
}