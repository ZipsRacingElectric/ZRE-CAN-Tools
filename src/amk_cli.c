// AMK Inverter CLI -----------------------------------------------------------------------------------------------------------
//
// Author: Cole Barach
// Date Created: 2024.11.11
//
// Description: Command-line interface for interacting with the AMK inverter kit.

// Includes -------------------------------------------------------------------------------------------------------------------

// Includes
#include "amk.h"
#include "can_database.h"

// C Standard Library
#include <stdio.h>

// Entrypoint -----------------------------------------------------------------------------------------------------------------

int main (int argc, char** argv)
{
	if (argc != 4)
	{
		printf ("Format: amk-cli <device name> <DBC file path> <device ID>\n");
		return -1;
	}

	const char* deviceName = argv [1];
	const char* dbcPath = argv [2];
	uint16_t deviceId = atoi (argv [3]);

	// Initialize the database.
	canDatabase_t database;
	if (!canDatabaseInit (&database, deviceName, dbcPath))
		return -1;

	while (true)
	{
		char selection;
		printf ("Enter an option:\n");
		printf (" q - Quit the program.\n");

		struct can_frame frame;

		// TODO(Barach)
		(void) deviceId;
		(void) frame;

		fscanf (stdin, "%c%*1[\n]", &selection);
		switch (selection)
		{
		case 'q':
			return 0;
		}
	};
}