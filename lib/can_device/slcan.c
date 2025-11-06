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
#include <errno.h>
#include <stdlib.h>
#include <string.h>

// TODO(DiBacco): headers required?
#include <windows.h> // provides WIN32 types
#include <setupapi.h> // provides SetupAPI declarations
#include <initguid.h> // provides predefined GUID constants 
#include <devguid.h> // provides predefined GUID constants for device setup classes
#include <regstr.h> // provides predefined constants for registry keys and property names
#include <tchar.h> // provides necessary macros and types for Unicode / ANSI compatibility 
#include <stdio.h> // provides standard I/O functions

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

char** slcanEnumerateDevices(size_t* deviceCount) 
{
	// Check the OS running the program based on system-defined macro 
	#if defined (_WIN32) || defined (_WIN64)
		// Lists the communication (COM) devices connected to the serial communication port
		// SetupDiGetClassDevs: returns handle to a device information set that contians requested device information elements for a local computer
		// Note: will automatically map to the function version with the A suffix (ANSI [narrow char]) or the W suffix (Unicode [wide char])
    	// 		- GUID class
		//			- GUID_DEVINTERFACE_COMPORT: identifies the device interface class for COM ports
		// 		- Device Enumerator Name
		// 			- 0 / Null disables enumerator filtering 
		// 		- Window handle (hwndParent)
		// 			- 0 / Null disables dialogs
		// 		- DIGCF flags, which used to filter devices during enumeration
		// 			- DIGCF_PRESENT: include devices that are physically attached
		// 			- DIGCF_DEVICEINTERFACE: include devices that expose an interface
		// 		- Returns handle to device information set (HDEVINFO) 
		HDEVINFO hDevInfo = SetupDiGetClassDevs (&GUID_DEVINTERFACE_COMPORT, 0, 0, DIGCF_PRESENT | DIGCF_DEVICEINTERFACE);

		// SP_DEVINFO_DATA: used to identify a specific device within a device information set
		SP_DEVINFO_DATA devInfoData;
		devInfoData.cbSize = sizeof(SP_DEVINFO_DATA);

		// Store device names
		// TODO(DiBacco): would like to not have to dynamically allocate this list
		char** deviceNames = malloc(devInfoData.cbSize * sizeof(char*));
		*deviceCount = devInfoData.cbSize;

		// Enumerate each device within the device information set
		// SetupDiEnumDeviceInfo: retrieves information about the device @ position i in the device information set
		// DWORD: 32-bit data type
		for (DWORD i = 0; SetupDiEnumDeviceInfo(hDevInfo, i, &devInfoData); i++) {
    		// TCHAR: generic-text character type
			TCHAR friendlyName[256];
			// SetupDiGetDeviceRegistryProperty: used to get properties of a device from its registry key
			// 		- DeviceInfoSet (HDEVINFO): handle to the device information set
			// 		- DeviceInfoData (PSP_DEVINFO_DATA): pointer to the structure identifying the specific device within the set
			//			- PSP_DEVINFO_DATA: a pointer to SP_DEVINFO_DATA
			// 			- SP_DEVINFO_DATA: represents a single device in a device information set
			// 		- Property (DWORD): the device property to retrieve 
			// 		- PropertyRegDataType (PWORD): [optional] retrieves the data type of the property 
			// 		- PropertyBuffer (PBYTE): buffer where the property value will be stored
			// 			- PBYTE: pointer to a byte
			// 		- PropertyBufferSize (DWORD): size of property buffer (in bytes)
			// 		- RequiredSize (PDWORD): [optional] retrieves the size of the data 
			//			- PDWORD: pointer to DWORD
			// 		- Returns true is PropertyBuffer contains information associated with the device and false otherwise
			// PBYTE: a pointer to a byte 
			if (SetupDiGetDeviceRegistryProperty (
				hDevInfo, 
				&devInfoData,
				SPDRP_FRIENDLYNAME, 
				NULL, 
				(PBYTE)friendlyName, 
				sizeof(friendlyName), 
				NULL)) 
				{	
					printf ("Friendly Name: %s\n", friendlyName);

					// Formats the friendly name, such that only the device name is displayed (ex. COM1)
					strtok (friendlyName, "(");
					char* deviceName = strtok (NULL, ")");
					deviceNames[i] = deviceName;
				}


		}

		// Frees the resources associated with the device information set
		SetupDiDestroyDeviceInfoList(hDevInfo);	
		return deviceNames;
		
	#elif defined (__linux__)
   		printf("Running on Linux\n");

	#endif
}

char* slcanGetDevice (char** deviceNames, size_t deviceCount, char* baudRate) 
{
	static char deviceName [20];
	sprintf (deviceName, "%s@%s", deviceNames[0], baudRate);

	return deviceName;
}