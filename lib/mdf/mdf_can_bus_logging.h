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
#include <stdio.h>
#include <time.h>

// Datatypes ------------------------------------------------------------------------------------------------------------------

typedef struct
{
	/// @brief The path of the file to create.
	char* filePath;

	/// @brief The ID of the program generating this file. Note, strings longer than 7 characters will be truncated.
	char* programId;

	/// @brief Tag for the version of the software generating this file.
	char* softwareVersion;

	/// @brief Tag for the version of the hardware generating this file.
	char* hardwareVersion;

	/// @brief Serial number of the device generating this file.
	char* serialNumber;

	/// @brief The baudrate of the first CAN bus.
	uint32_t channel1Baudrate;

	/// @brief The baudrate of the second CAN bus.
	uint32_t channel2Baudrate;

	/// @brief The time at which this file was created.
	time_t timeStart;

	/// @brief The total capacity of the storage device containing this file, in bytes.
	size_t storageSize;

	/// @brief The remaining capacity of the storage device containing this file, in bytes.
	size_t storageRemaining;

	/// @brief The index of this data log, starting from 0 and incrementing monotonically.
	uint32_t sessionNumber;

	/// @brief The index of the split of this data log. When a log is split due to size, the session number should remaing the
	/// same, but the split number should be incremented. Should start from 1.
	uint32_t splitNumber;
} mdfCanBusLogConfig_t;

typedef struct
{
	const mdfCanBusLogConfig_t* config;
	FILE* mdf;
} mdfCanBusLog_t;

// Functions ------------------------------------------------------------------------------------------------------------------

/**
 * @brief Initializes an MDF CAN bus log. This will create the log file based on the provided file path in the @c config
 * structure. Note all entries of @c config must be populated.
 * @param log The log file to initialize.
 * @param config The configuration to use.
 * @return 0 if successful, the error code otherwise.
 */
int mdfCanBusLogInit (mdfCanBusLog_t* log, const mdfCanBusLogConfig_t* config);

/**
 * @brief Writes a CAN data frame to an MDF log.
 * @param log The log to write to.
 * @param frame The data frame to write.
 * @param busChannel The CAN bus channel the frame originated from.
 * @param timestamp The time at which the frame arrived.
 * @return 0 if successful, the error code otherwise.
 */
int mdfCanBusLogWriteDataFrame (mdfCanBusLog_t* log, canFrame_t* frame, uint8_t busChannel, struct timeval* timestamp);

/**
 * @brief Closes a an MDF log.
 * @param log The log to close.
 * @return 0 if successful, the error code otherwise.
 */
int mdfCanBusLogClose (mdfCanBusLog_t* log);

#endif // MDF_CAN_BUS_LOGGING