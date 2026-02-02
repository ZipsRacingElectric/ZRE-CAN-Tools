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

#ifdef __unix__
#include <dirent.h>
#else
#include <windows.h>
#endif

// Define a list of canDevice_t pointers
typedef canDevice_t* canDevicePtr_t;
listDefine (canDevicePtr_t); // Defines the list methods for the specific datatype 
list_t (canDevicePtr_t) devices; // Creates an instance of a list of the specific datatype

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

bool slcanWildcard (const char* name) {
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
	// REVIEW(Barach): Should initialize deviceCount
	*deviceCount = 0;

	// REVIEW(Barach): Not an issue, but much faster to init capacity to something like 8.
	// Dynamic list stores enumerated can devices 
	listInit (canDevicePtr_t) (&devices, 1);

	# ifdef ZRE_CANTOOLS_OS_linux
		const char* dev = "/dev";
		struct dirent* entry;
		DIR* directory = opendir (dev);
		
		// Review(Barach): Should probably print a list of the potential devices to verbose output.

		// Traverses the /dev directory
		while ((entry = readdir (directory)))
		{
			if (entry == NULL)
			{
				break;
			}
			char* device = entry->d_name;

			// Checks if device is of type ttyACM
			if (strstr (device, "ttyACM"))
			{
				// Prepend the device path to the device name
				char* devicePath = "/dev/";
				// REVIEW(Barach): Unchecked malloc
				char* x = malloc (strlen(devicePath) + strlen(device) + 1);
				strcpy (x, devicePath);
				strcat (x, device);

				// Attempts to create slcan device
				canDevice_t* slcanDevice = slcanInit (x, baudrate);
				if (slcanDevice == NULL)
				{
					debugPrintf ("Failed to initialize CAN device '%s': %s", x, errorCodeToMessage (errno));
					errno = 0;
					continue;
				}
				
				// Appends slcan device to the list of enumerated slcan devices 
				listAppend (canDevicePtr_t) (&devices, slcanDevice);
				++(*deviceCount);
				free (x);
			}
		}

		// Memory internal to the opendir() function is deallocated using the closedir() function
		closedir (directory);

	# endif // ZRE_CANTOOLS_OS_linux

	# ifdef ZRE_CANTOOLS_OS_windows
		for (int i = 1; i <= 255; i++) {
			
			// Sets the size of the device name
			int n = strlen ("COM");
			if (i > 9) n += 2;
			else if (i > 99) n += 3;
			else n += 1;

			// Allocates memory for the size of the device name
			char* device = malloc (n + 1);

			// Creates the device name on successful memory allocation.
			if (device) {
    			sprintf (device, "COM%d", i);
			}

			// Creates a handle that can be used to access the device connected to the COM port.
    	    HANDLE handle = CreateFileA (
    	        device,
    	        GENERIC_READ | GENERIC_WRITE,
    	        0,  
    	        NULL,
    	        OPEN_EXISTING,
    	        0,
    	        NULL
    	    );

			// Checks that there is a device occupying the COM port.
    	    if (handle != INVALID_HANDLE_VALUE) {

				// Invalidates the object handle.
				CloseHandle (handle);

				// Attempts to create SLCAN device
				canDevice_t* slcanDevice = slcanInit (device, baudrate);
				if (slcanDevice == NULL)
				{
					errorPrintf ("Failed to Create Can Device");
					continue;
				}
				
				// Appends slcan device to the list of enumerated slcan devices 
				listAppend (canDevicePtr_t) (&devices, slcanDevice);
				++(*deviceCount);        	
    	    }

		free (device);

    	}
		

	# endif // ZRE_CANTOOLS_OS_windows
	
	// REVIEW(Barach): Should update to listToArray
	// Returns array of enumerated slcan devices
	if (listSize (canDevicePtr_t) (&devices) > 0)
		return listArray (canDevicePtr_t) (&devices);

	// REVIEW(Barach): Should set errno on failure
	errno = ERRNO_CAN_DEVICE_MISSING_DEVICE;
	return NULL;
}

// REVIEW(Barach): I'd move this into canDevice, as there's nothing inherently specific to SLCAN here (if we change the prompt).
// TODO(DiBacco): consider automatically selecting can device if only one is detected.
size_t slcanSelectDevice (canDevice_t** devices, size_t deviceCount)
{
	char command [512];

	while (true)
	{
		printf ("\n");
		printf ("Which SLCAN Device would you like to use?\n");
		for (size_t deviceIndex = 0; deviceIndex < deviceCount; ++deviceIndex)
		{
			// REVIEW(Barach): 0-based index (even on the user side) is preferred unless there is strong incentive not to.
			// REVIEW(Barach): zu doesn't work on Windows (or debian, can't rememeber which). %lu and (unsigned long) cast are preferred.
			printf (" %zu). %s\n", deviceIndex + 1, canGetDeviceName (devices [deviceIndex]));
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