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
 * @brief Prompts the user to select a CAN device from a list of enumerated options.
 * @param devices The array of CAN devices to select from.
 * @param deviceCount The number of elements in @c devices .
 * @return The index of the selected device.
 */
static size_t selectDevice (canDevice_t** devices, size_t deviceCount)
{
	// If only one device is present, no need to prompt.
	if (deviceCount == 1)
		return 0;

	char buffer [512];

	while (true)
	{
		// Prompt for a device to select
		printf ("Select a device:\n");
		for (unsigned deviceIndex = 0; deviceIndex < deviceCount; ++deviceIndex)
			printf ("  %u - %s\n", deviceIndex, canGetDeviceName (devices [deviceIndex]));

		// Read the user input
		fgets (buffer, sizeof (buffer), stdin);
		buffer [strcspn (buffer, "\r\n")] = '\0';

		// Parse as int and validate
		char* end;
		size_t selection = (size_t) strtol (buffer, &end, 0);
		if (end == buffer || selection >= deviceCount)
		{
			// Prompt again
			printf ("Invalid selection.\n");
			continue;
		}

		return selection;
	}
}

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
	{
		// If a wildcard is present, use device enumeration
		if (slcanWildcard (deviceName))
		{
			// Enumerate all possible devices
	 		size_t deviceCount;
	 		canDevice_t** devices = slcanEnumerate (baudrate, &deviceCount);
	 		if (devices == NULL)
	 			return NULL;

			// Get the user's selection
	 		size_t index = selectDevice (devices, deviceCount);
			canDevice_t* device = devices [index];

			// Deallocate unused devices
			for (size_t i = 0; i < deviceCount; ++i)
			{
				if (i != index)
					slcanDealloc (devices [i]);
			}

			// Deallocate the array of devices
			free (devices);

			// Return the selected device
	 		return device;
	 	}

		// Otherwise, use normal initialization
		return slcanInit(deviceName, baudrate);
	 }

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