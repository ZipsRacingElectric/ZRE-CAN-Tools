// For asprintf. Note this must be the first include in this file.
#define _GNU_SOURCE
#include <stdio.h>

// Header
#include "slcan.h"

// Includes
#include "list.h"
#include "debug.h"
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

#ifdef ZRE_CANTOOLS_OS_linux
#include <dirent.h>
#else
#include <windows.h>
#endif

// Defines the datatype for a list of canDevice_t pointers
typedef canDevice_t* canDevicePtr_t;
listDefine (canDevicePtr_t);

// Datatypes ------------------------------------------------------------------------------------------------------------------

typedef struct
{
	canDeviceVmt_t vmt;
	int handle;
	char* name;
	long int timeoutMs;
	canBaudrate_t baudrate;
} slcan_t;

// Functions ------------------------------------------------------------------------------------------------------------------

static int getErrorCode (int code)
{
	// The receiver empty error indicates a timeout occurred, so translate that into a timeout error.
	if (code == CANERR_RX_EMPTY)
		return ERRNO_CAN_DEVICE_TIMEOUT;

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

bool slcanWildcard (const char* name)
{
	// POSIX device format
	if (strncmp("/dev/tty*", name, strlen ("/dev/tty*")) == 0)
		return true;

	// Windows device format
	if (strncmp("COM*", name, strlen ("COM*")) == 0)
		return true;

	return false;
}

canDevice_t* slcanInit (char* name, canBaudrate_t baudrate)
{
	// Map the baudrate to one of the available options.
	can_bitrate_t slcanBaudrate;
	switch (baudrate)
	{
	case 1000000:
		slcanBaudrate.index = CANBTR_INDEX_1M;
		break;
	case 800000:
		slcanBaudrate.index = CANBTR_INDEX_800K;
		break;
	case 500000:
		slcanBaudrate.index = CANBTR_INDEX_500K;
		break;
	case 250000:
		slcanBaudrate.index = CANBTR_INDEX_250K;
		break;
	case 125000:
		slcanBaudrate.index = CANBTR_INDEX_125K;
		break;
	case 100000:
		slcanBaudrate.index = CANBTR_INDEX_100K;
		break;
	case 50000:
		slcanBaudrate.index = CANBTR_INDEX_50K;
		break;
	case 20000:
		slcanBaudrate.index = CANBTR_INDEX_20K;
		break;
	case 10000:
		slcanBaudrate.index = CANBTR_INDEX_10K;
		break;
	default:
		debugPrintf ("Non-standard SLCAN baudrate: %u bit/s\n", baudrate);
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

	int code = can_start (handle, &slcanBaudrate);
	if (code < 0)
	{
		errno = getErrorCode (code);
		return NULL;
	}

	// Device must be dynamically allocated
	slcan_t* device = malloc (sizeof (slcan_t));
	if (device == NULL)
		return NULL;

	// Setup the device's VMT
	device->vmt.transmit		= slcanTransmit;
	device->vmt.receive			= slcanReceive;
	device->vmt.flushRx			= slcanFlushRx;
	device->vmt.setTimeout		= slcanSetTimeout;
	device->vmt.getBaudrate		= slcanGetBaudrate;
	device->vmt.getDeviceName	= slcanGetDeviceName;
	device->vmt.getDeviceType	= slcanGetDeviceType;
	device->vmt.dealloc			= slcanDealloc;

	// Internal housekeeping
	device->handle = handle;
	device->name = strdup (name);
	if (device->name == NULL)
	{
		return NULL;
	}

	device->baudrate = baudrate;

	// Default to blocking.
	device->vmt.setTimeout (device, 0);

	return (canDevice_t*) device;
}

canDevice_t** slcanEnumerate (canBaudrate_t baudrate, size_t* deviceCount)
{
	// Dynamic list stores enumerated can devices
	list_t (canDevicePtr_t) devices;
	if (listInit (canDevicePtr_t) (&devices, 8) != 0)
		return NULL;

	debugPrintf ("Enumerating SLCAN devices...\n");

	#ifdef ZRE_CANTOOLS_OS_linux

		// Open the /dev/ directory
		const char* devPath = "/dev";
		DIR* devDir = opendir (devPath);

		// Traverse the /dev/ directory
		struct dirent* entry;
		while (true)
		{
			// Read the next entry, exit the loop at the end
			entry = readdir (devDir);
			if (entry == NULL)
				break;

			// Get the device name from the full path
			char* deviceName;
			if (asprintf (&deviceName, "%s/%s", devPath, entry->d_name) < 0)
			{
				listDealloc (canDevicePtr_t) (&devices);
				return NULL;
			}

			// Ignore all non-ACM devices
			if (strstr (deviceName, "ttyACM") == NULL)
			{
				free (deviceName);
				continue;
			}

			debugPrintf ("    %s - ", deviceName);

			// Attempts to create slcan device
			canDevice_t* slcanDevice = slcanInit (deviceName, baudrate);
			if (slcanDevice == NULL)
			{
				debugPrintf ("%s.\n", errorCodeToMessage (errno));
				errno = 0;
				free (deviceName);
				continue;
			}

			debugPrintf ("Success.\n");

			// Append the device to the list of created devices
			listAppend (canDevicePtr_t) (&devices, slcanDevice);

			// Deallocate the device name
			free (deviceName);
		}

		// Memory internal to the opendir() function is deallocated using the closedir() function
		closedir (devDir);

	#else // ZRE_CANTOOLS_OS_linux

		// Iterate over every potential COM device
		// - Note: This is not efficient, but trust me this is the most reliable option I could find. Please do not change
		//     without considering the consequences.
		for (int index = 1; index < 256; ++index)
		{
			// Allocate the device name
			char* deviceName;
			if (asprintf (&deviceName, "COM%i", index) < 0)
			{
				listDealloc (canDevicePtr_t) (&devices);
				return NULL;
			}

			// Creates a handle that can be used to access the device connected to the COM port.
    	    HANDLE handle = CreateFileA
			(
    	        deviceName,
    	        GENERIC_READ | GENERIC_WRITE,
    	        0,
    	        NULL,
    	        OPEN_EXISTING,
    	        0,
    	        NULL
    	    );

			// Checks that there is a device occupying the COM port.
    	    if (handle == INVALID_HANDLE_VALUE)
			{
				free (deviceName);
				continue;
			}

			// Release the device handler.
			CloseHandle (handle);

			debugPrintf ("    %s - ", deviceName);

			// To narrow things down (and especially to avoid Bluetooth devices, which cause the program to hang) we strip
			// out any devices that aren't USB devices.

			char buffer [65536];
			if (QueryDosDeviceA (deviceName, buffer, sizeof (buffer)) == 0)
			{
				debugPrintf ("Failed to query DOS device information.\n");
				free (deviceName);
				continue;
			}

			if (strstr (buffer, "USBSER") == NULL)
			{
				debugPrintf ("No occurance of 'CAN' in device info.\n");
				free (deviceName);
				continue;
			}

			// Attempt to create SLCAN device
			canDevice_t* slcanDevice = slcanInit (deviceName, baudrate);
			if (slcanDevice == NULL)
			{
				debugPrintf ("%s.\n", errorCodeToMessage (errno));
				errno = 0;
				free (deviceName);
				continue;
			}

			debugPrintf ("Success.\n");

			// Append the device to the list of created devices
			listAppend (canDevicePtr_t) (&devices, slcanDevice);

			// Deallocate the device name
			free (deviceName);
    	}

	#endif // ZRE_CANTOOLS_OS_windows

	// Returns array of enumerated slcan devices
	// TODO(Barach): Should update to listToArray
	if (listSize (canDevicePtr_t) (&devices) > 0)
	{
		*deviceCount = listSize (canDevicePtr_t) (&devices);
		return listArray (canDevicePtr_t) (&devices);
	}

	debugPrintf ("    No valid SLCAN devices found.\n");
	errno = ERRNO_CAN_DEVICE_MISSING_DEVICE;
	return NULL;
}

void slcanDealloc (void* device)
{
	slcan_t* slcan = device;

	// Stop and terminate the SLCAN device
	can_reset (slcan->handle);
	can_exit (slcan->handle);

	// Free the device's memory
	free(slcan->name);
	free (device);
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

canBaudrate_t slcanGetBaudrate (void *device)
{
	return ((slcan_t*) device)->baudrate;
}

const char* slcanGetDeviceName (void* device)
{
	return ((slcan_t*) device)->name;
}

const char* slcanGetDeviceType (void)
{
	return "SLCAN";
}