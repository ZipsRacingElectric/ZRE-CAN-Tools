#ifndef BMS_H
#define BMS_H

// Battery Management System CAN Interface ------------------------------------------------------------------------------------
//
// Author: Cole Barach
// Date Created: 2025.05.24
//
// Description: CAN interface for a battery management system.

// TODO(Barach):
// - Should have a regular config in addition to JSON.

// Includes -------------------------------------------------------------------------------------------------------------------

// Includes
#include "can_database/can_database.h"
#include "cjson/cjson.h"

// Macros ---------------------------------------------------------------------------------------------------------------------

/// @brief Converts a local (per LTC) cell index to a global cell index.
#define CELL_INDEX_LOCAL_TO_GLOBAL(bms, segmentIndex, ltcIndex, cellIndex)													\
	((bms)->cellsPerLtc * ((bms)->ltcsPerSegment * (segmentIndex) + ltcIndex) + (cellIndex))

/// @brief Converts a local (per LTC) sense line index to a global index.
#define SENSE_LINE_INDEX_LOCAL_TO_GLOBAL(bms, segmentIndex, ltcIndex, senseLineIndex)										\
	(((bms)->cellsPerLtc + 1) * ((bms)->ltcsPerSegment * (segmentIndex) + ltcIndex) + (senseLineIndex))

/// @brief Converts a local (per segment) LTC index to a global index.
#define LTC_INDEX_LOCAL_TO_GLOBAL(bms, segmentIndex, ltcIndex)																\
	(((bms)->ltcsPerSegment) * (segmentIndex) + (ltcIndex))

// Datatypes ------------------------------------------------------------------------------------------------------------------

/// @brief State of a BMS's LTC.
typedef enum
{
	/// @brief Indicates no information about the LTC is available.
	BMS_LTC_STATE_MISSING = CAN_DATABASE_MISSING,

	/// @brief Indicates one or more signals belonging to the LTC have timed out.
	BMS_LTC_STATE_TIMEOUT = CAN_DATABASE_TIMEOUT,

	/// @brief Indicates the LTC has an IsoSPI fault.
	BMS_LTC_STATE_ISOSPI_FAULT,

	/// @brief Indicates the LTC has a self test fault.
	BMS_LTC_STATE_SELF_TEST_FAULT,

	/// @brief Indicates no issues.
	BMS_LTC_STATE_OKAY
} bmsLtcState_t;

/// @brief Object representing a battery management system.
typedef struct
{
	/// @brief The CAN database the BMS is bound to.
	canDatabase_t* database;

	/// @brief The number of segments in the BMS.
	uint16_t segmentCount;
	/// @brief The total number of battery cells in the BMS.
	uint16_t cellCount;
	/// @brief The total number of sense lines in the BMS.
	uint16_t senseLineCount;
	/// @brief The total number of LTCs in the BMS.
	size_t ltcCount;

	/// @brief The number of LTCs per segment.
	uint16_t ltcsPerSegment;
	/// @brief The number of battery cells per LTC.
	uint16_t cellsPerLtc;
	/// @brief The number of sense lines per LTC.
	uint16_t senseLinesPerLtc;

	/// @brief The number of status signals the BMS exposes.
	uint16_t statusSignalsCount;

	/// @brief The minimum nominal cell voltage.
	float minCellVoltage;
	/// @brief The maximum nominal cell voltage.
	float maxCellVoltage;
	/// @brief The minimum nominal sense line temperature.
	float minTemperature;
	/// @brief The maximum nominal sense line temperature.
	float maxTemperature;
	/// @brief The maximum nominal LTC temperature.
	float maxLtcTemperature;

	/// @brief Array of cell voltage global signal indices.
	ssize_t* cellVoltageIndices;
	/// @brief Array of cells discharging global signal indices.
	ssize_t* cellsDischargingIndices;
	/// @brief Array of sense line temperature global signal indices.
	ssize_t* senseLineTemperatureIndices;
	/// @brief Array of sense line status global signal indices.
	ssize_t* senseLinesOpenIndices;
	/// @brief Array of LTC IsoSPI fault global signal indices.
	ssize_t* ltcIsoSpiFaultIndices;
	/// @brief Array of LTC self test fault global signal indices.
	ssize_t* ltcSelfTestFaultIndices;
	/// @brief Array of LTC temperature global signal indices.
	ssize_t* ltcTemperatureIndices;
	/// @brief Array of BMS status signal global indices.
	ssize_t* statusSignalIndices;
	/// @brief Global index of the pack voltage signal.
	ssize_t packVoltageIndex;
	/// @brief Global index of the pack current signal.
	ssize_t packCurrentIndex;
	/// @brief Global indices of the logical (gapless) temperatures.
	ssize_t* logicalTemperatureIndices;
	/// @brief Number of elements in @c logicalTemperatureIndices .
	size_t logicalTemperatureCount;

} bms_t;

// Functions ------------------------------------------------------------------------------------------------------------------

/**
 * @brief Initializes a BMS interface.
 * @param bms The BMS to initialize.
 * @param config The configuration JSON to use.
 * @param database The CAN database to bind to.
 * @return 0 if successful, the error code otherwise. Note @c errno is set on error.
 */
int bmsInit (bms_t* bms, cJSON* config, canDatabase_t* database);

/**
 * @brief Gets the pack voltage of the BMS.
 * @param bms The BMS to use.
 * @param voltage Buffer to write the voltage into.
 * @return The state of the signal. Note that @c voltage is only written if the return is @c CAN_DATABASE_VALID .
 */
canDatabaseSignalState_t bmsGetPackVoltage (bms_t* bms, float* voltage);

/**
 * @brief Gets the pack current of the BMS.
 * @param bms The BMS to use.
 * @param current Buffer to write the current into.
 * @return The state of the signal. Note that @c current is only written if the return is @c CAN_DATABASE_VALID .
 */
canDatabaseSignalState_t bmsGetPackCurrent (bms_t* bms, float* current);

/**
 * @brief Gets the pack power consumption of the BMS.
 * @param bms The BMS to use.
 * @param power Buffer to write the power into.
 * @return The state of the signal. Note that @c power is only written if the return is @c CAN_DATABASE_VALID .
 */
canDatabaseSignalState_t bmsGetPackPower (bms_t* bms, float* power);

/**
 * @brief Gets a cell voltage from the BMS.
 * @param bms The BMS to use.
 * @param index The global index of the cell to get.
 * @param voltage Buffer to write the voltage into.
 * @return The state of the signal. Note that @c voltage is only written if the return is @c CAN_DATABASE_VALID .
 */
canDatabaseSignalState_t bmsGetCellVoltage (bms_t* bms, size_t index, float* voltage);

/**
 * @brief Gets a cell's discharging status from the BMS.
 * @param bms The BMS to use.
 * @param index The global index of the cell to get.
 * @param discharging Buffer to write the status into. True if discharging, false otherwise.
 * @return The state of the signal. Note that @c discharging is only written if the return is @c CAN_DATABASE_VALID .
 */
canDatabaseSignalState_t bmsGetCellDischarging (bms_t* bms, size_t index, bool* discharging);

/**
 * @brief Gets a sense line's temperature from the BMS.
 * @param bms The BMS to use.
 * @param index The global index of the sense line to get.
 * @param temperature Buffer to write the temperature into.
 * @return The state of the signal. Note that @c temperature is only written if the return is @c CAN_DATABASE_VALID .
 */
canDatabaseSignalState_t bmsGetSenseLineTemperature (bms_t* bms, size_t index, float* temperature);

/**
 * @brief Gets a sense line's status from the BMS.
 * @param bms The BMS to use.
 * @param index The global index of the sense line to get.
 * @param open Buffer to write the status into. True if open, false otherwise.
 * @return The state of the signal. Note that @c open is only written if the return is @c CAN_DATABASE_VALID .
 */
canDatabaseSignalState_t bmsGetSenseLineOpen (bms_t* bms, size_t index, bool* open);

/**
 * @brief Gets a logical temperature from the BMS. Logical temperatures are a "gapless" version of the sense line temperatures,
 * that is, no missing signals are present in this.
 * @param bms The BMS to use.
 * @param index The global index of the logical temperature to get.
 * @param temperature Buffer to write the temperature into.
 * @return The state of the signal. Note that @c temperature is only written if the return is @c CAN_DATABASE_VALID .
 */
canDatabaseSignalState_t bmsGetLogicalTemperature (bms_t* bms, size_t index, float* temperature);

/**
 * @brief Gets the number of logical temperatures in the BMS. Logical temperatures are a "gapless" version of the sense line
 * temperatures, that is, no missing signals are present in this.
 * @param bms The BMS to use.
 * @return The number of elements in the array.
 */
static inline size_t bmsGetLogicalTemperatureCount (bms_t* bms)
{
	return bms->logicalTemperatureCount;
}

/**
 * @brief Gets the state of an LTC from the BMS.
 * @param bms The BMS to use.
 * @param index The global index of the LTC to get.
 * @return The state of the LTC.
 */
bmsLtcState_t bmsGetLtcState (bms_t* bms, size_t index);

/**
 * @brief Gets an LTC's temperature from the BMS.
 * @param bms The BMS to use.
 * @param index The global index of the LTC to get.
 * @param temperature Buffer to write the temperature into.
 * @return The state of the signal. Note that @c temperature is only written if the return is @c CAN_DATABASE_VALID .
 */
canDatabaseSignalState_t bmsGetLtcTemperature (bms_t* bms, size_t index, float* temperature);

/**
 * @brief Gets summary statistics of all cell voltages.
 * @param bms The BMS to use.
 * @param min Buffer to write the minimum cell voltage into, or @c NULL to ignore.
 * @param max Buffer to write the maximum cell voltage into, or @c NULL to ignore.
 * @param avg Buffer to write the average cell voltage into, or @c NULL to ignore.
 * @return True if the statistics were written, false otherwise.
 */
bool bmsGetCellVoltageStats (bms_t* bms, float* min, float* max, float* avg);

/**
 * @brief Gets summary statistics of all cell voltage deltas. A cell's delta is the difference between its voltage and the
 * minimum cell's voltage.
 * @param bms The BMS to use.
 * @param max Buffer to write the maximum cell delta into, or @c NULL to ignore.
 * @param avg Buffer to write the average cell delta into, or @c NULL to ignore.
 * @return True if the statistics were written, false otherwise.
 */
bool bmsGetCellDeltaStats (bms_t* bms, float* max, float* avg);

/**
 * @brief Gets summary statistics of all sense line temperatures.
 * @param bms The BMS to use.
 * @param min Buffer to write the minimum temperature voltage into, or @c NULL to ignore.
 * @param max Buffer to write the maximum temperature voltage into, or @c NULL to ignore.
 * @param avg Buffer to write the average temperature voltage into, or @c NULL to ignore.
 * @return True if the statistics were written, false otherwise.
 */
bool bmsGetTemperatureStats (bms_t* bms, float* min, float* max, float* avg);

/**
 * @brief Deallocates the memory used by a BMS.
 * @param bms The BMS to deallocate.
 */
void bmsDealloc (bms_t* bms);

/**
 * @brief Gets the number of status signals a BMS exposes.
 * @param bms The BMS to use.
 * @return The number of status signals.
 */
static inline size_t bmsGetStatusCount (bms_t* bms)
{
	return bms->statusSignalsCount;
}

/**
 * @brief Gets the name of a BMS's status signal.
 * @param bms The BMS to use.
 * @param index The index of the signal.
 * @return The name of the signal.
 */
static inline char* bmsGetStatusName (bms_t* bms, size_t index)
{
	ssize_t globalIndex = bms->statusSignalIndices [index];
	canSignal_t* signal = canDatabaseGetSignal (bms->database, globalIndex);
	if (signal == NULL)
		return NULL;

	return signal->name;
}

/**
 * @brief Gets the unit associated with a BMS's status signal.
 * @param bms The BMS to use.
 * @param index The index of the signal.
 * @return The unit of the signal.
 */
static inline char* bmsGetStatusUnit (bms_t* bms, size_t index)
{
	ssize_t globalIndex = bms->statusSignalIndices [index];
	canSignal_t* signal = canDatabaseGetSignal (bms->database, globalIndex);
	if (signal == NULL)
		return NULL;

	return signal->unit;
}

/**
 * @brief Gets the value of a BMS's status signal.
 * @param bms The BMS to use.
 * @param index The index of the signal.
 * @param value Buffer to write the signal into.
 * @return The state of the status signal. Note @c value is only written if the return is @c CAN_DATABASE_VALID .
 */
static inline canDatabaseSignalState_t bmsGetStatusValue (bms_t* bms, size_t index, float* value)
{
	// Get the value of the signal
	ssize_t globalIndex = bms->statusSignalIndices[index];
	return canDatabaseGetFloat (bms->database, globalIndex, value);
}

/**
 * @brief Prints the index of a sense line into a string. Note that the textual index of a sense line is not simply the numeric
 * representation of its global index, rather it "doubles up" on the boundaries of each LTC.
 * @param bms The BMS to print an index of.
 * @param index The global sense line index to print.
 * @param str The string to print into.
 * @param n The maximum number of characters to write, including the terminator.
 * @return The number of characters written (excluding the terminator) if successful, or 0 if truncation occurred.
 */
size_t bmsSnprintSenseLineIndex (bms_t* bms, size_t index, char* str, size_t n);

/**
 * @brief Gets the index of the LO (negative) sense-line of a cell.
 * @param bms The BMS to use.
 * @param cellIndex The global index of the cell.
 * @return The global index of the sense line.
 */
static inline size_t bmsGetCellLoSenseLineIndex (bms_t* bms, size_t cellIndex)
{
	size_t ltcIndex = cellIndex / bms->cellsPerLtc;
	size_t cellLtcIndex = cellIndex % bms->cellsPerLtc;
	return cellLtcIndex + ltcIndex * bms->senseLinesPerLtc;
}

/**
 * @brief Gets the index of the HI (positive) sense-line of a cell.
 * @param bms The BMS to use.
 * @param cellIndex The global index of the cell.
 * @return The global index of the sense line.
 */
static inline size_t bmsGetCellHiSenseLineIndex (bms_t* bms, size_t cellIndex)
{
	return bmsGetCellLoSenseLineIndex (bms, cellIndex) + 1;
}

// TODO(Barach): Doesn't work.
/**
 * @brief Gets the global index of a sense line from a logical temperature index.
 * @param bms The BMS to use.
 * @param index The logical temperature index.
 * @return The global sense line index.
 */
static inline ssize_t bmsGetLogicalTemperatureIndex (bms_t* bms, size_t index)
{
	return bms->logicalTemperatureIndices [index];
}

#endif // BMS_H