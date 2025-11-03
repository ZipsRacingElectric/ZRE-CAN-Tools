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