#ifndef ABS_SIGNAL_H
#define ABS_SIGNAL_H

// Absolute Axis Input via a CAN Signals --------------------------------------------------------------------------------------
//
// Author: Cole Barach
// Date Created: 2025.11.23
//
// Description: Objects and functions for mapping CAN bus signals to an absolute axis input. An absolute axis is an analog
//   input that has an 'absolute' value associated with it. Examples of these inputs are the X and Y axes of a joystick.

// Includes -------------------------------------------------------------------------------------------------------------------

// Includes
#include "can_database/can_database.h"
#include "cjson/cjson.h"

// Objects --------------------------------------------------------------------------------------------------------------------

/// @brief Config structure for the @c absSignal_t structure.
typedef struct
{
	/// @brief Name of the CAN signal controlling the axis's positive values.
	const char* positiveSignalName;
	/// @brief Name of the CAN signal controlling the axis's negative values. Use @c NULL if this axis is only positive.
	const char* negativeSignalName;
	/// @brief Input code of the absolute axis to control. Ex @c ABS_X , @c ABS_Y , @c ABS_Z
	int code;
	/// @brief Value of the positive CAN signal that should map to zero input.
	float positiveZero;
	/// @brief Value of the positive CAN signal that should map to the positive-most input.
	float positiveMax;
	/// @brief Value of the negative CAN signal that should map to zero input. Ignored if @c negativeSignalName is @c NULL .
	float negativeZero;
	/// @brief Value of the negative CAN signal that should map to the negative-most input. Ignored if @c negativeSignalName
	/// is @c NULL .
	float negativeMax;
} absSignalConfig_t;

/// @brief Object representing an absolute axis controlled via CAN bus.
typedef struct
{
	absSignalConfig_t config;
	int fd;
	canDatabase_t* database;
	ssize_t positiveSignal;
	ssize_t negativeSignal;
} absSignal_t;

// Functions ------------------------------------------------------------------------------------------------------------------

/**
 * @brief Initializes an absolute axis controlled via CAN bus.
 * @param abs The axis to initialize.
 * @param config The configuration to use.
 * @param fd The file descriptor of the uinput file.
 * @param database The CAN database the CAN signal belongs to.
 * @return 0 if successful, the error code otherwise.
 */
int absSignalInit (absSignal_t* abs, absSignalConfig_t* config, int fd, canDatabase_t* database);

/**
 * @brief Loads and initializes an array of absolute axes from a JSON configuration file.
 * @param config The configuration JSON.
 * @param fd The file descriptor of the uinput file.
 * @param database The CAN database the CAN signals belong to.
 * @param count Buffer to write the number of loaded axes into.
 * @return Dynamically allocated array of absolute axes.
 */
absSignal_t* absSignalsLoad (cJSON* config, int fd, canDatabase_t* database, size_t* count);

/**
 * @brief Updates the input of an absolute axis based on the current CAN bus state. Note the update will note take affect until
 * the sync event has been reported.
 * @param abs The axis to update.
 * @return 0 if successful, the error code otherwise.
 */
int absSignalUpdate (absSignal_t* abs);

#endif // ABS_SIGNAL_H