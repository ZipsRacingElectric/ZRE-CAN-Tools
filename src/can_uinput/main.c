// CAN Bus User-space Input ---------------------------------------------------------------------------------------------------
//
// Author: Cole Barach
// Date Created: 2026.03.27
//
// Description: See help page.

// Includes -------------------------------------------------------------------------------------------------------------------

// Includes
#include "abs_signal.h"
#include "key_signal.h"
#include "uinput_helper.h"
#include "can_device/can_device_stdio.h"
#include "can_database/can_database_stdio.h"
#include "cjson/cjson_util.h"
#include "debug.h"
#include "options.h"
#include "misc_port.h"

// POSIX
#include <unistd.h>

// C Standard Library
#include <signal.h>

// Globals --------------------------------------------------------------------------------------------------------------------

bool running = true;

// Help Page ------------------------------------------------------------------------------------------------------------------

void fprintUsage (FILE* stream)
{
	fprintf (stream, "Usage: can-uinput <Options> <Config JSON> <Device 0 Name> <Device 1 Name> ...\n");
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
	// Debug initialization
	debugInit ();

	// Handle program options
	if (handleOptions (&argc, &argv, &(handleOptionsParams_t)
	{
		.fprintHelp		= fprintHelp,
		.charHandlers	= NULL,
		.chars			= NULL,
		.charCount		= 0,
		.stringHandlers	= NULL,
		.strings		= NULL,
		.stringCount	= 0,
	}) != 0)
		return errorPrintf ("Failed to handle options");

	// Validate arguments
	if (argc < 1)
	{
		fprintUsage (stderr);
		return -1;
	}

	#ifdef ZRE_CANTOOLS_OS_linux

	// Load the config file

	cJSON* config = jsonLoad (argv [0]);
	if (config == NULL)
		return errorPrintf ("Failed to load config JSON");

	cJSON* deviceConfigArray = jsonGetObjectV2 (config, "canDevices");
	if (config == NULL)
		return errorPrintf ("Failed to load CAN device config array");

	// Get the number of CAN devices from the config file

	size_t deviceCount = cJSON_GetArraySize (deviceConfigArray);

	if ((size_t) argc != deviceCount + 1)
	{
		fprintf (stderr, "Missing CAN device name, expected %lu device(s).\n", (long unsigned) deviceCount);
		fprintUsage (stderr);
		return -1;
	}

	// Allocate arrays

	canDevice_t** devices = malloc (sizeof (canDevice_t*) * deviceCount);
	if (devices == NULL)
		return errorPrintf ("Failed to allocate CAN device array");

	canDatabase_t* databases = malloc (sizeof (canDatabase_t) * deviceCount);
	if (databases == NULL)
		return errorPrintf ("Failed to allocate CAN database array");

	size_t* keyCounts = malloc (sizeof (size_t) * deviceCount);
	if (keyCounts == NULL)
		return errorPrintf ("Failed to allocate key count array");

	keySignal_t** keys = malloc (sizeof (keySignal_t*) * deviceCount);
	if (keys == NULL)
		return errorPrintf ("Failed to allocate key jagged array");

	size_t* absCounts = malloc (sizeof (size_t) * deviceCount);
	if (absCounts == NULL)
		return errorPrintf ("Failed to allocate abs count array");

	absSignal_t** abs = malloc (sizeof (absSignal_t*) * deviceCount);
	if (abs == NULL)
		return errorPrintf ("Failed to allocate abs jagged array");

	// Uinput initalization

	int fd = uinputInit (true, true);
	if (fd < 0)
		return errorPrintf ("Failed to initialize uinput device");

	// Load per-device configs

	for (size_t index = 0; index < deviceCount; ++index)
	{
		cJSON* deviceConfig = cJSON_GetArrayItem (deviceConfigArray, index);

		// Get the DBC file path and expand any environment variables.

		char* dbcPath;
		if (jsonGetString (deviceConfig, "dbcFile", &dbcPath) != 0)
			return errorPrintf ("Device config is missing 'dbcFile' key");

		char* dbcPathExpanded = expandEnv (dbcPath);
		if (dbcPathExpanded == NULL)
			return errorPrintf ("Failed to expand environment variable");

		// Get the base name of the DBC file for user context.
		char* baseName = getBaseName (dbcPathExpanded);

		// Initialize the CAN device
		char* deviceName = argv [index + 1];
		devices [index] = canInit (deviceName, baseName);
		if (devices [index] == NULL)
			return errorPrintf ("Failed to initialize CAN device '%s'", deviceName);

		// Initialize the CAN database
		if (canDatabaseInit (&databases [index], devices [index], dbcPathExpanded) != 0)
			return errorPrintf ("Failed to initialize CAN database '%s'", dbcPathExpanded);

		free (baseName);
		free (dbcPathExpanded);

		keys [index] = keySignalsLoad (deviceConfig, fd, &databases [index], &keyCounts [index]);
		if (keys [index] == NULL)
			return errorPrintf ("Failed to load key signals");

		abs [index] = absSignalsLoad (deviceConfig, fd, &databases [index], &absCounts [index]);
		if (abs [index] == NULL)
			return errorPrintf ("Failed to load abs signals");
	}

	// Finish uinput setup

	if (uinputSetup (fd, 0x054C, 0x0CE6, "DualSense Wireless Controller") != 0)
		return errorPrintf ("Failed to setup uinput device");

	if (signal (SIGTERM, sigtermHandler) == SIG_ERR)
		return errorPrintf ("Failed to bind SIGTERM handler");

	if (signal (SIGINT, sigtermHandler) == SIG_ERR)
		return errorPrintf ("Failed to bind SIGINT handler");

	// Main loop

	while (running)
	{
		// Check for inputs
		for (size_t deviceIndex = 0; deviceIndex < deviceCount; ++deviceIndex)
		{
			for (size_t keyIndex = 0; keyIndex < keyCounts [deviceIndex]; ++keyIndex)
				keySignalUpdate (&keys [deviceIndex] [keyIndex]);

			for (size_t absIndex = 0; absIndex < absCounts [deviceIndex]; ++absIndex)
				absSignalUpdate (&abs [deviceIndex] [absIndex]);
		}

		// Synchronize inputs
		uinputSync (fd);

		// Sleep 10 ms
		usleep (10000);
	}

	// Termination

	uinputClose (fd);

	return 0;

	#else // ZRE_CANTOOLS_OS_linux

	fprintf (stderr, "Operating system not supported.\n");
	return -1;

	#endif // ZRE_CANTOOLS_OS_linux
}