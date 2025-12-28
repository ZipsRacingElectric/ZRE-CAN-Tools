// CAN MDF Logger -------------------------------------------------------------------------------------------------------------
//
// Author: Cole Barach
// Date Created: 2025.10.11
//
// Description: See help page.

// Includes -------------------------------------------------------------------------------------------------------------------

// Includes
#include "can_device/can_bus_load.h"
#include "can_device/can_device.h"
#include "can_device/can_device_stdio.h"
#include "cjson/cjson_util.h"
#include "debug.h"
#include "mdf/mdf_can_bus_logging.h"
#include "mdf/mdf_stdio.h"
#include "options.h"
#include "time_port.h"

// POSIX
#include <pthread.h>

// C Standard Library
#include <math.h>
#include <signal.h>
#include <stdlib.h>

// Globals --------------------------------------------------------------------------------------------------------------------

bool logging = true;

// Functions ------------------------------------------------------------------------------------------------------------------

void testSystemTick (char option, char* value)
{
	(void) option;
	(void) value;

	struct timespec resolution;
	if (mdfCanBusLogGetTimeResolution (&resolution) == 0)
		printf ("%lli ns\n", timespecToNs (&resolution));
	else
		errorPrintf ("Failed to get timer resolution");

	exit (0);
}

void sigtermHandler (int sig)
{
	(void) sig;

	printf ("Terminating...\n");
	logging = false;
}

void fprintUsage (FILE* stream)
{
	fprintf (stream, ""
		"Usage:\n"
		"    can-mdf-logger <Options> <MDF Directory> <Config JSON> <Channel 1 Device Name>.\n"
		"    can-mdf-logger <Options> <MDF Directory> <Config JSON> <Channel 1 Device Name> <Channel 2 Device Name>.\n");
}

void fprintHelp (FILE* stream)
{
	fprintf (stream, ""
		"can-mdf-logger - Application for logging the traffic of a CAN bus to an MDF\n"
		"                 file. This application also can transmit a status message\n"
		"                 containing the logging session and CAN bus's load / error\n"
		"                 count.\n\n");

	fprintUsage (stream);

	fprintf (stream, "\nParameters:\n\n");
	fprintCanDeviceNameHelp (stream, "    ");
	fprintMdfFileHelp (stream, "    ");

	fprintf (stream, ""
		"Options:\n\n"
		"    -r                    - Test the logging timer resolution. This is both OS\n"
		"                            and hardware dependent\n\n");
	fprintOptionHelp (stream, "    ");
}

// Threads --------------------------------------------------------------------------------------------------------------------

typedef struct
{
	canDevice_t* device;
	mdfCanBusLog_t* log;
	pthread_mutex_t* logMutex;
	uint8_t busChannel;
} loggingThreadArg_t;

void* loggingThread (void* argPtr)
{
	loggingThreadArg_t* arg = argPtr;

	// Set a receive timeout so we can check for the termination signal.
	canSetTimeout (arg->device, 100);

	// Status measurements
	struct timespec timeStart;
	if (mdfCanBusLogGetTimestamp (&timeStart) != 0)
		errorPrintf ("Warning, failed to get MDF timestamp");
	struct timespec timeEnd = timespecAdd (&timeStart, &(struct timespec) { .tv_sec = 1 });

	// Bus load measurements
	size_t frameCount = 0;
	size_t errorCount = 0;
	size_t minBitCount = 0;
	size_t maxBitCount = 0;

	// Calculate the bit time from the bus baudrate.
	float bitTime = canCalculateBitTime (canGetBaudrate (arg->device));

	while (logging)
	{
		// Receive a CAN frame. Due to its blocking nature, this must be outside the mutex guard.
		canFrame_t frame;
		int code = canReceive (arg->device, &frame);

		// Acquire access to the log file. Note this must be before the timestamp is generated.
		pthread_mutex_lock (arg->logMutex);

		// Get a timestamp for the frame
		struct timespec timeCurrent;
		if (mdfCanBusLogGetTimestamp (&timeCurrent) != 0)
			errorPrintf ("Warning, failed to get MDF timestamp");

		// Check for success
		if (code == 0)
		{
			if (!frame.rtr)
			{
				// Log data frame
				if (mdfCanBusLogWriteDataFrame (arg->log, &frame, arg->busChannel, false, &timeCurrent) != 0)
					errorPrintf ("Warning, failed to log CAN data frame");
			}
			else
			{
				// Log RTR frame
				if (mdfCanBusLogWriteRemoteFrame (arg->log, &frame, arg->busChannel, false, &timeCurrent) != 0)
					errorPrintf ("Warning, failed to log CAN remote frame");
			}

			// Measure the frame's size
			++frameCount;
			minBitCount += canGetMinBitCount (&frame);
			maxBitCount += canGetMaxBitCount (&frame);
		}
		else if (canCheckBusError (code))
		{
			// If an error frame was generated, log it
			if (mdfCanBusLogWriteErrorFrame (arg->log, &frame, arg->busChannel, false, code, &timeCurrent) != 0)
				errorPrintf ("Warning, failed to log CAN error frame");

			// Measure the error count
			++errorCount;
		}

		// Release access to the log file.
		pthread_mutex_unlock (arg->logMutex);

		// Print status message
		if (timespecCompare (&timeCurrent, &timeEnd, >))
		{
			// Calculate the actual measurement period
			struct timespec period = timespecSub (&timeCurrent, &timeStart);

			// Calculate the min and max loads
			float maxLoad = canCalculateBusLoad (maxBitCount, bitTime, period);
			float minLoad = canCalculateBusLoad (minBitCount, bitTime, period);

			// Print the status message
			printf ("Channel %u,   Bus Load: [%6.2f%%, %6.2f%%],   CAN Frames Received: %5lu,   Error Frames Received: %5lu,   "
				"Bits Received: [%7lu, %7lu]\n", arg->busChannel, minLoad * 100.0f, maxLoad * 100.0f,
				(unsigned long) frameCount, (unsigned long) errorCount, (unsigned long) minBitCount,
				(unsigned long) maxBitCount);

			canFrame_t statusFrame =
			{
				.id = 0x180,
				.ide = false,
				.rtr = false,
				.dlc = 4,
				.data =
				{
					floorf (minLoad * 100.0f / 0.6f),
					ceilf (maxLoad * 100.0f / 0.6f),
					errorCount,
					errorCount >> 8
				}
			};

			// Transmit the status message. Due to its blocking nature, this must be outside the mutex guard.
			if (canTransmit (arg->device, &statusFrame) != 0)
				errorPrintf ("Warning, failed to transmit status message");

			// Acquire access to the log file. Note this must be before the timestamp is generated.
			pthread_mutex_lock (arg->logMutex);

			// Get a timestamp for the frame. We canot reuse the previous value, as it will have already been used if a frame
			// was just received.
			struct timespec timeCurrent;
			if (mdfCanBusLogGetTimestamp (&timeCurrent) != 0)
				errorPrintf ("Warning, failed to get MDF timestamp");

			// Log the status frame.
			if (mdfCanBusLogWriteDataFrame (arg->log, &statusFrame, arg->busChannel, true, &timeCurrent) != 0)
				errorPrintf ("Warning, failed to log CAN data frame");

			// Release access to the log file.
			pthread_mutex_unlock (arg->logMutex);

			// Reset the measurements (include the status frame, as we just transmitted that)
			frameCount = 1;
			errorCount = 0;
			minBitCount = canGetMinBitCount (&statusFrame);
			maxBitCount = canGetMaxBitCount (&statusFrame);

			// Set the new deadline
			timeStart = timeCurrent;
			timeEnd = timespecAdd (&timeStart, &(struct timespec) { .tv_sec = 1 });
		}
	}

	return NULL;
}

int loadConfiguration (mdfCanBusLogConfig_t* config, const char* directory, const char* configPath, canDevice_t* channel1, canDevice_t* channel2)
{
	cJSON* configJson = jsonLoad (configPath);
	if (configJson == NULL)
		return errno;

	char* configName;
	if (jsonGetString (configJson, "configName", &configName) != 0)
		return errno;

	char* hardwareName;
	if (jsonGetString (configJson, "hardwareName", &hardwareName) != 0)
		return errno;

	char* hardwareVersion;
	if (jsonGetString (configJson, "hardwareVersion", &hardwareVersion) != 0)
		return errno;

	char* serialNumber;
	if (jsonGetString (configJson, "serialNumber", &serialNumber) != 0)
		return errno;

	*config = (mdfCanBusLogConfig_t)
	{
		.directory			= directory,
		.configurationName	= configName,
		.softwareName		= ZRE_CANTOOLS_NAME,
		.softwareVersion	= ZRE_CANTOOLS_VERSION_FULL,
		.softwareVendor		= "ZRE",
		.hardwareName		= hardwareName,
		.hardwareVersion	= hardwareVersion,
		.serialNumber		= serialNumber,
		.channel1Baudrate	= canGetBaudrate (channel1),
		.channel2Baudrate	= channel2 == NULL ? 0 : canGetBaudrate (channel2),
		.storageSize		= 0,
		.storageRemaining	= 0,
		.sessionNumber		= mdfCanBusLogGetSessionNumber (directory)
	};
	return 0;
}

// Entrypoint -----------------------------------------------------------------------------------------------------------------

int main (int argc, char** argv)
{
	// Debug initialization
	debugInit ();

	// Handle program options
	if (handleOptions (&argc, &argv, &(handleOptionsParams_t)
	{
		.fprintHelp		= fprintHelp,
		.chars			= (char []) { 'r' },
		.charHandlers	= (optionCharCallback_t* []) { testSystemTick },
		.charCount		= 1,
		.stringHandlers	= NULL,
		.strings		= NULL,
		.stringCount	= 0
	}) != 0)
		return errorPrintf ("Failed to handle options");

	// Validate usage
	if (argc < 3 || argc > 4)
	{
		fprintUsage (stderr);
		return -1;
	}

	char* mdfDirectory = argv [0];
	char* configPath = argv [1];

	// Initialize the channel 1 CAN device
	char* channel1DeviceName = argv [2];
	canDevice_t* channel1 = canInit (channel1DeviceName);
	if (channel1 == NULL)
		return errorPrintf ("Failed to initialize channel 1 CAN device '%s'", channel1DeviceName);

	// Require baudrate for bus load
	if (canGetBaudrate (channel1) == CAN_BAUDRATE_UNKNOWN)
	{
		fprintf (stderr, "Channel 1 CAN device missing baudrate.\n");
		return -1;
	}

	// If provided, initialize the channel 2 CAN device
	canDevice_t* channel2 = NULL;
	if (argc == 4)
	{
		char* channel2DeviceName = argv [3];
		channel2 = canInit (channel2DeviceName);
		if (channel2 == NULL)
			return errorPrintf ("Failed to initialize channel 2 CAN device '%s'", channel2DeviceName);

		// Require baudrate for bus load
		if (canGetBaudrate (channel2) == CAN_BAUDRATE_UNKNOWN)
		{
			fprintf (stderr, "Channel 2 CAN device missing baudrate.\n");
			return -1;
		}
	}

	mdfCanBusLogConfig_t config;
	if (loadConfiguration (&config, mdfDirectory, configPath, channel1, channel2) != 0)
		return errorPrintf ("Failed to load CAN bus MDF log configuration");

	mdfCanBusLog_t log;
	if (mdfCanBusLogInit (&log, &config) != 0)
		return errorPrintf ("Failed to initialize CAN bus MDF log");

	// Create a mutex guarding access to the log file. While single fwrite operations are thread-safe on their own, this mutex
	// is used to guarantee the timestamp written to each record of the log is monotonic, as our data analysis software imposes
	// said requirement. As a result of this, all timestamps written must be acquired inside the guard of this mutex.
	pthread_mutex_t logMutex;
	pthread_mutex_init (&logMutex, NULL);

	printf ("Starting MDF log: File name '%s'.\n", mdfCanBusLogGetName (&log));

	if (signal (SIGTERM, sigtermHandler) == SIG_ERR)
		return errorPrintf ("Failed to bind SIGTERM handler");

	if (signal (SIGINT, sigtermHandler) == SIG_ERR)
		return errorPrintf ("Failed to bind SIGINT handler");

	// Create the logging thread for channel 1.
	loggingThreadArg_t channel1Arg =
	{
		.device		= channel1,
		.log		= &log,
		.logMutex	= &logMutex,
		.busChannel	= 1
	};
	pthread_t channel1Thread;
	pthread_create (&channel1Thread, NULL, loggingThread, &channel1Arg);

	// Create the logging thread for channel 2.
	loggingThreadArg_t channel2Arg =
	{
		.device		= channel2,
		.log		= &log,
		.logMutex	= &logMutex,
		.busChannel	= 2
	};
	pthread_t channel2Thread;
	if (channel2 != NULL)
		pthread_create (&channel2Thread, NULL, loggingThread, &channel2Arg);

	// Wait for both threads to terminate.
	if (channel2 != NULL)
		pthread_join (channel2Thread, NULL);
	pthread_join (channel1Thread, NULL);

	// Destroy the mutex.
	pthread_mutex_destroy (&logMutex);

	// Terminate the log gracefully
	printf ("Closing MDF file...\n");
	mdfCanBusLogClose (&log);
	if (channel2 != NULL)
		canDealloc (channel2);
	canDealloc (channel1);

	return 0;
}