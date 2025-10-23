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

// C Standard Library
#include <stdio.h>
#include <time.h>

// Functions ------------------------------------------------------------------------------------------------------------------

int mdfCanBusLogInit (FILE* mdf, const char* programId, time_t unixTime);

int mdfCanBusLogWriteDataFrame (FILE* mdf, canFrame_t* frame, uint64_t timestamp, uint8_t busChannel);

#endif // MDF_CAN_BUS_LOGGING