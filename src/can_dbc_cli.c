// CAN Database CLI -----------------------------------------------------------------------------------------------------------
//
// Author: Cole Barach
// Date Created: 2024.10.27
//
// Description: Command-line interface for interacting with a CAN bus.

// Includes -------------------------------------------------------------------------------------------------------------------

// Includes
#include "can_database.h"

// C Standard Library
#include <stdio.h>

// Entrypoint -----------------------------------------------------------------------------------------------------------------

int main (int argc, char** argv)
{
	if (argc != 3)
	{
		printf ("Format: can-dbc-cli <device name> <DBC file path>\n");
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
		printf (" t - Transmit a CAN message.\n");
		printf (" r - Read a received CAN message.\n");
		printf (" p - Print a list of all messages.\n");
		printf (" q - Quit the program.\n");

		size_t messageIndex;
		struct can_frame frame;

		fscanf (stdin, "%c%*1[\n]", &selection);
		switch (selection)
		{
		case 't':
			messageIndex = canDatabaseMessageNamePrompt (&database);
			frame = canDatabaseMessageValuePrompt (&database, messageIndex);
			if (canSocketTransmit (&database.txSocket, &frame))
				printf ("Success.\n");
			else
			 	printf ("Failure.\n");
			break;
		case 'r':
			messageIndex = canDatabaseMessageNamePrompt (&database);
			canDatabaseMessageValuePrint (&database, messageIndex);
			break;
		case 'p':
			canDatabasePrint (&database);
			break;
		case 'q':
			return 0;
		}
	};
}