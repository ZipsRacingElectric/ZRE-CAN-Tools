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
#include <stdio.h>
#include <stddef.h>
#include <errno.h>
#include <stdlib.h>
#include <string.h> 
#include <stdio.h>
#include <dirent.h>

#if ! (__unix__)
#include <windows.h>
#endif

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
	slcan_t* can = device;

	can_message_t message;
	int code = can_read (can->handle, &message, can->timeoutMs);
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

/*
TODO(DiBacco): change device enumeration option syntax
	- slcan: /dev/tty*@1000000
	- /dev/tty*@1000000
	- *@1000000 
	- ?
*/

int slcanenumerateDevice (char** deviceNames, size_t* deviceCount, char* baudRate) 
{	
	char deviceName[128];

	// Check the OS running the program based on system-defined macro 
	# if ! (__unix__)
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
				sprintf (deviceName, "%s@%s", device, baudRate);
				deviceNames [*deviceCount] = malloc(strlen(deviceName) + 1);
				strcpy (deviceNames [*deviceCount], deviceName);
				++(*deviceCount);
			}

			device += strlen (device) + 1;
		}

	# else
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
				sprintf (deviceName, "/dev/%s@%s", device, baudRate);
				deviceNames [*deviceCount] = malloc(strlen(deviceName) + 1);
				strcpy (deviceNames [*deviceCount], deviceName);
				++(*deviceCount);
			}

		}

		closedir (directory);

	# endif

	if (!(*deviceCount))
		return -1;

	return 0;
}
