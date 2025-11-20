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

// C Standard Library
#include <inttypes.h>
#include <math.h>
#include <signal.h>

// Globals --------------------------------------------------------------------------------------------------------------------

bool logging = true;

// Functions ------------------------------------------------------------------------------------------------------------------

void testSystemTick ()
{
	struct timespec res;
	clock_getres (CLOCK_MONOTONIC, &res);
	printf ("%lins\n", res.tv_nsec);
}

void sigtermHandler (int sig)
{
	(void) sig;

	printf ("Terminating...\n");
	logging = false;
}

void fprintUsage (FILE* stream)
{
	fprintf (stream, "Usage: can-mdf-logger <Options> <Device Name> <MDF file>.\n");
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

	fprintf (stream, "Options:\n\n");
	fprintOptionHelp (stream, "    ");
}

// Entrypoint -----------------------------------------------------------------------------------------------------------------

int main (int argc, char** argv)
{
	// Debug initialization
	debugInit ();

	// Check standard arguments
	for (int index = 1; index < argc; ++index)
	{
		const char* option;
		switch (handleOption (argv [index], &option, fprintHelp))
		{
		case OPTION_CHAR:
			switch (option [0])
			{
			case 'r':
				testSystemTick ();
				return 0;
			default:
				fprintf (stderr, "Unknown argument '%s'.\n", argv [index]);
				return -1;
			}
			break;

		case OPTION_STRING:
			fprintf (stderr, "Unknown argument '%s'.\n", argv [index]);
			return -1;

		case OPTION_QUIT:
			return 0;

		default:
			break;
		}
	}

	// Validate usage
	if (argc < 3)
	{
		fprintUsage (stderr);
		return -1;
	}

	// Initialize the CAN device
	char* deviceName = argv [argc - 2];
	canDevice_t* device = canInit (deviceName);
	if (device == NULL)
		return errorPrintf ("Failed to initialize CAN device '%s'", deviceName);

	// Calculate the bit time from the bus baudrate.
	canBaudrate_t baudrate = canGetBaudrate (device);
	if (baudrate == CAN_BAUDRATE_UNKNOWN)
	{
		fprintf (stderr, "CAN device baudrate is required.\n");
		return -1;
	}
	float bitTime = canCalculateBitTime (baudrate);

	// TODO(Barach): Internalize?
	struct timespec logTime;
	clock_gettime (CLOCK_MONOTONIC, &logTime);

	// TODO(Barach): A lot of placeholders here.
	mdfCanBusLogConfig_t config =
	{
		// TODO(Barach): How much can I change this?
		.filePath			= argv [argc - 1],
		.programId			= "ZRECAN",
		.softwareName		= "zre_cantools", // TODO(Barach): Want this to match the release name.
		.softwareVersion	= ZRE_CANTOOLS_VERSION_FULL,
		.hardwareVersion	= canGetDeviceType (device),
		.serialNumber		= "0",
		.channel1Baudrate	= canGetBaudrate (device),
		.channel2Baudrate	= 0,
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

	// Set a receive timeout so we can check for the termination signal.
	canSetTimeout (device, 100);

	// Status measurements
	struct timespec timeStart;
	clock_gettime (CLOCK_MONOTONIC, &timeStart);
	struct timespec timeEnd = timespecAdd (&timeStart, &(struct timespec) { .tv_sec = 1 });

	// Bus load measurements
	size_t frameCount = 0;
	size_t errorCount = 0;
	size_t minBitCount = 0;
	size_t maxBitCount = 0;

	while (logging)
	{
		// Receive a CAN frame
		canFrame_t frame;
		int code = canReceive (device, &frame);

		// Get a timestamp for the frame
		struct timespec timeCurrent;
		clock_gettime (CLOCK_MONOTONIC, &timeCurrent);

		// Check for success
		if (code == 0)
		{
			if (!frame.rtr)
			{
				// Log data frame
				if (mdfCanBusLogWriteDataFrame (&log, &frame, 1, false, &timeCurrent) != 0)
					errorPrintf ("Warning, failed to log CAN data frame");
			}
			else
			{
				// Log RTR frame
				if (mdfCanBusLogWriteRemoteFrame (&log, &frame, 1, false, &timeCurrent) != 0)
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
			if (mdfCanBusLogWriteErrorFrame (&log, &frame, 1, false, code, &timeCurrent) != 0)
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
			printf ("Bus Load: [%6.2f%%, %6.2f%%],   CAN Frames Received: %5lu,   Error Frames Received: %5lu,   "
				"Bits Received: [%7lu, %7lu]\n", minLoad * 100.0f, maxLoad * 100.0f, (unsigned long) frameCount,
				(unsigned long) errorCount, (unsigned long) minBitCount, (unsigned long) maxBitCount);

			// TODO(Barach): Keep hard-coded, or use database lib?
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
			if (canTransmit (device, &statusFrame) != 0)
				errorPrintf ("Warning, failed to transmit status message");

			// Log the status frame.
			if (mdfCanBusLogWriteDataFrame (&log, &statusFrame, 1, true, &timeCurrent) != 0)
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

	// Terminate the log gracefully
	printf ("Closing MDF file...\n");
	mdfCanBusLogClose (&log);
	canDealloc (device);

	return 0;
}