// Header
#include "slcan.h"

// Includes
#include "error_codes.h"

// SerialCAN
#define OPTION_CANAPI_DRIVER  1
#include "can_api.h"
#include "CANAPI_Defines.h"
#include "SerialCAN_Defines.h"

// C Standard Library
#include <errno.h>
#include <stdlib.h>
#include <string.h>

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
	// POSIX device format
	if (strncmp("/dev/tty", name, strlen ("/dev/tty")) == 0)
		return true;

	// Windows device format
	if (strncmp("COM", name, strlen ("COM")) == 0)
		return true;

	return false;
}

canDevice_t* slcanInit (char* name)
{
	// Split the name into the device name and bitrate: Format <device>@<baud>
	char* savePtr;
	strtok_r (name, "@", &savePtr);
	char* bitrateParam = strtok_r (NULL, "@", &savePtr);
	if (bitrateParam == NULL)
	{
		errno = ERRNO_SLCAN_BAUDRATE;
		return NULL;
	}

	can_sio_param_t port =
	{
		.name = (char*) name,
		.attr =
		{
			.protocol = CANSIO_CANABLE,
			.baudrate = CANSIO_BD57600,
			.bytesize = CANSIO_8DATABITS,
			.parity = CANSIO_NOPARITY,
			.stopbits = CANSIO_1STOPBIT
		}
	};

	int handle = can_init (CAN_BOARD (CANLIB_SERIALCAN, CANDEV_SERIAL), CANMODE_DEFAULT, (const void*) &port);
	if (handle < 0)
	{
		// Offset the error code to match this project's convention.
		errno = handle + 10000;
		return NULL;
	}

	// Identify the baudrate
	can_bitrate_t bitrate;
	if (strcmp (bitrateParam, "1000000") == 0)
		bitrate.index = CANBTR_INDEX_1M;
	else if (strcmp (bitrateParam, "800000") == 0)
		bitrate.index = CANBTR_INDEX_800K;
	else if (strcmp (bitrateParam, "500000") == 0)
		bitrate.index = CANBTR_INDEX_500K;
	else if (strcmp (bitrateParam, "250000") == 0)
		bitrate.index = CANBTR_INDEX_250K;
	else if (strcmp (bitrateParam, "125000") == 0)
		bitrate.index = CANBTR_INDEX_125K;
	else if (strcmp (bitrateParam, "100000") == 0)
		bitrate.index = CANBTR_INDEX_100K;
	else if (strcmp (bitrateParam, "50000") == 0)
		bitrate.index = CANBTR_INDEX_50K;
	else if (strcmp (bitrateParam, "20000") == 0)
		bitrate.index = CANBTR_INDEX_20K;
	else if (strcmp (bitrateParam, "10000") == 0)
		bitrate.index = CANBTR_INDEX_10K;
	else
	{
		errno = ERRNO_SLCAN_BAUDRATE;
		return NULL;
	}

	int code = can_start (handle, &bitrate);
	if (code < 0)
	{
		// Offset the error code to match this project's convention.
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
		// Offset the error code to match this project's convention.
		errno = code + 10000;
		return errno;
	}

	return 0;
}

int slcanReceive (void* device, canFrame_t* frame)
{
	int code;
	slcan_t* can = device;

	can_message_t message;

	/*
		can_read() = -30 (Recevier Empty) Overview

		Context:
			- Windows only
			- Intermittent -- replicable when receiving from the CAN bus with no valid ids as input
		Error: Can_Read is returning -30 (Recevier Empty)
			- timout value is set to 65535 which should create blocking functionality in the function (is not working in this context)
	 	Fix: repeat the can_read() function which essentially creates a blocking operation
	*/
	do
	{
		code = can_read (can->handle, &message, can->timeoutMs);
	} while (can->timeoutMs == 65535 && code == -30);

	if (code != 0)
	{
		// Offset the error code to match this project's convention.
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