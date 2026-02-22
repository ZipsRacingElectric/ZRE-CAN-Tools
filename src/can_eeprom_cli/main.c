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
#include <stdlib.h>

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

void handleOptionProgram (char option, char* path)
{
	(void) option;

	// If no path is specified, use stdin
	if (path == NULL)
	{
		programStream = stdin;
	}
	else
	{
		programStream = fopen (path, "r");
		if (programStream == NULL)
			exit (errorPrintf ("Failed to open EEPROM data file '%s'", path));
	}

	mode = MODE_PROGRAM;
}

void handleOptionRecover (char option, char* path)
{
	(void) option;

	// If no path is specified, use stdout
	if (path == NULL)
	{
		recoverStream = stdout;
	}
	else
	{
		recoverStream = fopen (path, "w");
		if (recoverStream == NULL)
			exit (errorPrintf ("Failed to open EEPROM data file '%s'", path));
	}

	mode = MODE_RECOVER;
}

void fprintUsage (FILE* stream)
{
	fprintf (stream, "Usage: 'can-eeprom-cli <Options> <Device Name> <EEPROM Config Files>'.\n");
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

canEeprom_t* promptEepromSelection (canEeprom_t* eeproms, size_t eepromCount)
{
	// If only one EEPROM is available, no need to prompt.
	if (eepromCount == 1)
		return &eeproms [0];

	char buffer [512];

	while (true)
	{
		// Prompt for a device to select
		printf ("Select an EEPROM:\n");
		for (unsigned eepromIndex = 0; eepromIndex < eepromCount; ++eepromIndex)
			printf ("  %u - %s\n", eepromIndex, eeproms [eepromIndex].name);

		// Read the user input
		fgets (buffer, sizeof (buffer), stdin);
		buffer [strcspn (buffer, "\r\n")] = '\0';

		// Parse as int and validate
		char* end;
		size_t selection = (size_t) strtol (buffer, &end, 0);
		if (end == buffer || selection >= eepromCount)
		{
			// Prompt again
			printf ("Invalid selection.\n");
			continue;
		}

		return &eeproms [selection];
	}
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
		.charHandlers	= (optionCharCallback_t* []) { handleOptionProgram, handleOptionRecover },
		.chars			= (char []) { MODE_PROGRAM, MODE_RECOVER },
		.charCount		= 2,
		.stringHandlers	= NULL,
		.strings		= NULL,
		.stringCount	= 0
	}) != 0)
		return errorPrintf ("Failed to handle options");

	// Validate usage
	if (argc < 2)
	{
		fprintUsage (stderr);
		return -1;
	}

	// Initialize the CAN device
	char* deviceName = argv [0];
	canDevice_t* device = canInit (deviceName);
	if (device == NULL)
		return errorPrintf ("Failed to initialize CAN device '%s'", deviceName);

	// Set the CAN device's timeout
	if (canSetTimeout (device, 1) != 0)
		return errorPrintf ("Failed to set CAN device timeout");

	// Load the EEPROM configurations
	size_t eepromCount = argc - 1;
	canEeprom_t* eeproms = malloc (sizeof (canEeprom_t) * eepromCount);
	for (size_t index = 0; index < eepromCount; ++index)
		if (canEepromLoad (&eeproms [index], argv [index + 1]) != 0)
			return errorPrintf ("Failed to load CAN EEPROM '%s'", argv [index + 1]);

	// Prompt the user to select an EEPROM
	canEeprom_t* eeprom = promptEepromSelection (eeproms, eepromCount);

	// Programming mode
	if (mode == MODE_PROGRAM)
	{
		int code = 0;

		cJSON* dataJson = jsonRead (programStream);
		if (dataJson == NULL)
			return errorPrintf ("Failed to read EEPROM data file");

		if (canEepromWriteJson (eeprom, device, dataJson) != 0)
			code = errorPrintf ("Failed to program EEPROM data");

		fclose (programStream);
		canDealloc (device);
		return code;
	}

	// Recovery mode
	if (mode == MODE_RECOVER)
	{
		int code = 0;

		if (canEepromReadJson (eeprom, device, recoverStream) != 0)
			code = errorPrintf ("Failed to recover EEPROM data");

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
			variable = canEepromPromptVariable (eeprom, stdin, stdout);
			canEepromPromptValue (variable, eeprom->buffer, stdin, stdout);
			if (canEepromWriteVariable (eeprom, device, variable, eeprom->buffer) != 0)
				errorPrintf ("Failed to write to EEPROM");
			break;

		case 'r':
			variable = canEepromPromptVariable (eeprom, stdin, stdout);
			if (canEepromReadVariable (eeprom, device, variable, eeprom->buffer) != 0)
				errorPrintf ("Failed to read variable from EEPROM");
			else
			{
				printf ("Read: ");
				canEepromPrintVariableValue (variable, eeprom->buffer, "      ", stdout);
			}
			break;

		case 'm':
			if (canEepromPrintMap (eeprom, device, stdout) != 0)
				errorPrintf ("Failed to print EEPROM map");
			break;

		case 'e':
			canEepromPrintEmptyMap (eeprom, stdout);
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