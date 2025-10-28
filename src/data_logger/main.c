// Includes
#include "can_device/can_device.h"
#include "debug.h"
#include "error_codes.h"
#include "mdf/mdf_can_bus_logging.h"

// C Standard Library
#include <signal.h>
#include <sys/time.h>

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
		fprintf (stderr, "Invalid arguments, usage: data-logger <options> <CAN device> <MDF file>.\n");
		return -1;
	}

	char* deviceName = argv [argc - 2];
	canDevice_t* device = canInit (deviceName);
	if (device == NULL)
		return errorPrintf ("Failed to initialize CAN device '%s'", deviceName);

	char* mdfPath = argv [argc - 1];
	FILE* mdf = fopen (mdfPath, "w");
	if (mdf == NULL)
		return errorPrintf ("Failed to open MDF file '%s'", mdfPath);

	time_t timeStart = time (NULL);
	if (mdfCanBusLogInit (mdf, "ZREDART", timeStart) != 0)
		return errorPrintf ("Failed to initialize CAN bus MDF log");

	if (signal (SIGTERM, sigtermHandler) == SIG_ERR)
		return errorPrintf ("Failed to bind SIGTERM handler");

	if (signal (SIGINT, sigtermHandler) == SIG_ERR)
		return errorPrintf ("Failed to bind SIGINT handler");

	canSetTimeout (device, 100);

	while (logging)
	{
		canFrame_t frame;
		if (canReceive (device, &frame) != 0)
			continue;

		// printf ("Received CAN frame: 0x%03X\n", frame.id);

		struct timeval timeCurrent;
		gettimeofday (&timeCurrent, NULL);
		uint64_t timestamp = (timeCurrent.tv_sec - timeStart) * 1e6 + timeCurrent.tv_usec;

		// printf ("Timestamp: %lu.\n", timestamp);

		if (mdfCanBusLogWriteDataFrame (mdf, &frame, timestamp, 1) != 0)
		{
			errorPrintf ("Warning, failed to log CAN frame");
			continue;
		}
	}

	printf ("Closing MDF file...\n");
	fclose (mdf);

	return 0;
}