// DART-CLI -------------------------------------------------------------------------------------------------------------------
//
// Author: Cole Barach
// Date Created: 2026.02.15
//
// Description:
//   For usage, see help page.
//   For SSH details, see https://github.com/ZipsRacingElectric/DART-ZR/blob/main/doc/ethernet_configuration.md

// Includes -------------------------------------------------------------------------------------------------------------------

// TODO(Barach):
// - Support for multiple firmware updates at once

// For asprintf. Note this must be the first include in this file.
#define _GNU_SOURCE
#include <stdio.h>

// Includes
#include "firmware_update.h"
#include "debug.h"
#include "error_codes.h"
#include "options.h"
#include "misc_port.h"

// C Standard Library
#include <errno.h>
#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

// Globals --------------------------------------------------------------------------------------------------------------------

enum
{
	MODE_INTERACTIVE,
	MODE_SSH,
	MODE_SCP,
	MODE_FIRMWARE_UPDATE
} mode = MODE_INTERACTIVE;

char* host				= "root@192.168.0.1";
char* hostLogDir		= "/root/zre/";
char* localLogDir		= NULL;

char* targetZreCantoolsPath	= NULL;
char* targetInitSystemPath	= NULL;

bool testConnection = true;

// Help Page ------------------------------------------------------------------------------------------------------------------

void fprintUsage (FILE* stream)
{
	fprintf (stream, ""
		"Usage:\n"
		"    dart-cli <Options>\n"
		"    dart-cli <Options> --ssh -- <SSH Args>.\n"
		"    dart-cli <Options> --scp -- <SCP Args>.\n"
		"\n");
}

void fprintHelp (FILE* stream)
{
	fprintf (stream, ""
		"dart-cli - Command-line Interface for Zips Racing's DART (Dashboard And Racing\n"
		"           Telemetry System).\n\n");

	fprintUsage (stream);

	fprintf (stream, ""
		"Options:\n"
		"\n"
		"    --ssh                 - SSH mode. All arguments are passed directly to SSH.\n"
		"    --scp                 - SCP mode. All arguments are passed directly to SCP.\n"
		"    --host=<User>@<IP>    - Override the default host user and IP address.\n"
		"                            Default is \"root@192.168.0.1\".\n"
		"    --host-log-dir=<Dir>  - Override the default host data log directory.\n"
		"                            Default is \"/root/zre\".\n"
		"    --local-log-dir=<Dir> - Override the default local data log directory.\n"
		"                            Default is the value of the ZRE_CANTOOLS_LOGGING_DIR\n"
		"                            environment variable.\n"
		"    --skip-test           - Skips the SSH connection test at the start of the\n"
		"                            application. Not recommended when performing\n"
		"                            firmware updates.\n"
		"\n"
		"    --update-zre-cantools=<Path> - Updates the DART's version of ZRE-CAN-Tools.\n"
		"                                   Please see the URL below before using this.\n"
		"    --update-init-system=<Path>  - Updates the DART's version of the\n"
		"                                   init-system. Please see the URL below before\n"
		"                                   using this.\n"
		"\n"
		"    https://github.com/ZipsRacingElectric/DART-ZR/blob/main/doc/updating_firmware.md"
		"\n\n");

	fprintOptionHelp (stream, "    ");

	fprintf (stream, ""
		"Examples:\n"
		"\n"
		"    Run in interactive mode. User is prompted for what to do:\n"
		"        dart-cli\n"
		"\n"
		"    Open an interactive SSH connection to the DART:\n"
		"        dart-cli --ssh -- zre@192.168.0.1\n"
		"\n"
		"    Copy a local file to a remote directory:\n"
		"        dart-cli --scp -- ./temp.txt zre@192.168.0.1:/home/zre/\n"
		"\n");
}

// Standard Options -----------------------------------------------------------------------------------------------------------

void handleSshOption (char* option, char* value)
{
	(void) option;
	(void) value;

	debugPrintf ("Running in SSH mode.\n");
	mode = MODE_SSH;
}

void handleScpOption (char* option, char* value)
{
	(void) option;
	(void) value;

	debugPrintf ("Running in SCP mode.\n");
	mode = MODE_SCP;
}

void handleUpdateZreCantoolsOption (char* option, char* value)
{
	(void) option;

	targetZreCantoolsPath = value;

	debugPrintf ("Running in firmware update mode.\n");
	mode = MODE_FIRMWARE_UPDATE;
}

void handleUpdateInitSystemOption (char* option, char* value)
{
	(void) option;

	targetInitSystemPath = value;

	debugPrintf ("Running in firmware update mode.\n");
	mode = MODE_FIRMWARE_UPDATE;
}

void handleHostOption (char* option, char* value)
{
	(void) option;
	host = value;
}

void handleHostLogDirOption (char* option, char* value)
{
	(void) option;
	hostLogDir = value;
}

void handleLocalLogDirOption (char* option, char* value)
{
	(void) option;
	localLogDir = value;
}

void handleSkipTestOption (char* option, char* value)
{
	(void) option;
	(void) value;

	testConnection = false;
}

// Functions ------------------------------------------------------------------------------------------------------------------

bool promptConfirmation ()
{
	char selection;
	printf ("\nAre you sure? [y/n]: ");
	fscanf (stdin, "%c%*1[\n]", &selection);
	return selection == 'y' || selection == 'Y';
}

char* concetenateCommand (char* command, char* options, char** argv, int argc)
{
	size_t bufferSize = strlen (command) + 1 + strlen (options);
	for (int index = 0; index < argc; ++index)
		bufferSize += strlen (argv [index]) + 1;
	++bufferSize;

	char* buffer = malloc (bufferSize);
	if (buffer == NULL)
		return NULL;

	char* bufferHead = buffer;

	int count = snprintf (bufferHead, bufferSize, "%s %s", command, options);
	if (count < 0 || (size_t) count >= bufferSize)
	{
		errno = EINVAL;
		free (buffer);
		return NULL;
	}

	bufferHead += count;
	bufferSize -= count;

	for (int index = 0; index < argc; ++index)
	{
		count = snprintf (bufferHead, bufferSize, " %s", argv [index]);
		if (count < 0 || (size_t) count >= bufferSize)
		{
			errno = EINVAL;
			free (buffer);
			return NULL;
		}

		bufferHead += count;
		bufferSize -= count;
	}

	return buffer;
}

char* sshGetEnv (char* env, char* sshOptions, char* host)
{
	char* command;
	if (asprintf (&command, "ssh %s %s \"printenv %s\"", sshOptions, host, env) < 0)
		return NULL;

	FILE* sshStdout = popen (command, "r");
	if (sshStdout == NULL)
		return NULL;

	free (command);

	char* buffer = malloc (1024);

	if (fgets (buffer, 1024, sshStdout) == NULL)
	{
		if (feof (sshStdout))
		{
			errno = ERRNO_END_OF_FILE;
			return NULL;
		}

		return NULL;
	}
	buffer [strcspn (buffer, "\r\n")] = '\0';

	return buffer;
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
		.chars			= NULL,
		.charHandlers	= NULL,
		.charCount		= 0,
		.stringHandlers = (optionStringCallback_t* [])
		{
			&handleSshOption,
			&handleScpOption,
			&handleUpdateZreCantoolsOption,
			&handleUpdateInitSystemOption,
			&handleHostOption,
			&handleHostLogDirOption,
			&handleLocalLogDirOption,
			&handleSkipTestOption
		},
		.strings = (char* [])
		{
			"ssh",
			"scp",
			"update-zre-cantools",
			"update-init-system",
			"host",
			"host-log-dir",
			"local-log-dir",
			"skip-test"
		},
		.stringCount = 8
	}) != 0)
		return errorPrintf ("Failed to handle options");

	// Get the ZRE-CAN-Tools directory for the key file
	char* localZreCantoolsDir = getenv ("ZRE_CANTOOLS_DIR");
	if (localZreCantoolsDir == NULL)
	{
		fprintf (stderr, "Failed to get ZRE_CANTOOLS_DIR environment variable.\n");
		return -1;
	}

	debugPrintf ("Using host '%s'...\n", host);

	// Allocate SSH options
	char* sshOptions;
	if (asprintf (&sshOptions, ""
		"-i %s/bin/keys/id_rsa "
		"-o StrictHostKeyChecking=no "
		"-o UserKnownHostsFile=/dev/null "
		"-o LogLevel=ERROR "
		"-o ConnectTimeout=4", localZreCantoolsDir) < 0)
		errorPrintf ("Failed to allocate SSH option buffer");

	debugPrintf ("Using SSH options '%s'...\n", sshOptions);

	// SSH Mode ---------------------------------------------------------------------------------------------------------------

	if (mode == MODE_SSH)
	{
		if (argc == 0)
		{
			// No arguments, default to interactive SSH
			return systemf ("ssh %s %s", sshOptions, host);
		}
		else
		{
			// Use user-provided arguments

			char* command = concetenateCommand ("ssh", sshOptions, argv, argc);
			if (command == NULL)
				return errorPrintf ("Failed to allocate command buffer");

			debugPrintf ("Executing command '%s'...\n", command);
			int code = system (command);
			free (command);
			return code;
		}
	}

	// SCP Mode ---------------------------------------------------------------------------------------------------------------

	if (mode == MODE_SCP)
	{
		char* command = concetenateCommand ("scp", sshOptions, argv, argc);
		if (command == NULL)
			errorPrintf ("Failed to allocate command buffer");

		debugPrintf ("Executing command '%s'...\n", command);
		int code = system (command);
		free (command);

		return code;
	}

	// Test Connection --------------------------------------------------------------------------------------------------------

	if (testConnection)
	{
		printf ("\nTesting Connection... (Ctrl+C to Cancel)\n");
		if (systemf ("ssh %s %s \"printf Connected.\"", sshOptions, host) != 0)
		{
			printf ("\nFailed to connect to the DART.\n\n");
			return -1;
		}
		printf ("\n\n");
	}

	// Update Firmware Mode ---------------------------------------------------------------------------------------------------

	if (mode == MODE_FIRMWARE_UPDATE)
	{
		if (targetZreCantoolsPath != NULL)
		{
			if (updateZreCantools (targetZreCantoolsPath, sshOptions, host) != 0)
				return -1;
		}
		else
			printf ("Skipping ZRE-CAN-Tools update: No path provided.\n");

		if (targetInitSystemPath != NULL)
		{
			if (updateInitSystem (targetInitSystemPath, sshOptions, host) != 0)
				return -1;
		}
		else
			printf ("Skipping Init-system update: No path provided.\n");

		return 0;
	}

	// Interactive Mode -------------------------------------------------------------------------------------------------------

	// If not overridden, set the default local log directory.
	if (localLogDir == NULL)
	{
		localLogDir = getenv ("ZRE_CANTOOLS_LOGGING_DIR");
		if (localLogDir == NULL)
			fprintf (stderr, "Failed to get ZRE_CANTOOLS_LOGGING_DIR environment variable.\n");
	}

	// Create the local log directory if it doesn't exist.
	mkdirPort (localLogDir);
	debugPrintf ("Using local data log directory '%s'...\n", localLogDir);

	// Allocate log destination directory.
	time_t timeCurrent = time (NULL);
	struct tm* timeLocal = localtime (&timeCurrent);
	char* destinationDirectory;
	if (asprintf (&destinationDirectory, "%s/dart_%02i.%02i.%02i", localLogDir, timeLocal->tm_year + 1900, timeLocal->tm_mon + 1, timeLocal->tm_mday) < 0)
		return errorPrintf ("Failed to allocate destination directory buffer");

	debugPrintf ("Using local destination directory '%s'...\n", destinationDirectory);

	// Main loop

	while (true)
	{
		char selection;
		printf (""
			"Enter an option:\n"
			" l - List all remote log files\n"
			" c - Copy all logs locally\n"
			" x - Delete all remote log files\n"
			" t - Test connection to the DART\n"
			" a - List developer options\n"
			" q - Quit\n");
		fscanf (stdin, "%c%*1[\n]", &selection);

		switch (selection)
		{
		// List
		case 'l':
			printf ("\nRemote Logs:\n\n");
			systemf ("ssh %s %s \"ls %s\"", sshOptions, host, hostLogDir);
			printf ("\n");
			systemf ("ssh %s %s \"df %s\"", sshOptions, host, hostLogDir);
			printf ("\n");
			break;

		// Copy
		case 'c':
			printf ("\nCopying Logs to '%s'...\n\n", destinationDirectory);
			mkdirPort (destinationDirectory);
			systemf ("scp %s -r %s:%s \"%s\"", sshOptions, host, hostLogDir, destinationDirectory);

			char* hostConfigDir = sshGetEnv ("DART_CONFIG", sshOptions, host);
			if (hostConfigDir != NULL)
			{
				debugPrintf ("Using DART config directory '%s'...\n", hostConfigDir);

				systemf ("scp %s %s:%s/*.dbc %s", sshOptions, host, hostConfigDir, destinationDirectory);
				printf ("\nDone.\n\n");
			}
			else
				errorPrintf ("Failed to get DART_CONFIG environment variable");

			break;

		// Delete
		case 'x':
			if (!promptConfirmation ())
			{
				printf ("Cancelled.\n\n");
				break;
			}

			printf ("\nDeleting Logs...\n\n");
			systemf ("ssh %s %s \"rm -r %s/* && systemctl restart init_system\"", sshOptions, host, hostLogDir);
			printf ("\nDone.\n\n");
			break;

		// Advanced Options
		case 'a':
			printf ("\n"
			"Developer Options:\n"
			" p - Stop the data logger\n"
			" r - Restart the device\n"
			" m - Modify the DART's configuration\n"
			" s - Open an interactive SSH connection to the DART\n"
			" j - Print the DART's system journal\n"
			" i - Print the DART's init-system journal\n"
			" d - Print the DART's kernel message buffer\n\n");
			break;

		// Stop
		case 'p':
			printf ("\nStopping...\n\n");
			systemf ("ssh %s %s \"systemctl stop init_system\"", sshOptions, host);
			printf ("\nStopped.\n\n");
			break;

		// Restart
		case 'r':
			printf ("\nRestarting the DART...\n\n");
			systemf ("ssh %s %s \"systemctl restart init_system\"", sshOptions, host);
			printf ("\nDone.\n\n");
			break;

		// Modify config
		case 'm':
			if (!promptConfirmation ())
			{
				printf ("Cancelled.\n\n");
				break;
			}

			printf ("\nOpening Editor... (Ctrl+X to Exit)\n\n");
			systemf ("ssh %s %s -t \"nano /etc/systemd/system/init_system.service\"", sshOptions, host);
			systemf ("ssh %s %s \"systemctl daemon-reload\"", sshOptions, host);
			systemf ("ssh %s %s \"systemctl restart init_system\"", sshOptions, host);
			printf ("\nDone.\n\n");
			break;

		// Interactive SSH
		case 's':
			printf ("\nOpening SSH Connection... (Type \"exit\" to Close)\n");
			systemf ("ssh %s %s", sshOptions, host);
			printf ("\n");
			break;

		// System journal
		case 'j':
			printf ("\nFetching System Journal...\n\n");
			systemf ("ssh %s %s \"journalctl\"", sshOptions, host);
			printf ("\n\n");
			break;

		// Init-system journal
		case 'i':
			printf ("\nFetching Init-system Journal...\n\n");
			systemf ("ssh %s %s \"journalctl -u init_system\"", sshOptions, host);
			printf ("\n\n");
			break;

		// Kernel message buffer
		case 'd':
			printf ("\nFetching Kernel Message Buffer...\n\n");
			systemf ("ssh %s %s \"dmesg\"", sshOptions, host);
			printf ("\n\n");
			break;

		// Connection test
		case 't':
			printf ("\nTesting Connection... (Ctrl+C to Cancel)\n");
			systemf ("ssh %s %s \"printf Connected.\"", sshOptions, host);
			printf ("\n\n");
			break;

		// Quit
		case 'q':
			free (sshOptions);
			free (destinationDirectory);
			return 0;
		}
	};
}