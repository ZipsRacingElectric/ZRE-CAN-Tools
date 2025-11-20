// Header
#include "slcan.h"

// Includes
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

#ifdef __unix__
#include <dirent.h>
#else
#include <windows.h>
#endif

// Datatypes ------------------------------------------------------------------------------------------------------------------

typedef struct
{
	canDeviceVmt_t vmt;
	int handle;
	const char* name;
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

bool slcanWildcard (const char* name) {
	// determine whether the * is directly after COM or /dev/tty/
	if ((strstr (name,  "/dev/tty/*")) || strstr (name,  "COM*")) {
		return true;
	}
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
	device->name = name;
	device->baudrate = baudrate;

	// Default to blocking.
	device->vmt.setTimeout (device, 0);

	return (canDevice_t*) device;
}

canDevice_t** slcanEnumerate (canBaudrate_t* baudrate, size_t* deviceCount)
{	
	// DiBacco: 
	// The canDevice vmt requires the canDevice instances to persist as pointers because the vmt will not recognize the requested members -- that are 
	// required for the handle (such as the deviceName & baudrate) -- if the instance is a copy of another instance or has been dereferenced.
	// This implies that the list implementation cannot be used, given that it does not support pointers, & the function must return a double pointer 
	// to act as an array of canDevice pointers. 

	// TODO(DiBacco): consider what would be an apprpriate size for this array
	static canDevice_t* devices [512];

	# ifdef ZRE_CANTOOLS_OS_linux
		// Communication device enumeration on a Linux OS will involve enumerating the /dev directory
		// dirent (directory entry): represents an entry inside a directory
		/* struct dirent {
		   		ino_t d_ino;            // inode number
				char d_name[256];       // filename
				unsigned char d_type;   // file type
			};
		*/
		// DIR: data type representing a directory stream
		// included in the dirent header file
		const char* dev = "/dev";
		struct dirent* entry;
		DIR* directory = opendir (dev);
		while (entry = readdir (directory))
		{
			if (entry == NULL)
			{
				break;
			}
			char* device = entry->d_name;
			if (strstr (device, "ttyACM"))
			{
				devices[(*deviceCount)] = slcanInit (device, *baudrate);
				++(*deviceCount);
			}
		}
		closedir (directory);

	# endif // ZRE_CANTOOLS_OS_linux

	# ifdef ZRE_CANTOOLS_OS_windows
		// QueryDosDeviceA: retrieves information about MS-DOS (Microsoft Disk Operating System) device names
		// DWORD (Double-Word): 32-bit unsigned data type
		// LPSTR (Long Pointer to a String): typedef (alias) for a char*
		LPSTR device;
		char buffer [32768];
		DWORD bufferSize = 32768;
		QueryDosDeviceA (NULL, buffer, bufferSize);
		device = buffer;
		while (*device)
		{
			if (strstr (device, "COM") && strlen (device) <= 5)
			{
				// DiBacco: why does baud need to be used in this function as a pointer?
				devices[(*deviceCount)] = slcanInit (device, *baudrate);
				++(*deviceCount);
			}
			device += strlen (device) + 1;
		}

	# endif // ZRE_CANTOOLS_OS_windows

	if ((*deviceCount) > 0)
	{
		return devices; 
	} 
	return NULL;
}

size_t slcanSelectDevice (canDevice_t** devices, size_t deviceCount)
{
	char command [512];

	while (true)
	{
		printf ("\n");
		printf ("Which SLCAN Device would you like to use?\n");
		for (size_t deviceIndex = 0; deviceIndex < deviceCount; ++deviceIndex)
		{
			printf (" %zu). %s\n", deviceIndex + 1, canGetDeviceName(devices [deviceIndex]));
		}

		fgets (command, sizeof (command), stdin);
		command [strcspn (command, "\r\n")] = '\0';

		size_t x = (size_t) strtol (command, NULL, 0);
		if (0 < x && x <= deviceCount)
		{
			printf ("\n");
			return x -1;
		}	
	}
}

void slcanDealloc (void* device)
{
	slcan_t* slcan = device;

	// Stop and terminate the SLCAN device
	can_reset (slcan->handle);
	can_exit (slcan->handle);

	// Free the device's memory
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

/*
	TODO(DiBacco): change device enumeration option syntax
	- slcan: /dev/tty*@1000000
	- /dev/tty*@1000000
	- *@1000000
	- ?
	NOTE(Barach): For now, I'd say /dev/tty*@1000000 and COM*@1000000 are the preferred options. We can consider switching
	  things up if need be, but for now this is distinct enough.
*/

// REVIEW(Barach): Temporarily removed this just in case it causes any compilation issues on ARM. Probably okay, but didn't
//   really feel like testing on the RPi.

// int slcanEnumerateDevice (char** deviceNames, size_t* deviceCount, char* baudRate)
// {
// 	

// 	// Check the OS running the program based on system-defined macro

// 	#endif // __unix__

// 	if (!(*deviceCount))
// 		return -1;

// 	return 0;
// }
