// CAN Database CLI -----------------------------------------------------------------------------------------------------------
//
// Author: Cole Barach
// Date Created: 2024.10.27
//
// Description: Command-line interface for interacting with a CAN bus.
//
// TODO(Barach):
// - Utilize stdin/stdout in the same way the EEPROM CLI does.
// - Long term, I want a shell script that can use this program to automatically clear all AMK errors (slow and tedious by
//   hand)
// - GNUPlot allows the user to pipe commands to it in realtime, meaning it can be used to monitor real-time data. Obviously
//   this is incredibly useful. For now though, that part of the toolchain is covered by MoTeC, so its not worth implementing
//   until next year.

// Includes -------------------------------------------------------------------------------------------------------------------

// Includes
#include "can_database/can_database.h"
#include "can_device/can_device.h"
#include "error_codes.h"

// C Standard Library
#include <errno.h>

// Entrypoint -----------------------------------------------------------------------------------------------------------------

int main (int argc, char** argv)
{
	if (argc != 3)
	{
		fprintf (stderr, "Format: can-dbc-cli <device name> <DBC file path>\n");
		return -1;
	}

	const char* deviceName = argv [1];
	const char* dbcPath = argv [2];

	canDevice_t* device = canInit (deviceName);
	if (device == NULL)
	{
		int code = errno;
		fprintf (stderr, "Failed to create CAN device: %s.\n", errorMessage (code));
		return code;
	}

	// Initialize the database.
	canDatabase_t database;
	if (canDatabaseInit (&database, device, dbcPath) != 0)
	{
		int code = errno;
		fprintf (stderr, "Failed to initialize CAN database: %s.\n", errorMessage (code));
		return code;
	}

	while (true)
	{
		char selection;
		printf ("Enter an option:\n");
		printf (" t - Transmit a CAN message.\n");
		printf (" r - Read a received CAN message.\n");
		printf (" p - Print a list of all messages.\n");
		printf (" q - Quit the program.\n");

		size_t messageIndex;
		fscanf (stdin, "%c%*1[\n]", &selection);
		switch (selection)
		{
		case 't':
			messageIndex = canDatabaseMessageNamePrompt (&database);
			canFrame_t frame = messagePrompt (database.messages + messageIndex);
			if (canTransmit (database.device, &frame) == 0)
				printf ("Success.\n");
			else
			 	printf ("Error: %s.\n", errorMessage (errno));
			break;
		case 'r':
			messageIndex = canDatabaseMessageNamePrompt (&database);
			canDatabaseMessageValuePrint (stdout, &database, messageIndex);
			break;
		case 'p':
			canDatabasePrint (stdout, &database);
			break;
		case 'q':
			return 0;
		}
	};
}