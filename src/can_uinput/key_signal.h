#ifndef KEY_SIGNAL_H
#define KEY_SIGNAL_H

// Key Input via a CAN Bus Signal ---------------------------------------------------------------------------------------------
//
// Author: Cole Barach
// Date Created: 2025.11.23
//
// Description: Objects and functions for mapping a CAN bus signal to a key input. A key input refers to any digital input, for
//   instance the keys on a keyboard or buttons on a controller.

// Includes -------------------------------------------------------------------------------------------------------------------

// Includes
#include "can_database/can_database.h"
#include "cjson/cjson.h"

// C Standard Library
#include <stdbool.h>

// Objects --------------------------------------------------------------------------------------------------------------------

/// @brief Configuration for the @c keySignal_t structure.
typedef struct
{
	/// @brief The name of the CAN signal controlling the input.
	const char* signalName;
	/// @brief The key code of the input.
	int code;
	/// @brief The threshold at which the key is considered to be depressed. The key is considered pressed if the CAN signal's
	/// value exceeds this value.
	float threshold;
	/// @brief Indicates if the depressed and un-depressed states of the key should be swapped.
	bool inverted;
} keySignalConfig_t;

/// @brief Object representing a key controlled via a CAN bus signal.
typedef struct
{
	int fd;
	canDatabase_t* database;
	keySignalConfig_t config;
	ssize_t index;
	bool pressed;
} keySignal_t;

// Functions ------------------------------------------------------------------------------------------------------------------

/**
 * @brief Initializes a key bound to a CAN bus signal.
 * @param key The key to initialize.
 * @param config The configuration to use.
 * @param fd The file descriptor of the uinput file.
 * @param database The CAN database the CAN signal belongs to.
 * @return 0 if successful, the error code otherwise.
 */
int keySignalInit (keySignal_t* key, keySignalConfig_t* config, int fd, canDatabase_t* database);

/**
 * @brief Loads and initializes an array of keys from a JSON configuration file.
 * @param config The configuration JSON.
 * @param fd The file descriptor of the uinput file.
 * @param database The CAN database the CAN signal belongs to.
 * @param count Buffer to write the number of loaded keys into.
 * @return Dynamically allocated array of keys.
 */
keySignal_t* keySignalsLoad (cJSON* json, int fd, canDatabase_t* database, size_t* count);

/**
 * @brief Updates the input of a key based on the current CAN bus state. Note the update will note take affect until the sync
 * event has been reported.
 * @param key The key to update.
 * @return 0 if successful, the error code otherwise.
 */
int keySignalUpdate (keySignal_t* key);

#endif // KEY_SIGNAL_H