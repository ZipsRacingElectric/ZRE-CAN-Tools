#ifndef FAULT_SIGNAL_H
#define FAULT_SIGNAL_H

// TODO(Barach): Docs

// Includes
#include "can_database/can_database.h"
#include "cjson/cjson.h"

// C Standard Library
#include <stdbool.h>
#include <stdlib.h>

typedef enum
{
	FAULT_SIGNAL_MISSING,
	FAULT_SIGNAL_TIMEOUT,
	FAULT_SIGNAL_FAULTED,
	FAULT_SIGNAL_OKAY
} faultSignalState_t;

typedef struct
{
	/// @brief User-friendly name of the fault.
	char* name;

	/// @brief Index of the source signal.
	ssize_t index;

	/// @brief Threshold of fault activation.
	float threshold;

	/// @brief Indicates fault activation is inverted.
	bool inverted;
} faultSignal_t;

/**
 * @brief Loads a fault signal from a JSON configuration.
 * @param signal The signal to initialize.
 * @param config The JSON element to load the configuration from.
 * @param database The CAN database to bind to.
 * @return 0 if successful, the error code otherwise.
 */
int faultSignalLoad (faultSignal_t* signal, cJSON* config, canDatabase_t* database);

/**
 * @brief Loads an array of fault signals from a JSON configuration.
 * @param configArray The JSON array to load the configurations from.
 * @param count Buffer to write the number of signals into.
 * @param database The CAN database to bind to.
 * @return The dynamically allocated array of signals, if successful, @c NULL otherwise.
 */
faultSignal_t* faultSignalsLoad (cJSON* configArray, size_t* count, canDatabase_t* database);

/**
 * @brief Checks whether a fault signal is faulted or not.
 * @param database The database to source the signal from.
 * @param signal The fault signal to check.
 * @return The state of the fault.
 */
faultSignalState_t faultSignalCheck (canDatabase_t* database, faultSignal_t* signal);

/**
 * @brief Checks whether any fault in an array is fault or not.
 * @param database The database to source the signals from.
 * @param signals The array of fault signals to check.
 * @param signalCount The number of elements in the @c signals array.
 * @return The name of the first faulted signal, if any. "TIMEOUT" if a timed out signal was found. "MISSING" if a missing
 * signal was found. "OKAY" if no faults were found.
 */
char* faultSignalsCheck (canDatabase_t* database, faultSignal_t* signals, size_t signalCount);

#endif // FAULT_SIGNAL_H