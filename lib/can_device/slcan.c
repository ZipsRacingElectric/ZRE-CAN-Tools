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

static int getErrorCode (int code)
{
	// Offset the error code to match this project's convention.
	return code + 10000;
}

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
		errno = getErrorCode (handle);
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
		errno = getErrorCode (code);
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
	can->vmt.setTimeout (can, 0);

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

	// Convert to an SLCAN frame
	can_message_t slcanFrame =
	{
		.id = frame->id,
		.dlc = frame->dlc,
		.xtd = frame->ide,
		.rtr = frame->rtr
	};
	memcpy (slcanFrame.data, frame->data, frame->dlc);

	int code = can_write (can->handle, &slcanFrame, can->timeoutMs);
	if (code != 0)
	{
		errno = getErrorCode (code);
		return errno;
	}

	return 0;
}

int slcanReceive (void* device, canFrame_t* frame)
{
	slcan_t* can = device;

	can_message_t slcanFrame;

	// Read the CAN frame.
	// - Note there is an intermittent bug on the Windows implementation where can_read can return CANERR_RX_EMPTY even when
	//   the device is set to non-blocking operation. To patch this, the function call is re-attempted in these specific
	//   conditions.
	int code;
	do
	{
		code = can_read (can->handle, &slcanFrame, can->timeoutMs);
	} while (can->timeoutMs == 65535 && code == CANERR_RX_EMPTY);

	// Check the error code.
	if (code != 0)
	{
		errno = getErrorCode (code);
		return errno;
	}

	// Convert back from the SLCAN frame
	frame->id = slcanFrame.id;
	frame->dlc = slcanFrame.dlc;
	frame->ide = slcanFrame.xtd;
	frame->rtr = slcanFrame.rtr;
	memcpy (frame->data, slcanFrame.data, slcanFrame.dlc);
	return 0;
}

int slcanFlushRx (void* device)
{
	slcan_t* can = device;

	// Read all available data from the device.
	can_message_t slcanFrame;
	while (can_read (can->handle, &slcanFrame, 0) == 0);

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