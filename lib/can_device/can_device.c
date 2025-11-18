// Header
#include "can_device.h"

// Includes
#include "debug.h"
#include "error_codes.h"
#include "socket_can.h"
#include "slcan.h"

// C Standard Library
#include <errno.h>
#include <stdlib.h>

/**
 * @brief Splits a device name into the device handle and baudrate.
 * @param deviceName The provided device name. Expected form: "<Device Handle>@<Baudrate>". Modified to contain the device
 * handle on exit.
 * @param baudrate Buffer to write the baudrate into. Writted to be @c CAN_BAUDRATE_UNKNOWN if the baudrate is not specified.
 * @return 0 if successful, the error code otherwise.
 */
static int parseDeviceName (char* deviceName, unsigned int* baudrate)
{
	// Split at the '@' character.
	char* savePtr;
	strtok_r (deviceName, "@", &savePtr);
	char* baudrateStr = strtok_r (NULL, "", &savePtr);
	if (baudrateStr != NULL)
	{
		// If a baudrate was specified, parse it.
		char* end;
		*baudrate = strtoul (baudrateStr, &end, 0);
		if (end == baudrateStr)
		{
			// Failed to parse.
			errno = EINVAL;
			return errno;
		}
	}
	else
	{
		// No baudrate specified.
		debugPrintf ("Warning: CAN device did not specify baudrate.\n");
		*baudrate = CAN_BAUDRATE_UNKNOWN;
	}

	// Success
	return 0;
}

canDevice_t* canInit (char* deviceName)
{
	// Split the device name into the device handle and baudrate (if specified).
	canBaudrate_t baudrate;
	if (parseDeviceName (deviceName, &baudrate) != 0)
		return NULL;

	// Handle SocketCAN device
	if (socketCanNameDomain (deviceName))
		return socketCanInit (deviceName, baudrate);

	// Handle SLCAN device
	if (slcanNameDomain (deviceName))
		return slcanInit(deviceName, baudrate);

	// Unknown device
	errno = ERRNO_CAN_DEVICE_UNKNOWN_NAME;
	return NULL;
}

bool canCheckBusError (int code)
{
	return
		code == ERRNO_CAN_DEVICE_BIT_ERROR ||
		code == ERRNO_CAN_DEVICE_BIT_STUFF_ERROR ||
		code == ERRNO_CAN_DEVICE_FORM_ERROR ||
		code == ERRNO_CAN_DEVICE_ACK_ERROR ||
		code == ERRNO_CAN_DEVICE_CRC_ERROR ||
		code == ERRNO_CAN_DEVICE_BUS_OFF ||
		code == ERRNO_CAN_DEVICE_UNSPEC_ERROR;
}

char* canGetBusErrorName (int code)
{
	if (code == ERRNO_CAN_DEVICE_BIT_ERROR)
		return "BIT ERROR";

	if (code == ERRNO_CAN_DEVICE_BIT_STUFF_ERROR)
		return "BIT STUFF ERROR";

	if (code == ERRNO_CAN_DEVICE_FORM_ERROR)
		return "FORM ERROR";

	if (code == ERRNO_CAN_DEVICE_ACK_ERROR)
		return "ACK ERROR";

	if (code == ERRNO_CAN_DEVICE_CRC_ERROR)
		return "CRC ERROR";

	if (code == ERRNO_CAN_DEVICE_BUS_OFF)
		return "BUS-OFF ERROR";

	return "UNSPECIFIED ERROR";
}

// REVIEW(Barach): Temporarily removed until finalized.
// canDevice_t* enumerateDevice (char* baudRate)
// {
// 	char* deviceNames [5];
// 	size_t deviceCount = 0;

// 	if (slcanenumerateDevice (deviceNames, &deviceCount, "1000000") == 0)
// 	{
// 		char name [128];
// 		canDevice_t* device;

// 		for (int i = 0; i < deviceCount; ++i)
// 		{
// 			device = canInit (deviceNames[i]);

// 			if (device == NULL)
// 			{
// 				continue;
// 			}

// 			return device;
// 		}
// 	}
// 	else
// 	{
// 		// Indicates that the program failed to locate a CAN device
// 		errno = ERRNO_CAN_DEVICE_MISSING_DEVICE;
// 	}

// 	// Indicates that a CAN device could not be created
// 	return NULL;
// }