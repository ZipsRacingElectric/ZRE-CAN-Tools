// Includes
#include "can_device/can_bus_load.h"
#include "can_device/can_device.h"
#include "debug.h"
#include "mdf/mdf_can_bus_logging.h"
#include "time_port.h"

// C Standard Library
#include <inttypes.h>
#include <math.h>
#include <signal.h>

// TODO(Barach): Major testing...

bool logging = true;

void sigtermHandler (int sig)
{
	(void) sig;

	printf ("Terminating...\n");
	logging = false;
}

int main (int argc, char** argv)
{
	debugInit ();

	if (argc < 3)
	{
		fprintf (stderr, "Invalid arguments, usage: can-mdf-logger <options> <Device Name> <MDF file>.\n");
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

	// TODO(Barach): A lot of placeholders here.
	mdfCanBusLogConfig_t config =
	{
		// TODO(Barach): How much can I change this?
		.filePath			= argv [argc - 1],
		// TODO(Barach): Should have long-hand and short-hand versions of this.
		.programId			= "ZRECAN",
		// TODO(Barach): Want this to match the release name.
		.softwareName		= "zre_cantools",
		// TODO(Barach): Want this to match the release date.
		.softwareVersion	= __DATE__,
		.hardwareVersion	= canGetDeviceType (device),
		.serialNumber		= "0",
		.channel1Baudrate	= canGetBaudrate (device),
		.channel2Baudrate	= 0,
		.timeStart			= time (NULL),
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
	struct timeval timeStart;
	gettimeofday (&timeStart, NULL);
	struct timeval timeEnd;
	timeradd (&timeStart, &(struct timeval) { .tv_sec = 1 }, &timeEnd);

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
		struct timeval timeCurrent;
		gettimeofday (&timeCurrent, NULL);

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
		if (timercmp (&timeCurrent, &timeEnd, >))
		{
			// Calculate the actual measurement period
			struct timeval period;
			timersub (&timeCurrent, &timeStart, &period);

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
					roundf (minLoad * 100.0f / 0.6f),
					roundf (maxLoad * 100.0f / 0.6f),
					errorCount,
					errorCount >> 8
				}
			};

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
			timeradd (&timeStart, &(struct timeval) { .tv_sec = 1 }, &timeEnd);
		}
	}

	// Terminate the log gracefully
	printf ("Closing MDF file...\n");
	mdfCanBusLogClose (&log);
	canDealloc (device);

	return 0;
}