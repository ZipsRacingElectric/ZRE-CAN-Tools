// CAN EEPROM CLI -------------------------------------------------------------------------------------------------------------
//
// Author: Cole Barach
// Date Created: 2025.01.11
//
// Description: Command-line interface for interacting with a device's EEPROM through a CAN bus. See this project's readme for
//   usage details.
//
// TODO(Barach):
// - '-h' & '-v' options.
// - Figure out matrices. Prompt, parse, read, write, and print aren't hard, given the memory is already allocated. Allocation
//   is the issue I see.
// - Maybe a 'variableAllocateBuffer' function wrapping malloc?

// Includes -------------------------------------------------------------------------------------------------------------------

// Includes
#include "can_eeprom.h"
#include "error_codes.h"

// C Standard Library
#include <errno.h>

// Global Data ----------------------------------------------------------------------------------------------------------------

enum
{
	MODE_INTERACTIVE	= '\0',
	MODE_PROGRAM		= 'p',
	MODE_RECOVER		= 'r'
} mode = MODE_INTERACTIVE;

cJSON* programDataJson = NULL;
FILE* recoverStream;

// Standard I/O ---------------------------------------------------------------------------------------------------------------

int parseOptionProgram (char* arg)
{
	char* path = arg + 2;

	// If no path is specified, use stdin
	if (path [0] == '\0')
	{
		programDataJson = jsonRead (stdin);
	}
	else
	{
		if (path [0] == '=')
			++path;

		programDataJson = jsonLoad (path);
	}

	if (programDataJson == NULL)
	{
		int code = errno;
		fprintf (stderr, "Failed to read data JSON: %s.\n", errorMessage (code));
		return code;
	}

	mode = MODE_PROGRAM;

	return 0;
}

int parseOptionRecover (char* arg)
{
	char* path = arg + 2;

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
		{
			int code = errno;
			fprintf (stderr, "Failed to open JSON '%s': %s.\n", path, errorMessage (code));
			return code;
		}
	}

	mode = MODE_RECOVER;

	return 0;
}

// Entrypoint -----------------------------------------------------------------------------------------------------------------

int main (int argc, char** argv)
{
	if (argc < 3)
	{
		fprintf (stderr, "Missing arguments, usage: 'can-eeprom-cli <options> <device> <json>'.\n");
		return EINVAL;
	}

	for (int index = 1; index < argc - 2; ++index)
	{
		if (argv [index][0] == '-')
		{
			switch (argv [index][1])
			{
			case MODE_PROGRAM:
				if (parseOptionProgram (argv [index]) != 0)
					return -1;
				break;
			case MODE_RECOVER:
				if (parseOptionRecover (argv [index]) != 0)
					return -1;
				break;
			default:
				fprintf (stderr, "Unknown option '%s'.\n", argv [index]);
				return -1;
			}
		}
		else
		{
			fprintf (stderr, "Unknown option '%s'.\n", argv [index]);
			return -1;
		}
	}

	const char* deviceName = argv [argc - 2];
	const char* jsonPath = argv [argc - 1];

	canSocket_t socket;
	if (canSocketInit (&socket, deviceName) != 0)
	{
		int code = errno;
		fprintf (stderr, "Failed to open CAN socket '%s': %s.\n", deviceName, errorMessage (code));
		return code;
	}
	if (canSocketSetTimeout (&socket, 100) != 0)
	{
		int code = errno;
		fprintf (stderr, "Failed to set CAN socket timeout: %s\n", errorMessage (code));
		return errno;
	}

	canEeprom_t eeprom;
	cJSON* json = jsonLoad (jsonPath);
	if (json == NULL)
	{
		int code = errno;
		fprintf (stderr, "Failed to load address JSON file: %s.\n", errorMessage (code));
		return code;
	}
	if (canEepromInit (&eeprom, json) != 0)
	{
		int code = errno;
		fprintf (stderr, "Failed to initialize CAN EEPROM: %s.\n", errorMessage (code));
		return code;
	}

	if (mode == MODE_PROGRAM)
	{
		if (canEepromWriteJson (&eeprom, &socket, programDataJson) != 0)
		{
			int code = errno;
			fprintf (stderr, "Failed to program EEPROM: %s.\n", errorMessage (code));
			return code;
		}
	}
	else if (mode == MODE_RECOVER)
	{
		if (canEepromReadJson (&eeprom, &socket, recoverStream) != 0)
		{
			fclose (recoverStream);

			int code = errno;
			fprintf (stderr, "Failed to recover EEPROM: %s.\n", errorMessage (code));
			return code;
		}

		fclose (recoverStream);
	}
	else if (mode == MODE_INTERACTIVE)
	{
		while (true)
		{
			printf ("Enter an option:\n");
			printf (" w - Write to the EEPROM.\n");
			printf (" r - Read from the EEPROM.\n");
			printf (" m - Print a map of the EEPROM's memory.\n");
			printf (" e - Print an empty map of the EEPROM.\n");
			printf (" v - Validate the EEPROM.\n");
			printf (" i - Invalidate the EEPROM.\n");
			printf (" c - Check the EEPROM's validity.\n");
			printf (" q - Quit the program.\n");

			char selection;
			canEepromVariable_t* variable;
			uint8_t buffer [4];

			fscanf (stdin, "%c%*1[\n]", &selection);
			switch (selection)
			{
			case 'w':
				variable = canEepromPromptVariable (&eeprom, stdin, stdout);
				canEepromPromptValue (variable, buffer, stdin, stdout);
				if (canEepromWriteVariable (&eeprom, &socket, variable, buffer) != 0)
					printf ("Failed to write to EEPROM: %s.\n", errorMessage (errno));
				break;

			case 'r':
				variable = canEepromPromptVariable (&eeprom, stdin, stdout);
				if (canEepromReadVariable (&eeprom, &socket, variable, buffer) != 0)
				 	printf ("Failed to read variable from EEPROM: %s.\n", errorMessage (errno));
				else
				{
					printf ("Read: ");
					canEepromPrintVariable (variable, buffer, stdout);
					printf (".\n");
				}
				break;

			case 'm':
				if (canEepromPrintMap (&eeprom, &socket, stdout) != 0)
					printf ("Failed to print EEPROM map: %s.\n", errorMessage (errno));
				break;

			case 'e':
				canEepromPrintEmptyMap (&eeprom, stdout);
				break;

			case 'v':
				if (canEepromValidate (&eeprom, &socket, true) != 0)
					printf ("Failed to validate EEPROM: %s.\n", errorMessage (errno));
				break;

			case 'i':
				if (canEepromValidate (&eeprom, &socket, false) != 0)
					printf ("Failed to invalidate EEPROM: %s.\n", errorMessage (errno));
				break;

			case 'c':
				bool isValid;
				if (canEepromIsValid (&eeprom, &socket, &isValid) != 0)
				 	printf ("Failed to check EEPROM validity: %s.\n", errorMessage (errno));
				else
					printf ("%s.\n", isValid ? "Valid" : "Invalid");
				break;

			case 'q':
				return 0;
			}
		};
	}

	return 0;
}