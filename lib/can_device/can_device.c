// Header
#include "can_device.h"

// Includes
#include "debug.h"
#include "error_codes.h"

// CAN device implementations
#include "socket_can.h"
#include "slcan.h"
#include "can_null.h"

// C Standard Library
#include <errno.h>
#include <stdlib.h>

/**
 * @brief Prompts the user to select a CAN device from a list of enumerated options.
 * @param devices The array of CAN devices to select from.
 * @param deviceCount The number of elements in @c devices .
 * @param baudrate The baudrate to use (if the null device is selected).
 * @param userContext String to provide for user context. May be @c NULL .
 * @return The selected device. Note all other devices must be deallocated.
 */
static canDevice_t* selectDevice (canDevice_t** devices, size_t deviceCount, canBaudrate_t baudrate, char* userContext)
{
	char buffer [512];

	while (true)
	{
		// Prompt for a device to select
		if (userContext == NULL)
			printf ("Select a device:\n");
		else
			printf ("Select a device to use for %s:\n", userContext);

		for (unsigned deviceIndex = 0; deviceIndex < deviceCount; ++deviceIndex)
			printf ("  %u - %s\n", deviceIndex, canGetDeviceName (devices [deviceIndex]));

		printf ("  %u - none\n", (unsigned) deviceCount);

		// Read the user input
		fgets (buffer, sizeof (buffer), stdin);
		buffer [strcspn (buffer, "\r\n")] = '\0';

		// Parse as int and validate
		char* end;
		size_t selection = (size_t) strtol (buffer, &end, 0);
		if (end == buffer || selection > deviceCount)
		{
			// Prompt again
			printf ("Invalid selection.\n");
			continue;
		}

		// If 'none' was selected, return a null CAN device.
		if (selection == deviceCount)
			return canNullInit ("null", baudrate);

		return devices [selection];
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

canDevice_t* canInit (char* deviceName, char* userContext)
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
	 		size_t deviceCount = 0;
	 		canDevice_t** devices = slcanEnumerate (baudrate, &deviceCount);

			// Get the user's selection
	 		canDevice_t* device = selectDevice (devices, deviceCount, baudrate, userContext);

			if (devices != NULL)
			{
				// Deallocate unused devices
				for (size_t index = 0; index < deviceCount; ++index)
				{
					if (devices [index] != device)
						slcanDealloc (devices [index]);
				}

				// Deallocate the array of devices
				free (devices);
			}

			// Return the selected device
	 		return device;
	 	}

		// Otherwise, use normal initialization
		return slcanInit(deviceName, baudrate);
	}

	// Handle null device
	if (canNullNameDomain (deviceName))
		return canNullInit (deviceName, baudrate);

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