// CAN EEPROM Programmer ------------------------------------------------------------------------------------------------------
//
// Author: Cole Barach
// Date Created: 2025.01.11
//
// Description: TODO(Barach)

// Includes -------------------------------------------------------------------------------------------------------------------

// Includes
#include "can_eeprom.h"
#include "cjson_util.h"
#include "error_codes.h"

// C Standard Library
#include <errno.h>
#include <stdio.h>
#include <string.h>

// Global Data ----------------------------------------------------------------------------------------------------------------

enum
{
	MODE_WRITE	= 'w',
	MODE_READ	= 'r'
} mode;

char* key;
char* value;

// Standard I/O ---------------------------------------------------------------------------------------------------------------

int parseArgWrite (char* arg)
{
	key = arg + 2;

	if (key [0] == ':' || key [0] == ',' || key [0] == '-')
		++key;

	value = arg;
	strsep (&value, "=");

	if (value == NULL)
	{
		fprintf (stderr, "Invalid argument usage '%s'.\n", arg);
		return -1;
	}

	mode = MODE_WRITE;

	return 0;
}

// Entrypoint -----------------------------------------------------------------------------------------------------------------

int main (int argc, char** argv)
{
	// for (int index = 1; index < argc; ++index)
	// {
	// 	if (argv [index][0] == '-')
	// 	{
	// 		int code;

	// 		switch (argv [index][1])
	// 		{
	// 		case MODE_WRITE:
	// 			code = parseArgWrite (argv [index]);
	// 			if (code != 0)
	// 				return code;
	// 			break;
	// 		default:
	// 			fprintf (stderr, "Unknown argument '%s'.\n", argv [index]);
	// 			return -1;
	// 		}
	// 	}
	// 	else
	// 	{
	// 		fprintf (stderr, "Unknown argument '%s'.\n", argv [index]);
	// 		return -1;
	// 	}
	// }

	const char* deviceName = argv [1];
	const char* jsonPath = argv [2];

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
		fprintf (stderr, "Failed to load JSON file: %s.\n", errorMessage (code));
		return code;
	}
	if (canEepromInit (&eeprom, json) != 0)
	{
		int code = errno;
		fprintf (stderr, "Failed to initialize CAN EEPROM: %s.\n", errorMessage (code));
		return code;
	}

	canEepromPrintEmptyMap (stderr, &eeprom);

	return 0;
}