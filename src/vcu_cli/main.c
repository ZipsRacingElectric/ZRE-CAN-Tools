// Vehicle Control Unit CLI ---------------------------------------------------------------------------------------------------
//
// Author: Cole Barach
// Date Created: 2024.11.11
//
// Description: Command-line interface for interacting with the vehicle control unit.

// Includes -------------------------------------------------------------------------------------------------------------------

// Includes
#include "can_database.h"
#include "vcu.h"

// C Standard Library
#include <stdio.h>

// Entrypoint -----------------------------------------------------------------------------------------------------------------

int main (int argc, char** argv)
{
	if (argc != 3)
	{
		printf ("Format: vcu-cli <device name> <DBC file path>\n");
		return -1;
	}

	const char* deviceName = argv [1];
	const char* dbcPath = argv [2];

	// Initialize the database.
	canDatabase_t database;
	if (!canDatabaseInit (&database, deviceName, dbcPath))
		return -1;

	while (true)
	{
		char selection;
		printf ("Enter an option:\n");
		printf (" w - Write to the VCU's EEPROM.\n");
		printf (" r - Read from the VCU's EEPROM.\n");
		printf (" q - Quit the program.\n");

		struct can_frame frame;

		fscanf (stdin, "%c%*1[\n]", &selection);
		switch (selection)
		{
		case 'w':
			frame = vcuEepromMessagePrompt ();
			canSocketTransmit (&database.txSocket, &frame);
			break;
		case 'r':
			printf ("Not implemented.\n");
			break;
		case 'q':
			return 0;
		}
	};
}