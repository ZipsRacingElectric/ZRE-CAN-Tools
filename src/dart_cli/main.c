// DART-CLI -------------------------------------------------------------------------------------------------------------------
//
// Author: Cole Barach
// Date Created: 2026.02.15
//
// Description:
//   For usage, see help page.
//   For SSH details, see https://github.com/ZipsRacingElectric/DART-ZR/blob/main/doc/ethernet_configuration.md

// Includes -------------------------------------------------------------------------------------------------------------------

// For asprintf. Note this must be the first include in this file.
#define _GNU_SOURCE
#include <stdio.h>

// Includes
#include "debug.h"
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
	MODE_UPDATE_INIT_SYSTEM,
	MODE_UPDATE_ZRE_CANTOOLS
} mode = MODE_INTERACTIVE;

char* host			= "root@192.168.0.1";
char* hostLogDir	= "/root/zre/";
char* localLogDir	= NULL;

char* updateTarget	= NULL;

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
		"dart-cli - Command-line Interface for Zips Racing's DART (Data Acquisition and\n"
		"           Racing Telemetry System).\n\n");

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
		"\n");

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

	updateTarget = value;

	debugPrintf ("Running in ZRE-CAN-Tools update mode.\n");
	mode = MODE_UPDATE_ZRE_CANTOOLS;
}

void handleUpdateInitSystemOption (char* option, char* value)
{
	(void) option;

	updateTarget = value;

	debugPrintf ("Running in init-system update mode.\n");
	mode = MODE_UPDATE_INIT_SYSTEM;
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

// Functions ------------------------------------------------------------------------------------------------------------------

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

void abortZreCantools (char* sshOptions, char* host)
{
	(void) sshOptions;
	(void) host;

	printf ("\n\nUPDATE ABORTED. MAINTENANCE MAY BE REQUIRED.\n\n");
}

void recoverZreCantools (char* sshOptions, char* host)
{
	printf ("\n\nUPDATE FAILED. ATTEMPTING RECOVERY...\n\n");

	// Remove the failed version of ZRE-CAN-Tools.
	if (systemf ("ssh %s %s \"rm -rf /root/zre_cantools/\"", sshOptions, host) != 0)
	{
		printf ("FAILED TO RECOVER FIRMWARE. MAINTENANCE REQUIRED.\n\n");
		return;
	}

	// Restore the old version
	if (systemf ("ssh %s %s \"mv /root/zre_cantools_backup/ /root/zre_cantools/\"", sshOptions, host) != 0)
	{
		printf ("FAILED TO RECOVER FIRMWARE. MAINTENANCE REQUIRED.\n\n");
		return;
	}

	// Restart the init-system
	if (systemf ("ssh %s %s \"systemctl restart init_system\"", sshOptions, host) != 0)
	{
		printf ("FAILED TO RECOVER FIRMWARE. MAINTENANCE REQUIRED.\n\n");
		return;
	}

	printf ("FIRMWARE RECOVERY SUCCESSFUL.\n\n");
}

void updateZreCantools (char* localPath, char* sshOptions, char* host)
{
	if (localPath == NULL)
	{
		printf ("Failed to update ZRE-CAN-Tools: No local target path provided.\n");
		return;
	}

	// Stop the init-system
	if (systemf ("ssh %s %s \"systemctl stop init_system\"", sshOptions, host) != 0)
	{
		abortZreCantools (sshOptions, host);
		return;
	}

	// Backup the DART's version of ZRE-CAN-Tools
	if (systemf ("ssh %s %s \"mv /root/zre_cantools/ /root/zre_cantools_backup/\"", sshOptions, host) != 0)
	{
		abortZreCantools (sshOptions, host);
		return;
	}

	// Create the destination directory
	if (systemf ("ssh %s %s \"mkdir -p /root/zre_cantools\"", sshOptions, host) != 0)
	{
		recoverZreCantools (sshOptions, host);
		return;
	}

	// Copy the target version of ZRE-CAN-Tools onto the DART
	bool successful = true;
	successful &= systemf ("scp %s -r %s/config/ %s:/root/zre_cantools/", sshOptions, localPath, host) == 0;
	successful &= systemf ("scp %s -r %s/src/ %s:/root/zre_cantools/", sshOptions, localPath, host) == 0;
	successful &= systemf ("scp %s -r %s/lib/ %s:/root/zre_cantools/", sshOptions, localPath, host) == 0;
	successful &= systemf ("scp %s %s/include.mk %s:/root/zre_cantools/", sshOptions, localPath, host) == 0;
	successful &= systemf ("scp %s %s/makefile %s:/root/zre_cantools/", sshOptions, localPath, host) == 0;
	if (!successful)
	{
		recoverZreCantools (sshOptions, host);
		return;
	}

	// Compile ZRE-CAN-Tools on the DART
	if (systemf ("ssh %s %s \"make -C /root/zre_cantools/\"", sshOptions, host) != 0)
	{
		recoverZreCantools (sshOptions, host);
		return;
	}

	// Restart the init-system
	if (systemf ("ssh %s %s \"systemctl restart init_system\"", sshOptions, host) != 0)
	{
		recoverZreCantools (sshOptions, host);
		return;
	}

	// Delete the old version of ZRE-CAN-Tools
	if (systemf ("ssh %s %s \"rm -rf /root/zre_cantools_backup/\"", sshOptions, host) != 0)
	{
		recoverZreCantools (sshOptions, host);
		return;
	}

	printf ("\n\nFIRMWARE UPDATE SUCCESSFUL.\n\n");
}

void abortInitSystem (char* sshOptions, char* host)
{
	(void) sshOptions;
	(void) host;

	printf ("\n\nUPDATE ABORTED. MAINTENANCE MAY BE REQUIRED.\n\n");
}

void recoverInitSystem (char* sshOptions, char* host)
{
	printf ("\n\nUPDATE FAILED. ATTEMPTING RECOVERY...\n\n");

	// Remove the failed version of the init-system.
	if (systemf ("ssh %s %s \"rm -rf /root/init_system/\"", sshOptions, host) != 0)
	{
		printf ("FAILED TO RECOVER FIRMWARE. MAINTENANCE REQUIRED.\n\n");
		return;
	}

	// Restore the old version
	if (systemf ("ssh %s %s \"mv /root/init_system_backup/ /root/init_system/\"", sshOptions, host) != 0)
	{
		printf ("FAILED TO RECOVER FIRMWARE. MAINTENANCE REQUIRED.\n\n");
		return;
	}

	// Restart the init-system
	if (systemf ("ssh %s %s \"systemctl restart init_system\"", sshOptions, host) != 0)
	{
		printf ("FAILED TO RECOVER FIRMWARE. MAINTENANCE REQUIRED.\n\n");
		return;
	}

	printf ("FIRMWARE RECOVERY SUCCESSFUL.\n\n");
}

void updateInitSystem (char* localPath, char* sshOptions, char* host)
{
	if (localPath == NULL)
	{
		printf ("Failed to update Init-system: No local target path provided.\n");
		return;
	}

	// Stop the init-system
	if (systemf ("ssh %s %s \"systemctl stop init_system\"", sshOptions, host) != 0)
	{
		abortInitSystem (sshOptions, host);
		return;
	}

	// Backup the DART's version of the init-system
	if (systemf ("ssh %s %s \"mv /root/init_system/ /root/init_system_backup/\"", sshOptions, host) != 0)
	{
		abortInitSystem (sshOptions, host);
		return;
	}

	// Copy the target version of the init-system onto the DART
	if (systemf ("scp %s -r %s %s:/root/", sshOptions, localPath, host) != 0)
	{
		recoverInitSystem (sshOptions, host);
		return;
	}

	// Compile the init_system on the DART
	if (systemf ("ssh %s %s \"make -C /root/init_system/\"", sshOptions, host) != 0)
	{
		recoverInitSystem (sshOptions, host);
		return;
	}

	// Restart the init-system
	if (systemf ("ssh %s %s \"systemctl restart init_system\"", sshOptions, host) != 0)
	{
		recoverInitSystem (sshOptions, host);
		return;
	}

	// Delete the old version of the init-system
	if (systemf ("ssh %s %s \"rm -rf /root/init_system_backup/\"", sshOptions, host) != 0)
	{
		recoverInitSystem (sshOptions, host);
		return;
	}

	printf ("\n\nFIRMWARE UPDATE SUCCESSFUL.\n\n");
}

// Entrypoint -----------------------------------------------------------------------------------------------------------------

int main (int argc, char** argv)
{
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
		},
		.strings = (char* [])
		{
			"ssh",
			"scp",
			"update-zre-cantools",
			"update-init-system",
			"host",
			"host-log-dir",
			"local-log-dir"
		},
		.stringCount = 5
	}) != 0)
		return errorPrintf ("Failed to handle options");

	// Get the ZRE-CAN-Tools directory for the key file
	char* zreCantoolsDir = getenv ("ZRE_CANTOOLS_DIR");
	if (zreCantoolsDir == NULL)
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
		"-o ConnectTimeout=4", zreCantoolsDir) < 0)
		errorPrintf ("Failed to allocate SSH option buffer");

	debugPrintf ("Using SSH options '%s'...\n", sshOptions);

	// SSH Mode ---------------------------------------------------------------------------------------------------------------

	if (mode == MODE_SSH)
	{
		char* command = concetenateCommand ("ssh", sshOptions, argv, argc);
		if (command == NULL)
			errorPrintf ("Failed to allocate command buffer");

		debugPrintf ("Executing command '%s'...\n", command);
		int code = system (command);
		free (command);

		return code;
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

	// Update ZRE-CAN-Tools / Init-System Modes -------------------------------------------------------------------------------

	if (mode == MODE_UPDATE_ZRE_CANTOOLS)
	{
		updateZreCantools (updateTarget, sshOptions, host);
		return 0;
	}

	if (mode == MODE_UPDATE_INIT_SYSTEM)
	{
		updateInitSystem (updateTarget, sshOptions, host);
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

	// Allocate commands

	char* testCommand;
	if (asprintf (&testCommand, "ssh %s %s \"printf Connected.\"", sshOptions, host) < 0)
		return errorPrintf ("Failed to allocate command buffer");

	char* listCommand;
	if (asprintf (&listCommand, "ssh %s %s \"ls %s\"", sshOptions, host, hostLogDir) < 0)
		return errorPrintf ("Failed to allocate command buffer");

	char* diskUsageCommand;
	if (asprintf (&diskUsageCommand, "ssh %s %s \"df %s\"", sshOptions, host, hostLogDir) < 0)
		return errorPrintf ("Failed to allocate command buffer");

	char* copyCommand;
	if (asprintf (&copyCommand, "scp %s -r %s:%s \"%s\"", sshOptions, host, hostLogDir, destinationDirectory) < 0)
		return errorPrintf ("Failed to allocate command buffer");

	// TODO(Barach): Needs to expand environment variable, also wildcard.
	char* copyDbcCommand;
	if (asprintf (&copyDbcCommand, "scp %s %s:/root/zre_cantools/config/zr25/vehicle/main.dbc %s", sshOptions, host, destinationDirectory) < 0)
		return errorPrintf ("Failed to allocate command buffer");

	char* deleteCommand;
	if (asprintf (&deleteCommand, "ssh %s %s \"rm -r %s/* && systemctl restart init_system\"", sshOptions, host, hostLogDir) < 0)
		return errorPrintf ("Failed to allocate command buffer");

	char* reloadCommand;
	if (asprintf (&reloadCommand, "ssh %s %s \"systemctl daemon-reload\"", sshOptions, host) < 0)
		return errorPrintf ("Failed to allocate command buffer");

	char* restartCommand;
	if (asprintf (&restartCommand, "ssh %s %s \"systemctl restart init_system\"", sshOptions, host) < 0)
		return errorPrintf ("Failed to allocate command buffer");

	char* modifyConfigCommand;
	if (asprintf (&modifyConfigCommand, "ssh %s %s -t \"nano /etc/systemd/system/init_system.service\"", sshOptions, host) < 0)
		return errorPrintf ("Failed to allocate command buffer");

	char* sshInteractiveCommand;
	if (asprintf (&sshInteractiveCommand, "ssh %s %s", sshOptions, host) < 0)
		return errorPrintf ("Failed to allocate command buffer");

	char* journalCommand;
	if (asprintf (&journalCommand, "ssh %s %s \"journalctl -u init_system\"", sshOptions, host) < 0)
		return errorPrintf ("Failed to allocate command buffer");

	char* dmesgCommand;
	if (asprintf (&dmesgCommand, "ssh %s %s \"dmesg\"", sshOptions, host) < 0)
		return errorPrintf ("Failed to allocate command buffer");

	// Main loop

	printf ("\nTesting Connection... (Ctrl+C to Cancel)\n");
	if (system (testCommand) != 0)
	{
		printf ("\nFailed to connect to the DART.\n\n");
		return -1;
	}
	printf ("\n\n");

	while (true)
	{
		char selection;
		printf (""
			"Enter an option:\n"
			" l - List all remote log files\n"
			" c - Copy all logs locally\n"
			" x - Delete all remote log files\n"
			" r - Restart the device\n"
			" m - Modify the DART's configuration\n"
			" s - Open an interactive SSH connection to the DART\n"
			" j - Print the DART's system journal\n"
			" d - Print the DART's kernel message buffer\n"
			" t - Test connection to the DART\n"
			" q - Quit\n");
		fscanf (stdin, "%c%*1[\n]", &selection);

		switch (selection)
		{
		// List
		case 'l':
			printf ("\nRemote Logs:\n\n");
			system (listCommand);
			printf ("\n");
			system (diskUsageCommand);
			printf ("\n");
			break;

		// Copy
		case 'c':
			printf ("\nCopying Logs to '%s'...\n\n", destinationDirectory);
			mkdirPort (destinationDirectory);
			system (copyCommand);
			system (copyDbcCommand);
			printf ("\nDone.\n\n");
			break;

		// Delete
		case 'x':
			printf ("\nAre you sure? [y/n]: ");
			fscanf (stdin, "%c%*1[\n]", &selection);
			if (selection != 'y')
			{
				printf ("Cancelled.\n\n");
				break;
			}

			printf ("\nDeleting Logs...\n\n");
			system (deleteCommand);
			printf ("\nDone.\n\n");
			break;

		// Restart
		case 'r':
			printf ("\nThis may corrupt the current data log. Are you sure? [y/n]: ");
			fscanf (stdin, "%c%*1[\n]", &selection);
			if (selection != 'y')
			{
				printf ("Cancelled.\n\n");
				break;
			}

			printf ("\nRestarting the DART...\n\n");
			system (restartCommand);
			printf ("\nDone.\n\n");
			break;

		// Modify config
		case 'm':
			printf ("\nAre you sure? [y/n]: ");
			fscanf (stdin, "%c%*1[\n]", &selection);
			if (selection != 'y')
			{
				printf ("Cancelled.\n\n");
				break;
			}

			printf ("\nOpening Editor... (Ctrl+X to Exit)\n\n");
			system (modifyConfigCommand);
			system (reloadCommand);
			system (restartCommand);
			printf ("\nDone.\n\n");
			break;

		// Interactive SSH
		case 's':
			printf ("\nOpening SSH Connection... (Type \"exit\" to Close)\n");
			system (sshInteractiveCommand);
			printf ("\n");
			break;

		// System journal
		case 'j':
			printf ("\nFetching System Journal...\n\n");
			system (journalCommand);
			printf ("\n\n");
			break;

		// Kernel message buffer
		case 'd':
			printf ("\nFetching Kernel Message Buffer...\n\n");
			system (dmesgCommand);
			printf ("\n\n");
			break;

		// Connection test
		case 't':
			printf ("\nTesting Connection... (Ctrl+C to Cancel)\n");
			system (testCommand);
			printf ("\n\n");
			break;

		// Quit
		case 'q':
			free (sshOptions);
			free (destinationDirectory);
			free (testCommand);
			free (listCommand);
			free (diskUsageCommand);
			free (copyCommand);
			free (copyDbcCommand);
			free (deleteCommand);
			free (reloadCommand);
			free (restartCommand);
			free (modifyConfigCommand);
			free (sshInteractiveCommand);
			free (journalCommand);
			return 0;
		}
	};
}