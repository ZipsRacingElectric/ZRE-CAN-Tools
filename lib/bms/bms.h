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
#include "can_database/can_database.h"
#include "cjson/cjson.h"

// Macros ---------------------------------------------------------------------------------------------------------------------

#define LTC_TO_CELL_INDEX(bms, segmentIndex, ltcIndex, cellIndex)															\
	((bms)->cellsPerLtc * ((bms)->ltcsPerSegment * (segmentIndex) + ltcIndex) + (cellIndex))

#define LTC_TO_SENSE_LINE_INDEX(bms, segmentIndex, ltcIndex, senseLineIndex)												\
	(((bms)->cellsPerLtc + 1) * ((bms)->ltcsPerSegment * (segmentIndex) + ltcIndex) + (senseLineIndex))

// Datatypes ------------------------------------------------------------------------------------------------------------------

typedef enum
{
	BMS_LTC_STATE_MISSING = CAN_DATABASE_MISSING,
	BMS_LTC_STATE_TIMEOUT = CAN_DATABASE_TIMEOUT,
	BMS_LTC_STATE_ISOSPI_FAULT,
	BMS_LTC_STATE_SELF_TEST_FAULT,
	BMS_LTC_STATE_OKAY
} bmsLtcState_t;

typedef struct
{
	canDatabase_t* database;

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
	float maxLtcTemperature;

	ssize_t* cellVoltageIndices;
	ssize_t* cellsDischargingIndices;
	ssize_t* senseLineTemperatureIndices;
	ssize_t* senseLinesOpenIndices;
	ssize_t* ltcIsoSpiFaultIndices;
	ssize_t* ltcSelfTestFaultIndices;
	ssize_t* ltcTemperatureIndices;
	ssize_t packVoltageIndex;
	ssize_t packCurrentIndex;
} bms_t;

// Functions ------------------------------------------------------------------------------------------------------------------

int bmsInit (bms_t* bms, cJSON* config, canDatabase_t* database);

canDatabaseSignalState_t bmsGetPackVoltage (bms_t* bms, float* voltage);

canDatabaseSignalState_t bmsGetPackCurrent (bms_t* bms, float* current);

canDatabaseSignalState_t bmsGetCellVoltage (bms_t* bms, size_t index, float* voltage);

canDatabaseSignalState_t bmsGetCellDischarging (bms_t* bms, size_t index, bool* discharging);

canDatabaseSignalState_t bmsGetSenseLineTemperature (bms_t* bms, size_t index, float* temperature);

canDatabaseSignalState_t bmsGetSenseLineOpen (bms_t* bms, size_t index, bool* open);

bmsLtcState_t bmsGetLtcState (bms_t* bms, size_t index);

canDatabaseSignalState_t bmsGetLtcTemperature (bms_t* bms, size_t index, float* temperature);

bool bmsGetCellVoltageStats (bms_t* bms, float* min, float* max, float* avg);

bool bmsGetCellDeltaStats (bms_t* bms, float* max, float* avg);

bool bmsGetTemperatureStats (bms_t* bms, float* min, float* max, float* avg);

#endif // BMS_H