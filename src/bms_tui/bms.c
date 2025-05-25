// Header
#include "bms.h"

// Includes
#include "cjson/cjson_util.h"

// C Standard Library
#include <errno.h>

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

	bms->senseLinesPerLtc = bms->cellsPerLtc + 1;
	bms->cellCount = bms->segmentCount * bms->ltcsPerSegment * bms->cellsPerLtc;
	bms->senseLineCount = bms->segmentCount * bms->ltcsPerSegment * bms->senseLinesPerLtc;

	// Get database references

	bms->cellVoltages = malloc (sizeof (float*) * bms->cellCount);
	bms->cellVoltagesValid = malloc (sizeof (bool*) * bms->cellCount);

	bms->cellsDischarging = malloc (sizeof (float*) * bms->cellCount);
	bms->cellsDischargingValid = malloc (sizeof (bool*) * bms->cellCount);

	for (uint16_t index = 0; index < bms->cellCount; ++index)
	{
		char voltName [] = "CELL_VOLTAGE_###";
		size_t offset = printIndex (index, voltName + 13);
		voltName [offset + 13] = '\0';

		size_t signalIndex;
		if (canDatabaseFindSignal (database, voltName, &signalIndex) != 0)
			return errno;

		bms->cellVoltages [index]		= database->signalValues + signalIndex;
		bms->cellVoltagesValid [index]	= database->signalsValid + signalIndex;

		char disName [] = "CELL_BALANCING_###";
		offset = printIndex (index, disName + 15);
		disName [offset + 15] = '\0';

		if (canDatabaseFindSignal (database, disName, &signalIndex) != 0)
			return errno;

		bms->cellsDischarging [index]		= database->signalValues + signalIndex;
		bms->cellsDischargingValid [index]	= database->signalsValid + signalIndex;
	}

	bms->senseLineTemperatures = malloc (sizeof (float*) * bms->senseLineCount);
	bms->senseLineTemperaturesValid = malloc (sizeof (bool*) * bms->senseLineCount);
	
	bms->senseLinesOpen = malloc (sizeof (float*) * bms->senseLineCount);
	bms->senseLinesOpenValid = malloc (sizeof (bool*) * bms->senseLineCount);

	for (uint16_t segmentIndex = 0; segmentIndex < bms->segmentCount; ++segmentIndex)
	{
		for (uint16_t ltcIndex = 0; ltcIndex < bms->ltcsPerSegment; ++ltcIndex)
		{
			for (uint16_t senseLineIndex = 0; senseLineIndex < bms->senseLinesPerLtc; ++senseLineIndex)
			{
				uint16_t index = LTC_TO_SENSE_LINE_INDEX (bms, segmentIndex, ltcIndex, senseLineIndex);

				char tempName [] = "SENSE_LINE_###_##_TEMPERATURE";
				uint16_t offset = printSenseLineIndex (bms, segmentIndex, ltcIndex, senseLineIndex, tempName + 11);
				snprintf (tempName + 11 + offset, 13, "_TEMPERATURE");

				size_t signalIndex;
				if (canDatabaseFindSignal (database, tempName, &signalIndex) == 0)
				{
					bms->senseLineTemperatures [index]		= database->signalValues + signalIndex;
					bms->senseLineTemperaturesValid [index] = database->signalsValid + signalIndex;
				}
				else
				{
					bms->senseLineTemperatures [index]		= NULL;
					bms->senseLineTemperaturesValid [index]	= NULL;
				}

				char openName [] = "SENSE_LINE_###_##_OPEN";
				offset = printSenseLineIndex (bms, segmentIndex, ltcIndex, senseLineIndex, openName + 11);
				snprintf (openName + 11 + offset, 6, "_OPEN");

				if (canDatabaseFindSignal (database, openName, &signalIndex) != 0)
					return errno;

				bms->senseLinesOpen [index]			= database->signalValues + signalIndex;
				bms->senseLinesOpenValid [index]	= database->signalsValid + signalIndex;
			}
		}
	}

	bms->ltcIsoSpiFaults = malloc (sizeof (float*) * bms->ltcsPerSegment * bms->segmentCount);
	bms->ltcIsoSpiFaultsValid = malloc (sizeof (bool*) * bms->ltcsPerSegment * bms->segmentCount);

	bms->ltcIsoSpiSelfTestFaults = malloc (sizeof (float*) * bms->ltcsPerSegment * bms->segmentCount);
	bms->ltcIsoSpiSelfTestFaultsValid = malloc (sizeof (bool*) * bms->ltcsPerSegment * bms->segmentCount);

	for (uint16_t segmentIndex = 0; segmentIndex < bms->segmentCount; ++segmentIndex)
	{
		for (uint16_t ltcIndex = 0; ltcIndex < bms->ltcsPerSegment; ++ltcIndex)
		{
			uint16_t index = ltcIndex + bms->ltcsPerSegment * segmentIndex;

			char isoSpiName [] = "BMS_LTC_###_ISOSPI_FAULT";
			uint16_t offset = printIndex (index, isoSpiName + 8);
			snprintf (isoSpiName + 8 + offset, 14, "_ISOSPI_FAULT");

			size_t signalIndex;
			if (canDatabaseFindSignal (database, isoSpiName, &signalIndex) != 0)
				return errno;

			bms->ltcIsoSpiFaults [index]		= database->signalValues + signalIndex;
			bms->ltcIsoSpiFaultsValid [index]	= database->signalsValid + signalIndex;

			char selfTestName [] = "BMS_LTC_###_SELF_TEST_FAULT";
			offset = printIndex (index, isoSpiName + 8);
			snprintf (isoSpiName + 8 + offset, 17, "_SELF_TEST_FAULT");

			if (canDatabaseFindSignal (database, isoSpiName, &signalIndex) != 0)
				return errno;

			bms->ltcIsoSpiSelfTestFaults [index]		= database->signalValues + signalIndex;
			bms->ltcIsoSpiSelfTestFaultsValid [index]	= database->signalsValid + signalIndex;
		}
	}

	size_t signalIndex;
	if (canDatabaseFindSignal (database, "PACK_VOLTAGE", &signalIndex) != 0)
		return errno;

	bms->packVoltage = database->signalValues + signalIndex;
	bms->packVoltageValid = database->signalsValid + signalIndex;

	if (canDatabaseFindSignal (database, "PACK_CURRENT", &signalIndex) != 0)
		return errno;

	bms->packCurrent = database->signalValues + signalIndex;
	bms->packCurrentValid = database->signalsValid + signalIndex;

	return 0;
}