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
#include "debug.h"
#include "mdf/mdf_can_bus_logging.h"
#include "mdf/mdf_stdio.h"
#include "options.h"
#include "time_port.h"

// POSIX
#include <pthread.h>

// C Standard Library
#include <inttypes.h>
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

	struct timespec res;
	clock_getres (CLOCK_MONOTONIC, &res);
	printf ("%li ns\n", res.tv_nsec);
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
		"    can-mdf-logger <Options> <MDF file> <Channel 1 Device Name>.\n"
		"    can-mdf-logger <Options> <MDF file> <Channel 1 Device Name> <Channel 2 Device Name>.\n");
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
		"                            and hardware dependent\n");
	fprintOptionHelp (stream, "    ");
}

// Threads --------------------------------------------------------------------------------------------------------------------

typedef struct
{
	canDevice_t* device;
	mdfCanBusLog_t* log;
	uint8_t busChannel;
} loggingThreadArg_t;

void* loggingThread (void* argPtr)
{
	loggingThreadArg_t* arg = argPtr;

	// Set a receive timeout so we can check for the termination signal.
	canSetTimeout (arg->device, 100);

	// Status measurements
	struct timespec timeStart;
	clock_gettime (CLOCK_MONOTONIC, &timeStart);
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
		// Receive a CAN frame
		canFrame_t frame;
		int code = canReceive (arg->device, &frame);

		// Get a timestamp for the frame
		struct timespec timeCurrent;
		clock_gettime (CLOCK_MONOTONIC, &timeCurrent);

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

			// Transmit the status message.
			if (canTransmit (arg->device, &statusFrame) != 0)
				errorPrintf ("Warning, failed to transmit status message");

			// Log the status frame.
			if (mdfCanBusLogWriteDataFrame (arg->log, &statusFrame, arg->busChannel, true, &timeCurrent) != 0)
				errorPrintf ("Warning, failed to log CAN data frame");

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
	if (argc < 2 || argc > 3)
	{
		fprintUsage (stderr);
		return -1;
	}

	// First arg is MDF file path
	char* mdfFilePath = argv [0];

	// Initialize the channel 1 CAN device
	char* channel1DeviceName = argv [1];
	canDevice_t* channel1 = canInit (channel1DeviceName);
	if (channel1 == NULL)
		return errorPrintf ("Failed to initialize channel 1 CAN device '%s'", channel1DeviceName);

	// Require baudrate
	if (canGetBaudrate (channel1) == CAN_BAUDRATE_UNKNOWN)
	{
		fprintf (stderr, "Channel 1 CAN device missing baudrate.\n");
		return -1;
	}

	// If provided, initialize the channel 2 CAN device
	canDevice_t* channel2 = NULL;
	if (argc == 3)
	{
		char* channel2DeviceName = argv [2];
		channel2 = canInit (channel2DeviceName);
		if (channel2 == NULL)
			return errorPrintf ("Failed to initialize channel 2 CAN device '%s'", channel2DeviceName);

		if (canGetBaudrate (channel2) == CAN_BAUDRATE_UNKNOWN)
		{
			fprintf (stderr, "Channel 2 CAN device missing baudrate.\n");
			return -1;
		}
	}

	// TODO(Barach): Internalize?
	struct timespec logTime;
	clock_gettime (CLOCK_MONOTONIC, &logTime);

	// TODO(Barach): A lot of placeholders here.
	mdfCanBusLogConfig_t config =
	{
		// TODO(Barach): How much can I change this?
		.filePath			= mdfFilePath,
		.programId			= "ZRECAN",
		.softwareName		= "zre_cantools", // TODO(Barach): Want this to match the release name.
		.softwareVersion	= ZRE_CANTOOLS_VERSION_FULL,
		.hardwareVersion	= canGetDeviceType (channel1),
		.serialNumber		= "0",
		.channel1Baudrate	= canGetBaudrate (channel1),
		.channel2Baudrate	= channel2 == NULL ? 0 : canGetBaudrate (channel2),
		.dateStart			= time (NULL),
		.timeStart			= logTime,
		.storageSize		= 0,
		.storageRemaining	= 0,
		.sessionNumber		= 0,
		.splitNumber		= 1
	};

	mdfCanBusLog_t log;
	if (mdfCanBusLogInit (&log, &config) != 0)
		return errorPrintf ("Failed to initialize CAN bus MDF log");

	printf ("Starting data log: File name '%s', session number %"PRIu32", split %"PRIu32".\n",
		config.filePath, config.sessionNumber, config.splitNumber);

	if (signal (SIGTERM, sigtermHandler) == SIG_ERR)
		return errorPrintf ("Failed to bind SIGTERM handler");

	if (signal (SIGINT, sigtermHandler) == SIG_ERR)
		return errorPrintf ("Failed to bind SIGINT handler");

	loggingThreadArg_t channel1Arg =
	{
		.device		= channel1,
		.log		= &log,
		.busChannel	= 1
	};
	pthread_t channel1Thread;
	pthread_create (&channel1Thread, NULL, loggingThread, &channel1Arg);
	pthread_join (channel1Thread, NULL);

	// Terminate the log gracefully
	printf ("Closing MDF file...\n");
	mdfCanBusLogClose (&log);
	if (channel2 != NULL)
		canDealloc (channel2);
	canDealloc (channel1);

	return 0;
}