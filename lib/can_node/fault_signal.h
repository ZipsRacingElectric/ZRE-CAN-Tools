#ifndef FAULT_SIGNAL_H
#define FAULT_SIGNAL_H

// TODO(Barach): Docs

// Includes
#include "can_database/can_database.h"
#include "cjson/cjson.h"

// C Standard Library
#include <stdbool.h>
#include <stdlib.h>

// Datatypes ------------------------------------------------------------------------------------------------------------------

/// @brief Enum representing the ways in which a CAN signal can be faulted.
typedef enum
{
	/// @brief Indicates the signal is missing from the CAN database.
	FAULT_SIGNAL_MISSING,

	/// @brief Indicates the signal has timed out.
	FAULT_SIGNAL_TIMEOUT,

	/// @brief Indicates the signal reads a fault.
	FAULT_SIGNAL_FAULTED,

	/// @brief Indicates the signal reads no fault.
	FAULT_SIGNAL_OKAY
} faultSignalState_t;

/// @brief Object representing a CAN signal that represents a fault condition.
typedef struct
{
	/// @brief User-friendly name of the fault.
	char* name;

	/// @brief Index of the source signal.
	ssize_t index;

	/// @brief Threshold of fault activation.
	float threshold;

	/// @brief False, fault present if signal > threshold. True, fault is present if signal < threshold.
	bool inverted;
} faultSignal_t;

/// @brief Object representing an array of fault signals.
typedef struct
{
	canDatabase_t* database;
	faultSignal_t* signals;
	size_t count;
} faultSignals_t;

// Functions ------------------------------------------------------------------------------------------------------------------

/**
 * @brief Loads an array of fault signals from a JSON configuration.
 * @param faults The array of faults to initialize.
 * @param config The JSON configuration to load from.
 * @param database The CAN database the faults are to be bound to.
 * @return 0 if successful, the error code otherwise.
 */
int faultSignalsLoad (faultSignals_t* faults, cJSON* config, canDatabase_t* database);

/**
 * @brief Deallocates a fault signal array loaded by @c faultSignalsLoad .
 * @param faults The fault signal array to deallocate.
 */
void faultSignalsDealloc (faultSignals_t* faults);

/**
 * @brief Gets the state of an array of fault signals.
 * @param faults The array of fault signals to check.
 * @param index Buffer to write the index of the offending signal, if any.
 * @return The state of the array of signals. If @c FAULT_SIGNAL_FAULTED , @c FAULT_SIGNAL_TIMEOUT ,
 * or @c FAULT_SIGNAL_MISSING , the @c index is written to indicate the offending signal.
 */
faultSignalState_t faultSignalsGetIndex (faultSignals_t* faults, size_t* index);

/**
 * @brief Gets user-friendly text describing the status of an array of fault signals.
 * @param faults The array of fault signals to search.
 * @return The string describing the fault.
 */
char* faultSignalsGetString (faultSignals_t* faults);

#endif // FAULT_SIGNAL_H