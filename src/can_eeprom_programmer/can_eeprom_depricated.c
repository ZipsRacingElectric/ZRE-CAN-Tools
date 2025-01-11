// Header
#include "can_eeprom.h"

// C Standard Library
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Functions ------------------------------------------------------------------------------------------------------------------

// bool canEepromLoad (canEeprom_t* eeprom, const char* path)
// {
	// // Open the file for reading
	// FILE* file = fopen (path, "r");
	// if (file == NULL)
	// {
	// 	int code = errno;
	// 	fprintf (stderr, "Failed to parse EEPROM file: %s.\n", strerror (code));
	// 	return false;
	// }

	// while (!feof (file))
	// {
	// 	char keyBuffer [256];
	// 	char valueBuffer [256];
	// 	if (fscanf (file, "%s: %s\n", keyBuffer, valueBuffer) != 2)
	// 	{
	// 		fprintf (stderr, "Failed to parse EEPROM file: Expected key-value pair.\n");
	// 		return false;
	// 	}

	// 	if (strcmp (keyBuffer, KEY_NAME) == 0)
	// 	{
	// 		// Read in the name
	// 		eeprom->name = malloc (strlen (valueBuffer) + 1);
	// 		strcpy (eeprom->name, valueBuffer);
	// 	}
	// 	else if (strcmp (keyBuffer, KEY_ADDR) == 0)
	// 	{
	// 		// Read the CAN address
	// 		int addr = atoi (valueBuffer);
	// 		if (addr < 0 || addr > 0x7FF)
	// 		{
	// 			fprintf (file, "Failed to parse EEPROM file: Invalid address '%s'.\n", valueBuffer);
	// 			return false;
	// 		}
	// 		eeprom->addr = (uint16_t) addr;
	// 	}
	// 	else if (strcmp (keyBuffer, KEY_MAP) == 0)
	// 	{
	// 		// Read the map data
	// 		while (!feof (file))
	// 		{
	// 			if (fscanf (file, "\t%s: %s", keyBuffer, valueBuffer) != 2)
	// 			{
	// 				fprintf (file, "Failed to parse EEPROM file: Expected key-value pair.\n");
	// 				return false;
	// 			}

	// 			if (strcmp (keyBuffer, KEY_TYPE) == 0)
	// 			{
	// 				canEepromDatatype_t datatype;
	// 				for (datatype = 0; datatype < DATATYPE_KEY_COUNT; ++datatype)
	// 					if (strcmp (valueBuffer, DATATYPE_KEYS [datatype]) == 0)
	// 						break;

	// 				if (datatype == DATATYPE_KEY_COUNT)
	// 				{
	// 					fprintf (stderr, "Error while parsing EEPROM file: Unknown datatype '%s'.\n", valueBuffer);
	// 					return false;
	// 				}


	// 			}
	// 			else if (strcmp (keyBuffer, KEY_SIZE) == 0)
	// 			{
	// 				int size = atoi (valueBuffer);
	// 				if (size <= 0 || size > VARIABLE_SIZE_MAX)
	// 				{
	// 					fprintf (stderr, "Error while parsing EEPROM file: Invalid size '%s'.\n", valueBuffer);
	// 					return false;
	// 				}


	// 			}
	// 			else if (strcmp (keyBuffer, KEY_ADDR) == 0)
	// 			{
	// 				int addr = atoi (valueBuffer);
	// 				if (addr < 0 || addr >= 0x1000)
	// 				{
	// 					fprintf (stderr, "Error while parsing EEPROM file: Invalid EEPROM address '%s'.\n", valueBuffer);
	// 					return false;
	// 				}

	// 			}
	// 			else
	// 				fprintf (stderr, "Warning while parsing EEPROM file: Unknown key '%s', ignoring...\n", keyBuffer);
	// 		}
	// 	}
	// 	else
	// 		fprintf (stderr, "Warning while parsing EEPROM file: Unknown key '%s', ignoring...\n", keyBuffer);
	// }
// 	return false;
// }