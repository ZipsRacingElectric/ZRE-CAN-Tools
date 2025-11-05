// Includes
#include "can_device/can_device.h"
#include "debug.h"
#include "error_codes.h"
#include "mdf/mdf_can_bus_logging.h"

// C Standard Library
#include <signal.h>
#include <sys/time.h>

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

	char* deviceName = argv [argc - 2];
	canDevice_t* device = canInit (deviceName);
	if (device == NULL)
		return errorPrintf ("Failed to initialize CAN device '%s'", deviceName);

	// TODO(Barach): A lot of placeholders here.
	mdfCanBusLogConfig_t config =
	{
		// TODO(Barach): How much can I change this?
		.filePath			= argv [argc - 1],
		.programId			= "ZRECAN", // TODO(Barach): Should have long-hand and short-hand versions of this.
		.softwareVersion	= __DATE__, // TODO(Barach): Want this to match the CAN-Tools release tag.
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

	if (signal (SIGTERM, sigtermHandler) == SIG_ERR)
		return errorPrintf ("Failed to bind SIGTERM handler");

	if (signal (SIGINT, sigtermHandler) == SIG_ERR)
		return errorPrintf ("Failed to bind SIGINT handler");

	// Set a receive timeout so we can check for the termination signal.
	canSetTimeout (device, 100);

	while (logging)
	{
		canFrame_t frame;
		if (canReceive (device, &frame) != 0)
			continue;

		struct timeval timeCurrent;
		gettimeofday (&timeCurrent, NULL);

		if (!frame.rtr)
		{
			if (mdfCanBusLogWriteDataFrame (&log, &frame, 1, &timeCurrent) != 0)
			{
				errorPrintf ("Warning, failed to log CAN data frame");
				continue;
			}
		}
		else
		{
			if (mdfCanBusLogWriteRemoteFrame (&log, &frame, 1, &timeCurrent) != 0)
			{
				errorPrintf ("Warning, failed to log CAN remote frame");
				continue;
			}
		}
	}

	printf ("Closing MDF file...\n");
	mdfCanBusLogClose (&log);
	canDealloc (device);

	return 0;
}