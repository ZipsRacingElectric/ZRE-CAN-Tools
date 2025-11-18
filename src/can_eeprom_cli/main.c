// CAN EEPROM CLI -------------------------------------------------------------------------------------------------------------
//
// Author: Cole Barach
// Date Created: 2025.01.11
//
// Description: See help page.
//
// TODO(Barach):
// - Write-only data implementation is bugged, as it cannot be properly validated (can't read). The overall handshake needs
//   rev'd to include a response to all commands, not only read commands.

// Includes -------------------------------------------------------------------------------------------------------------------

// Includes
#include "can_device/can_device.h"
#include "can_device/can_device_stdio.h"
#include "can_eeprom/can_eeprom.h"
#include "can_eeprom/can_eeprom_stdio.h"
#include "cjson/cjson_util.h"
#include "debug.h"
#include "options.h"

// C Standard Library
#include <stdbool.h>

// Global Data ----------------------------------------------------------------------------------------------------------------

enum
{
	MODE_INTERACTIVE	= '\0',
	MODE_PROGRAM		= 'p',
	MODE_RECOVER		= 'r'
} mode = MODE_INTERACTIVE;

FILE* programStream;
FILE* recoverStream;

// Standard I/O ---------------------------------------------------------------------------------------------------------------

int parseOptionProgram (const char* arg)
{
	const char* path = arg + 1;

	// If no path is specified, use stdin
	if (path [0] == '\0')
	{
		programStream = stdin;
	}
	else
	{
		if (path [0] == '=')
			++path;

		programStream = fopen (path, "r");
		if (programStream == NULL)
			return errorPrintf ("Failed to open EEPROM data file '%s'", path);
	}

	mode = MODE_PROGRAM;
	return 0;
}

int parseOptionRecover (const char* arg)
{
	const char* path = arg + 1;

	// If no path is specified, use stdout
	if (path [0] == '\0')
	{
		recoverStream = stdout;
	}
	else
	{
		if (path [0] == '=')
			++path;

		recoverStream = fopen (path, "w");
		if (recoverStream == NULL)
			return errorPrintf ("Failed to open EEPROM data file '%s'", path);
	}

	mode = MODE_RECOVER;
	return 0;
}

void fprintUsage (FILE* stream)
{
	fprintf (stream, "Usage: 'can-eeprom-cli <Options> <Device Name> <EEPROM Config File>'.\n");
}

void fprintHelp (FILE* stream)
{
	fprintf (stream, ""
		"can-eeprom-cli - Command-line interface used to program a device's EEPROM via\n"
		"                 CAN bus.\n\n");

	fprintUsage (stream);

	fprintf (stream, "\nOptions:\n\n");
	fprintf (stream, ""
		"    -p=<EEPROM Data File> - Programming mode. Reads a data JSON from the\n"
		"                            specified path and programs the key-value pairs to\n"
		"                            the device. If no path is specified, the file is\n"
		"                            read from stdin.\n"
		"    -r=<EEPROM Data File> - Recovery mode. Writes the EEPROM's memory to a data\n"
		"                            JSON file. If no path is specified, the file is\n"
		"                            written to stdout.\n");
	fprintOptionHelp (stream, "    ");

	fprintf (stream, "\nParameters:\n\n");
	fprintCanDeviceNameHelp (stream, "    ");
	fprintCanEepromConfigHelp (stream, "    ");
	fprintCanEepromDataHelp (stream, "    ");
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
			case MODE_PROGRAM:
				if (parseOptionProgram (option) != 0)
					return -1;
				break;
			case MODE_RECOVER:
				if (parseOptionRecover (option) != 0)
					return -1;
				break;
			default:
				fprintf (stderr, "Unknown option '%s'.\n", argv [index]);
				return -1;
			}
			break;

		case OPTION_STRING:
			fprintf (stderr, "Unknown option '%s'.\n", argv [index]);
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

	// Set the CAN device's timeout
	if (canSetTimeout (device, 1) != 0)
		return errorPrintf ("Failed to set CAN device timeout");

	// Load the EEPROM's configuration
	char* configJsonPath = argv [argc - 1];
	cJSON* configJson = jsonLoad (configJsonPath);
	if (configJson == NULL)
		return errorPrintf ("Failed to load EEPROM config file '%s'", configJsonPath);

	// Initialize the CAN EEPROM
	canEeprom_t eeprom;
	if (canEepromInit (&eeprom, configJson) != 0)
		return errorPrintf ("Failed to initialize CAN EEPROM");

	// Programming mode
	if (mode == MODE_PROGRAM)
	{
		int code = 0;

		cJSON* dataJson = jsonRead (programStream);
		if (dataJson == NULL)
			return errorPrintf ("Failed to read EEPROM data file");

		if (canEepromWriteJson (&eeprom, device, dataJson) != 0)
			code = errorPrintf ("Failed to program '%s'", eeprom.name);

		fclose (programStream);
		canDealloc (device);
		return code;
	}

	// Recovery mode
	if (mode == MODE_RECOVER)
	{
		int code = 0;

		if (canEepromReadJson (&eeprom, device, recoverStream) != 0)
			code = errorPrintf ("Failed to recover '%s'", eeprom.name);

		fclose (recoverStream);
		canDealloc (device);
		return code;
	}

	// Interactive mode
	while (true)
	{
		printf ("Enter an option:\n");
		printf (" w - Write to the EEPROM.\n");
		printf (" r - Read from the EEPROM.\n");
		printf (" m - Print a map of the EEPROM's memory.\n");
		printf (" e - Print an empty map of the EEPROM.\n");
		printf (" q - Quit the program.\n");

		char selection;
		canEepromVariable_t* variable;

		fscanf (stdin, "%c%*1[\n]", &selection);
		switch (selection)
		{
		case 'w':
			variable = canEepromPromptVariable (&eeprom, stdin, stdout);
			canEepromPromptValue (variable, eeprom.buffer, stdin, stdout);
			if (canEepromWriteVariable (&eeprom, device, variable, eeprom.buffer) != 0)
				errorPrintf ("Failed to write to EEPROM");
			break;

		case 'r':
			variable = canEepromPromptVariable (&eeprom, stdin, stdout);
			if (canEepromReadVariable (&eeprom, device, variable, eeprom.buffer) != 0)
				errorPrintf ("Failed to read variable from EEPROM");
			else
			{
				printf ("Read: ");
				canEepromPrintVariableValue (variable, eeprom.buffer, "      ", stdout);
			}
			break;

		case 'm':
			if (canEepromPrintMap (&eeprom, device, stdout) != 0)
				errorPrintf ("Failed to print EEPROM map");
			break;

		case 'e':
			canEepromPrintEmptyMap (&eeprom, stdout);
			break;

		default:
			printf ("Enter a valid option.\n");
			break;

		case 'q':
			canDealloc (device);
			return 0;
		}
	};

	return 0;
}