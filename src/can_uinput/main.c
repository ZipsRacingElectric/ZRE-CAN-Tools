// Includes
#include "abs_signal.h"
#include "key_signal.h"
#include "uinput_helper.h"
#include "cjson/cjson_util.h"
#include "debug.h"

// POSIX
#include <unistd.h>

// C Standard Library
#include <signal.h>

bool running = true;

void sigtermHandler (int sig)
{
	(void) sig;

	printf ("Terminating...\n");
	running = false;
}

int main (int argc, char** argv)
{
	debugInit ();

	debugSetStream (stderr);

	if (argc < 4)
	{
		// TODO(Barach): Usage
		fprintf (stderr, "Invalid usage.\n");
		return -1;
	}

	char* deviceName = argv [argc - 3];
	canDevice_t* device = canInit (deviceName);
	if (device == NULL)
		return errorPrintf ("Failed to initialize CAN device '%s'", deviceName);

	char* dbcPath = argv [argc - 2];
	canDatabase_t database;
	if (canDatabaseInit (&database, device, dbcPath) != 0)
		return errorPrintf ("Failed to initialize CAN database");

	cJSON* config = jsonLoad (argv [argc - 1]);
	if (config == NULL)
		return -1;

	int fd = uinputInit (true, true);
	if (fd < 0)
		return errorPrintf ("Failed to initialize uinput device");

	size_t signalCount;
	keySignal_t* signals = keySignalsLoad (config, fd, &database, &signalCount);
	if (signals == NULL)
		return errorPrintf ("Failed to load key signals");

	size_t absCount;
	absSignal_t* abs = absSignalsLoad (config, fd, &database, &absCount);
	if (abs == NULL)
		return errorPrintf ("Failed to load abs signals");

	if (uinputSetup (fd, 0x054C, 0x0CE6, "DualSense Wireless Controller") != 0)
		return errorPrintf ("Failed to setup uinput device");

	if (signal (SIGTERM, sigtermHandler) == SIG_ERR)
		return errorPrintf ("Failed to bind SIGTERM handler");

	if (signal (SIGINT, sigtermHandler) == SIG_ERR)
		return errorPrintf ("Failed to bind SIGINT handler");

	while (running)
	{
		for (size_t index = 0; index < signalCount; ++index)
			keySignalUpdate (&signals [index]);

		for (size_t index = 0; index < absCount; ++index)
			absSignalUpdate (&abs [index]);

		uinputSync (fd);

		usleep (10000);
	}

	uinputClose (fd);

	return 0;
}