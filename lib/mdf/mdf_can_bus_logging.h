#ifndef MDF_CAN_BUS_LOGGING
#define MDF_CAN_BUS_LOGGING

// MDF CAN Bus Logging --------------------------------------------------------------------------------------------------------
//
// Author: Cole Barach
// Date Created: 2025.10.22
//
// Description: Implementation of the ASAM MDF Bus Logging Standard. // TODO(Barach): Better description

// Includes -------------------------------------------------------------------------------------------------------------------

// Includes
#include "can_device/can_device.h"

// POSIX
#include <sys/time.h>

// C Standard Library
#include <errno.h>
#include <stdio.h>
#include <time.h>

// Datatypes ------------------------------------------------------------------------------------------------------------------

typedef struct
{
	// TODO(Barach):
	const char* directory;

	const char* configurationName;

	const char* softwareName;

	/// @brief Tag for the version of the software generating this file.
	const char* softwareVersion;

	const char* softwareVendor;

	/// @brief Tag for the name of the hardware generating this file.
	const char* hardwareName;

	/// @brief Tag for the version of the hardware generating this file.
	const char* hardwareVersion;

	/// @brief Serial number of the device generating this file.
	const char* serialNumber;

	/// @brief The baudrate of the first CAN bus.
	uint32_t channel1Baudrate;

	/// @brief The baudrate of the second CAN bus.
	uint32_t channel2Baudrate;

	/// @brief The total capacity of the storage device containing this file, in bytes.
	size_t storageSize;

	/// @brief The remaining capacity of the storage device containing this file, in bytes.
	size_t storageRemaining;

	/// @brief The index of this data log, starting from 0 and incrementing monotonically.
	uint32_t sessionNumber;
} mdfCanBusLogConfig_t;

typedef struct
{
	// TODO(Barach)
	const mdfCanBusLogConfig_t* config;
	FILE* mdf;
	uint32_t splitNumber;
	size_t splitSize;
	char* splitName;
	time_t dateStart;
	struct timespec timeStart;
} mdfCanBusLog_t;

// Functions ------------------------------------------------------------------------------------------------------------------

/**
 * @brief Gets the assumed session number a log should start at based on the entries in a parent directory. This is done by
 * finding the highest session (ex, 'session_10') and incrementing this by 1.
 * @param directory The parent directory to search in.
 * @return The determined session number.
 */
uint32_t mdfCanBusLogGetSessionNumber (const char* directory);

/**
 * @brief Initializes an MDF CAN bus log. This will create the log file based on the provided file path in the @c config
 * structure. Note all entries of @c config must be populated.
 * @param log The log file to initialize.
 * @param config The configuration to use.
 * @return 0 if successful, the error code otherwise.
 */
int mdfCanBusLogInit (mdfCanBusLog_t* log, const mdfCanBusLogConfig_t* config);

/**
 * @brief Gets the name of the current logging file.
 * @param log The log file to get from.
 * @return The name of the current split file.
 */
const char* mdfCanBusLogGetName (mdfCanBusLog_t* log);

/**
 * @brief Gets a timestamp used by an MDF log.
 * @param timestamp Buffer to write the timestamp into.
 * @return 0 if successful, the error code otherwise.
 */
static inline int mdfCanBusLogGetTimestamp (struct timespec* timestamp)
{
	if (clock_gettime (CLOCK_MONOTONIC, timestamp) != 0)
		return errno;
	return 0;
}

/**
 * @brief Gets the resolution of the MDF log timer.
 * @param resolution Buffer to write the resolution into.
 * @return 0 if successful, the error code otherwise.
 */
static inline int mdfCanBusLogGetTimeResolution (struct timespec* resolution)
{
	if (clock_getres (CLOCK_MONOTONIC, resolution) != 0)
		return errno;
	return 0;
}

/**
 * @brief Writes a CAN data frame to an MDF log. Note this function is not thread-safe, in order to use it a multithreaded
 * context, a mutex must be employed.
 * @param log The log to write to.
 * @param frame The data frame to write.
 * @param busChannel The CAN bus channel the frame originated from. Either 1 or 2.
 * @param direction The direction of the frame. False => received, true => transmitted.
 * @param timestamp The time at which the frame arrived, as acquired by @c mdfCanBusLogGetTimestamp .
 * @return 0 if successful, the error code otherwise.
 */
int mdfCanBusLogWriteDataFrame (mdfCanBusLog_t* log, canFrame_t* frame, uint8_t busChannel, bool direction,
	struct timespec* timestamp);

/**
 * @brief Writes an CAN RTR frame to an MDF log. Note this function is not thread-safe, in order to use it a multithreaded
 * context, a mutex must be employed.
 * @param log The log to write to.
 * @param frame The RTR frame to write.
 * @param busChannel The CAN bus channel the frame originated from. Either 1 or 2.
 * @param direction The direction of the frame. False => received, true => transmitted.
 * @param timestamp The time at which the frame arrived, as acquired by @c mdfCanBusLogGetTimestamp .
 * @return 0 if successful, the error code otherwise.
 */
int mdfCanBusLogWriteRemoteFrame (mdfCanBusLog_t* log, canFrame_t* frame, uint8_t busChannel, bool direction,
	struct timespec* timestamp);

/**
 * @brief Writes a CAN error frame to an MDF log. Note this function is not thread-safe, in order to use it a multithreaded
 * context, a mutex must be employed.
 * @param log The log to write to.
 * @param frame The error frame to write.
 * @param busChannel The CAN bus channel the frame originated from. Either 1 or 2.
 * @param direction The direction of the frame. False => received, true => transmitted.
 * @param errorCode The error code that is associated with the frame (as returned by @c canReceive and checked by
 * @c canCheckBusError ).
 * @param timestamp The time at which the frame was generated, as acquired by @c mdfCanBusLogGetTimestamp .
 * @return 0 if successful, the error code otherwise.
 */
int mdfCanBusLogWriteErrorFrame (mdfCanBusLog_t* log, canFrame_t* frame, uint8_t busChannel, bool direction, int errorCode,
	struct timespec* timestamp);

/**
 * @brief Closes a an MDF log. Note this function is not thread-safe, in order to use it a multithreaded context, a mutex must
 * be employed.
 * @param log The log to close.
 * @return 0 if successful, the error code otherwise.
 */
int mdfCanBusLogClose (mdfCanBusLog_t* log);

#endif // MDF_CAN_BUS_LOGGING