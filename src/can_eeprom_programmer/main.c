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

// C Standard Library
#include <stdbool.h>
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

	// const char* deviceName = argv [argc - 2];
	const char* jsonPath = argv [1];

	canEeprom_t eeprom;
	// if (!canEepromLoad (&eeprom, jsonPath))
	// {
	// 	fprintf (stderr, "Failed to read CAN EEPROM JSON file '%s'.\n", jsonPath);
	// 	return -1;
	// }

	FILE* file = fopen (jsonPath, "r");
	cJSON* json = jsonPrompt (file);
	canEepromInit (&eeprom, json);
	canEepromPrintEmptyMap (&eeprom);

	// canSocket_t socket;
	// if (!canSocketInit (&socket, deviceName))
	// {
	// 	fprintf (stderr, "Failed to open CAN socket '%s'.\n", deviceName);
	// 	return -1;
	// }

	// if (!canSocketSetTimeout (&socket, 100))
	// {
	// 	fprintf (stderr, "Failed to set CAN socket timeout.\n");
	// 	return -1;
	// }

	return 0;
}