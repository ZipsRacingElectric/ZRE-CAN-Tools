// Header
#include "can_device.h"

// Includes
#include "error_codes.h"
#include "socket_can.h"
#include "slcan.h"

// C Standard Library
#include <errno.h>
#include <stdlib.h>

canDevice_t* canInit (const char* name)
{
	// Handle SocketCAN device
	if (socketCanNameDomain (name))
		return socketCanInit (name);

	if (slcanNameDomain (name))
		return slcanInit(name);

	// Unknown device
	errno = ERRNO_CAN_DEVICE_UNKNOWN_NAME;
	return NULL;
}