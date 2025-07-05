// Header
#include "slcan.h"

// SerialCAN
#define OPTION_CANAPI_DRIVER  1
#include "can_api.h"
#include "CANAPI_Defines.h"
#include "SerialCAN_Defines.h"

// C Standard Library
#include <errno.h>
#include <stdlib.h>
#include <string.h>

// TODO(Barach)
#include "error_codes.h"

// Datatypes ------------------------------------------------------------------------------------------------------------------

typedef struct
{
	canDeviceVmt_t vmt;
	int handle;
	const char* name;
	long int timeoutMs;
} slcan_t;

// Functions ------------------------------------------------------------------------------------------------------------------

bool slcanNameDomain (const char* name)
{
	if (strncmp("/dev/tty", name, strlen ("/dev/tty")) == 0)
		return true;

	if (strncmp("COM", name, strlen ("COM")) == 0)
		return true;

	return false;
}

canDevice_t* slcanInit (const char* name)
{
	can_bitrate_t bitrate;
	can_sio_param_t port;

	port.name = (char*) name;
	port.attr.protocol = CANSIO_CANABLE;
	port.attr.baudrate = CANSIO_BD57600;
	port.attr.bytesize = CANSIO_8DATABITS;
	port.attr.parity = CANSIO_NOPARITY;
	port.attr.stopbits = CANSIO_1STOPBIT;

	int handle = can_init (CAN_BOARD (CANLIB_SERIALCAN, CANDEV_SERIAL), CANMODE_DEFAULT, (const void*) &port);
	if (handle < 0)
	{
		errno = handle + 10000;
		return NULL;
	}

	bitrate.index = CANBTR_INDEX_1M;

	int code = can_start (handle, &bitrate);
	if (code < 0)
	{
		errno = code + 10000;
		return NULL;
	}

	// Setup the CAN device
	slcan_t* can = malloc (sizeof (slcan_t));
	can->handle = handle;
	can->name = name;
	can->vmt.transmit = slcanTransmit;
	can->vmt.receive = slcanReceive;
	can->vmt.setTimeout = slcanSetTimeout;
	can->vmt.flushRx = slcanFlushRx;

	// Default to blocking.
	canSetTimeout (can, 0);

	return (canDevice_t*) can;
}

int slcanDealloc (void* device)
{
	// TODO(Barach):
	(void) device;
	errno = ERRNO_OS_NOT_SUPPORTED;
	return errno;
}

int slcanTransmit (void* device, canFrame_t* frame)
{
	slcan_t* can = device;

	can_message_t message =
	{
		.id = frame->id,
		.dlc = frame->dlc
	};
	memcpy (message.data, frame->data, frame->dlc);

	int code = can_write (can->handle, &message, can->timeoutMs);
	if (code != 0)
	{
		errno = code + 10000;
		return errno;
	}

	return 0;
}

int slcanReceive (void* device, canFrame_t* frame)
{
	slcan_t* can = device;

	can_message_t message;
	int code = can_read (can->handle, &message, can->timeoutMs);
	if (code != 0)
	{
		errno = code + 10000;
		return errno;
	}

	frame->id = message.id;
	frame->dlc = message.dlc;
	memcpy (frame->data, message.data, message.dlc);
	return 0;
}

int slcanFlushRx (void* device)
{
	slcan_t* can = device;

	// Read all available data from the device.
	can_message_t message;
	while (can_read (can->handle, &message, 0) == 0);

	return 0;
}

int slcanSetTimeout (void* device, unsigned long timeoutMs)
{
	slcan_t* can = device;

	if (timeoutMs >= 65535)
	{
		errno = ERRNO_CAN_DEVICE_BAD_TIMEOUT;
		return errno;
	}

	// For blocking operation, use 65535
	if (timeoutMs == 0)
		timeoutMs = 65535;

	can->timeoutMs = timeoutMs;
	return 0;
}