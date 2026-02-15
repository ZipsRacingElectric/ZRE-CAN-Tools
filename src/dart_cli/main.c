// For asprintf. Note this must be the first include in this file.
#define _GNU_SOURCE
#include <stdio.h>

// Includes
#include "debug.h"
#include "options.h"
#include "misc_port.h"

// C Standard Library
#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <time.h>

void fprintUsage (FILE* stream)
{
	fprintf (stream, ""
		"Usage:\n"
		"    dart-cli <Options> <User>@<IP Address> <Private RSA Key File> <Remote Directory> <Local Directory>.\n");
}

void fprintHelp (FILE* stream)
{
	fprintf (stream, ""
		"dart-cli - Command-line Interface for Zips Racing's DART (Data Acquisition and\n"
		"           Racing Telemetry System).\n\n");

	fprintUsage (stream);

	fprintf (stream, "\n"
		"Parameters:\n"
		"\n"
		"    <User>                 - The user to login to the DART as. Typically 'zre'.\n"
		"\n"
		"    <IP Address>           - The IP address of the DART. Typically\n"
		"                             '192.168.0.1'.\n"
		"\n"
		"    <Private RSA Key File> - The path of the private RSA key to use to use for\n"
		"                             authentication with. Found in the source directory\n"
		"                             of this application.\n"
		"\n"
		"    <Remote Directory>     - The directory in the DART's filesystem to search\n"
		"                             for logging sessions in. Typically\n"
		"                             '/home/zre/mdf'\n"
		"\n"
		"    <Local Directory>      - The local directory to store copied logging\n"
		"                             sessions into. Typically defined in the\n"
		"                             ZRE_CANTOOLS_LOGGING_DIR environment variable.\n"
		"\n");

	fprintf (stream, "Options:\n\n");
	fprintOptionHelp (stream, "    ");
}

int main (int argc, char** argv)
{
	// Handle program options
	if (handleOptions (&argc, &argv, &(handleOptionsParams_t)
	{
		.fprintHelp		= fprintHelp,
		.chars			= NULL,
		.charHandlers	= NULL,
		.charCount		= 0,
		.stringHandlers	= NULL,
		.strings		= NULL,
		.stringCount	= 0
	}) != 0)
		return errorPrintf ("Failed to handle options");

	// Validate args.
	if (argc < 4)
	{
		fprintUsage (stderr);
		return -1;
	}

	// Parse args.
	char* remote = argv [0];
	char* keyPath = argv [1];
	char* remoteDirectory = argv [2];
	char* localDirectory = argv [3];

	// Allocate destination directory
	time_t timeCurrent = time (NULL);
	struct tm timeLocal;
	localtime_r (&timeCurrent, &timeLocal);
	char* destinationDirectory;
	if (asprintf (&destinationDirectory, "%s/dart_%02i.%02i.%02i", localDirectory, timeLocal.tm_year + 1900, timeLocal.tm_mon + 1, timeLocal.tm_mday) < 0)
		return errorPrintf ("Failed to allocate destination directory buffer");

	debugPrintf ("Using destination directory '%s'...\n", destinationDirectory);

	// Allocate SSH options
	char* sshOptions;
	if (asprintf (&sshOptions, ""
		"-i %s "
		"-o StrictHostKeyChecking=no "
		"-o UserKnownHostsFile=/dev/null "
		"-o LogLevel=ERROR "
		"-o ConnectTimeout=4", keyPath) < 0)
		errorPrintf ("Failed to allocate SSH option buffer");

	debugPrintf ("Using SSH options '%s'...\n", sshOptions);

	// Allocate commands

	char* testCommand;
	if (asprintf (&testCommand, "ssh %s %s \"printf Connected.\"", sshOptions, remote) < 0)
		return errorPrintf ("Failed to allocate command buffer");

	char* listCommand;
	if (asprintf (&listCommand, "ssh %s %s \"ls %s\"", sshOptions, remote, remoteDirectory) < 0)
		return errorPrintf ("Failed to allocate command buffer");

	char* diskUsageCommand;
	if (asprintf (&diskUsageCommand, "ssh %s %s \"df %s\"", sshOptions, remote, remoteDirectory) < 0)
		return errorPrintf ("Failed to allocate command buffer");

	char* copyCommand;
	if (asprintf (&copyCommand, "scp %s -r %s:%s \"%s\"", sshOptions, remote, remoteDirectory, destinationDirectory) < 0)
		return errorPrintf ("Failed to allocate command buffer");

	char* deleteCommand;
	if (asprintf (&deleteCommand, "ssh %s %s \"rm -r %s/*\"", sshOptions, remote, remoteDirectory) < 0)
		return errorPrintf ("Failed to allocate command buffer");

	char* sshInteractiveCommand;
	if (asprintf (&sshInteractiveCommand, "ssh %s %s", sshOptions, remote) < 0)
		return errorPrintf ("Failed to allocate command buffer");

	char* journalCommand;
	if (asprintf (&journalCommand, "ssh %s %s \"journalctl -u init_system_jk\"", sshOptions, remote) < 0)
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
			" s - Open an interactive SSH connection to the DART\n"
			" j - Print the DART's system journal\n"
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

		// Connection test
		case 't':
			printf ("\nTesting Connection... (Ctrl+C to Cancel)\n");
			system (testCommand);
			printf ("\n\n");
			break;

		// Quit
		case 'q':
			free (destinationDirectory);
			free (sshOptions);
			free (testCommand);
			free (listCommand);
			free (diskUsageCommand);
			free (copyCommand);
			free (deleteCommand);
			free (sshInteractiveCommand);
			return 0;
		}
	};
}