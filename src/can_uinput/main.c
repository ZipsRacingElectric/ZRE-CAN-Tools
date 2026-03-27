// Includes
#include "abs_signal.h"
#include "key_signal.h"
#include "uinput_helper.h"
#include "can_device/can_device_stdio.h"
#include "can_database/can_database_stdio.h"
#include "cjson/cjson_util.h"
#include "debug.h"
#include "options.h"

// POSIX
#include <unistd.h>

// C Standard Library
#include <signal.h>

// Globals --------------------------------------------------------------------------------------------------------------------

bool running = true;

// Help Page ------------------------------------------------------------------------------------------------------------------

void fprintUsage (FILE* stream)
{
	fprintf (stream, "Usage: can-uinput <Options> <Device Name> <DBC File> <Config JSON>.\n");
}

void fprintHelp (FILE* stream)
{
	fprintf (stream, ""
		"can-uinput - Linux-only application for emulating keyboard/controller input\n"
		"             from CAN bus signals.\n\n");

	fprintUsage (stream);

	fprintf (stream, "\nParameters:\n\n");
	fprintCanDeviceNameHelp (stream, "    ");
	fprintCanDbcFileHelp (stream, "    ");

	fprintf (stream, "Options:\n\n");
	fprintOptionHelp (stream, "    ");
}

// Functions ------------------------------------------------------------------------------------------------------------------

void sigtermHandler (int sig)
{
	(void) sig;

	printf ("Terminating...\n");
	running = false;
}

// Entrypoint -----------------------------------------------------------------------------------------------------------------

int main (int argc, char** argv)
{
	debugInit ();

	// TODO(Barach): Update

	debugSetStream (stderr);

	if (argc < 4)
	{
		// fprintUsage (stderr);
		fprintHelp (stderr);
		return -1;
	}

	#ifdef ZRE_CANTOOLS_OS_linux

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

	#else // ZRE_CANTOOLS_OS_linux

	fprintf (stderr, "Operating system not supported.\n");
	return -1;

	#endif // ZRE_CANTOOLS_OS_linux
}