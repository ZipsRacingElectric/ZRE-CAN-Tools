#ifndef BMS_H
#define BMS_H

// Battery Management System CAN Interface ------------------------------------------------------------------------------------
//
// Author: Cole Barach
// Date Created: 2025.05.24
//
// Description: CAN interface for a battery management system.

// Includes -------------------------------------------------------------------------------------------------------------------

// Includes
#include "can/can_database.h"
#include "cjson/cjson.h"

// Macros ---------------------------------------------------------------------------------------------------------------------

#define LTC_TO_CELL_INDEX(bms, segmentIndex, ltcIndex, cellIndex)															\
	((bms)->cellsPerLtc * ((bms)->ltcsPerSegment * (segmentIndex) + ltcIndex) + (cellIndex))

#define LTC_TO_SENSE_LINE_INDEX(bms, segmentIndex, ltcIndex, senseLineIndex)												\
	(((bms)->cellsPerLtc + 1) * ((bms)->ltcsPerSegment * (segmentIndex) + ltcIndex) + (senseLineIndex))

// Datatypes ------------------------------------------------------------------------------------------------------------------

typedef struct
{
	uint16_t segmentCount;

	uint16_t cellCount;
	uint16_t senseLineCount;
	uint16_t ltcsPerSegment;
	uint16_t cellsPerLtc;
	uint16_t senseLinesPerLtc;

	float minCellVoltage;
	float maxCellVoltage;
	float minTemperature;
	float maxTemperature;

	float** cellVoltages;
	bool** cellVoltagesValid;

	float** cellsDischarging;
	bool** cellsDischargingValid;

	float** senseLineTemperatures;
	bool** senseLineTemperaturesValid;

	float** senseLinesOpen;
	bool** senseLinesOpenValid;

	float** ltcIsoSpiFaults;
	bool** ltcIsoSpiFaultsValid;

	float** ltcIsoSpiSelfTestFaults;
	bool** ltcIsoSpiSelfTestFaultsValid;

	float* packVoltage;
	bool* packVoltageValid;

	float* packCurrent;
	bool* packCurrentValid;
} bms_t;

// Functions ------------------------------------------------------------------------------------------------------------------

int bmsInit (bms_t* bms, cJSON* config, canDatabase_t* database);

#endif // BMS_H