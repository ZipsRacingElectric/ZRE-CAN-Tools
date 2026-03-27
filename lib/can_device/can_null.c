// Header
#include "can_null.h"

// Includes
#include "error_codes.h"

// C Standard Library
#include <errno.h>
#include <stdlib.h>
#include <unistd.h>

// Datatypes ------------------------------------------------------------------------------------------------------------------

typedef struct
{
	canDeviceVmt_t vmt;
	canBaudrate_t baudrate;
	unsigned long timeout;
} canNull_t;

// Functions ------------------------------------------------------------------------------------------------------------------

bool canNullNameDomain (const char* name)
{
	return strcmp (name, "null") == 0;
}

canDevice_t* canNullInit (char* name, canBaudrate_t baudrate)
{
	// Name is always 'null', so ignore.
	(void) name;

	// Device must be dynamically allocated
	canNull_t* device = malloc (sizeof (canNull_t));
	if (device == NULL)
		return NULL;

	// Setup the device's VMT
	device->vmt.transmit		= canNullTransmit;
	device->vmt.receive			= canNullReceive;
	device->vmt.flushRx			= canNullFlushRx;
	device->vmt.setTimeout		= canNullSetTimeout;
	device->vmt.getBaudrate		= canNullGetBaudrate;
	device->vmt.getDeviceName	= canNullGetDeviceName;
	device->vmt.getDeviceType	= canNullGetDeviceType;
	device->vmt.dealloc			= canNullDealloc;

	// Internal housekeeping
	device->baudrate	= baudrate;
	device->timeout		= 0;

	return (canDevice_t*) device;
}

void canNullDealloc (void* device)
{
	// Free the device's memory
	free (device);
}

int canNullTransmit (void* device, canFrame_t* frame)
{
	(void) device;
	(void) frame;

	// Always succeeds
	return 0;
}

int canNullReceive (void* device, canFrame_t* frame)
{
	(void) frame;
	canNull_t* can = device;

	// Block for the timeout interval
	if (can->timeout == 0)
	{
		while (true)
			usleep (1000000);
	}
	else
	{
		usleep (can->timeout * 1000);
	}

	// Return timeout
	errno = ERRNO_CAN_DEVICE_TIMEOUT;
	return errno;
}

int canNullFlushRx (void* device)
{
	(void) device;

	// Always succeeds
	return 0;
}

int canNullSetTimeout (void* device, unsigned long timeoutMs)
{
	canNull_t* can = device;

	// Save the timeout
	can->timeout = timeoutMs;
	return 0;
}

canBaudrate_t canNullGetBaudrate (void *device)
{
	return ((canNull_t*) device)->baudrate;
}

const char* canNullGetDeviceName (void* device)
{
	(void) device;
	return "null";
}

const char* canNullGetDeviceType (void)
{
	return "null";
}