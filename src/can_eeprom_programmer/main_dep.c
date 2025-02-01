// Includes -------------------------------------------------------------------------------------------------------------------

// Includes
#include "can_eeprom.h"

// C Standard Library
#include <stdio.h>

// Entrypoint -----------------------------------------------------------------------------------------------------------------

int main (int argc, char** argv)
{
	if (argc != 3)
	{
		fprintf (stderr, "Format: can-eeprom-programmer <device name> <json path>\n");
		return -1;
	}

	const char* deviceName = argv [1];
	const char* jsonPath = argv [2];

	canEeprom_t eeprom;
	if (!canEepromLoad (&eeprom, jsonPath))
	{
		fprintf (stderr, "Failed to read CAN EEPROM JSON file '%s'.\n", jsonPath);
		return -1;
	}

	canSocket_t socket;
	if (!canSocketInit (&socket, deviceName))
	{
		fprintf (stderr, "Failed to open CAN socket '%s'.\n", deviceName);
		return -1;
	}

	if (!canSocketSetTimeout (&socket, 100))
	{
		fprintf (stderr, "Failed to set CAN socket timeout.\n");
		return -1;
	}

	while (true)
	{
		char selection;
		printf ("Enter an option:\n");
		printf (" w - Write to the EEPROM.\n");
		printf (" r - Read from the EEPROM.\n");
		printf (" m - Print a map of the EEPROM's memory.\n");
		printf (" e - Print an empty map of the EEPROM's memory.\n");
		printf (" v - Validate the EEPROM.\n");
		printf (" i - Invalidate the EEPROM.\n");
		printf (" c - Check the EEPROM's validity.\n");
		printf (" q - Quit the program.\n");

		uint16_t index;
		uint8_t data [4];

		fscanf (stdin, "%c%*1[\n]", &selection);
		switch (selection)
		{
		case 'w':
			index = canEepromVariableIndexPrompt (&eeprom);
			canEepromVariableValuePrompt (&eeprom, index, data);
			canEepromWrite (&eeprom, &socket, index, data);
			break;
		case 'r':
			index = canEepromVariableIndexPrompt (&eeprom);
			if (canEepromRead (&eeprom, &socket, index, data))
				canEepromPrintVariable (&eeprom, index, data);
			break;
		case 'm':
			canEepromPrintMap (&eeprom, &socket);
			break;
		case 'e':
			canEepromPrintEmptyMap (&eeprom);
			break;
		case 'v':
			canEepromValidate (&eeprom, &socket, true);
			break;
		case 'i':
			canEepromValidate (&eeprom, &socket, false);
			break;
		case 'c':
			bool isValid;
			if (canEepromIsValid (&eeprom, &socket, &isValid))
				printf ("%s.\n", isValid ? "Valid" : "Invalid");
			break;
		case 'q':
			return 0;
		}
	};
}