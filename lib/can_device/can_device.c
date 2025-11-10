// Header
#include "can_device.h"

// Includes
#include "error_codes.h"
#include "socket_can.h"
#include "slcan.h"

// C Standard Library
#include <errno.h>
#include <stdlib.h> 

canDevice_t* canInit (char* name)
{
	// Handle SocketCAN device
	if (socketCanNameDomain (name))
		return socketCanInit (name);

	// Handle SLCAN device
	if (slcanNameDomain (name))
		return slcanInit(name);

	// Unknown device
	errno = ERRNO_CAN_DEVICE_UNKNOWN_NAME;
	return NULL;
}

canDevice_t* findCanDevice (char** names, size_t count)
{
	char name [128];
	canDevice_t* device;
	
	for (int i = 0; i < count; ++i) 
	{
		device = canInit (names[i]);

		if (device == NULL)
		{
			continue;
		}

		return device;
	}

	// Errno is set in the canInit() function
	// Indicates that a CAN device could not be created
	return NULL;
}
