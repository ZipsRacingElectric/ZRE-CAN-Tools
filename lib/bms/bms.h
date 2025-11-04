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

#define CELL_INDEX_LOCAL_TO_GLOBAL(bms, segmentIndex, ltcIndex, cellIndex)													\
	((bms)->cellsPerLtc * ((bms)->ltcsPerSegment * (segmentIndex) + ltcIndex) + (cellIndex))

#define SENSE_LINE_INDEX_LOCAL_TO_GLOBAL(bms, segmentIndex, ltcIndex, senseLineIndex)										\
	(((bms)->cellsPerLtc + 1) * ((bms)->ltcsPerSegment * (segmentIndex) + ltcIndex) + (senseLineIndex))

#define LTC_INDEX_LOCAL_TO_GLOBAL(bms, segmentIndex, ltcIndex)																\
	(((bms)->ltcsPerSegment) * (segmentIndex) + (ltcIndex))

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
	uint16_t statusSignalsCount;

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
	ssize_t* statusSignalIndices;
	ssize_t packVoltageIndex;
	ssize_t packCurrentIndex;

	// TODO(DiBacco): cannot compile .h file w. make
	// 	- tried using make clean & make -b
	//  - fixed: worked when I removed unessesary attributes
	// 		- not a solution but allowed it to work 
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

// TODO(DiBacco): review suggestions
// REVIEW(Barach): If switching to global signal indices, there should be bmsGetStatusName and bmsGetStatusUnit functions to convert from a 0-based index.

// REVIEW(Barach): Not a huge fan of how this function is set up, would prefer a bit more abstraction to it. To the end user,
//   this information is just a array of CAN signals, so it shouldn't require anything other than an index. Ideally the
//   signature is something along the lines of:
//
//   canDatabaseSignalState_t bmsGetStatusSignal (bms_t* bms, size_t index, float* value);
//
// Where index goes from 0 to (signalCount - 1).
//
/**
 * @brief Retreives the value of the signal associated with the signal index.
 * @param database the CAN database associated with the BMS.
 * @param messageIndex the index of the CAN message.
 * @param signalIndex the index of the signal within the CAN message.
 * @param value the value of the signal.
 */
canDatabaseSignalState_t bmsGetSignalValue (canDatabase_t* database, size_t messageIndex, size_t signalIndex, float* value);

void bmsDealloc (bms_t* bms);

static inline size_t bmsGetStatusSignalCount (bms_t* bms) {
	return bms->statusSignalsCount;
}

#endif // BMS_H