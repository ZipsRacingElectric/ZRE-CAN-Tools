// AMK Inverter CLI -----------------------------------------------------------------------------------------------------------
//
// Author: Cole Barach
// Date Created: 2024.11.11
//
// Description: Command-line interface for interacting with the AMK inverter kit.
//
// To do: Still split on using the database vs hard-coding message formats. Obviously the former is more flexible, but it
//   takes quite a lot more work.

// Includes -------------------------------------------------------------------------------------------------------------------

// Includes
#include "amk.h"
#include "can_database.h"

// POSIX Libraries
#include <pthread.h>

// C Standard Library
#include <stdio.h>
#include <string.h>

// Global Data ----------------------------------------------------------------------------------------------------------------

bool stop = false;
float torqueRequest = 0.0f;
bool enabled = false;

// Function Prototypes --------------------------------------------------------------------------------------------------------

void* torqueThreadEntrypoint (void* arg);

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
	const char* deviceId = argv [3];

	amkIndex_t deviceIndex;
	if (strcmp (deviceId, "RL") == 0)
		deviceIndex = AMK_INDEX_RL;
	else if (strcmp (deviceId, "RR") == 0)
		deviceIndex = AMK_INDEX_RR;
	else if (strcmp (deviceId, "FL") == 0)
		deviceIndex = AMK_INDEX_FL;
	else if (strcmp (deviceId, "FR") == 0)
		deviceIndex = AMK_INDEX_FR;
	else
	{
		printf ("Invalid device ID, options: FL, FR, RL, RR");
		return -1;
	}

	// Initialize the database.
	canDatabase_t database;
	if (!canDatabaseInit (&database, deviceName, dbcPath))
		return -1;

	// Start the torque thread
	pthread_t torqueThread;
	pthread_create (&torqueThread, NULL, torqueThreadEntrypoint, &deviceIndex);

	while (true)
	{
		char selection;
		printf ("Enter an option:\n");
		printf (" t - Set a target torque.\n");
		printf (" q - Quit the program.\n");

		struct can_frame frame;

		// TODO(Barach)
		(void) deviceIndex;
		(void) frame;

		fscanf (stdin, "%c%*1[\n]", &selection);
		switch (selection)
		{
		case 't':

		case 'q':
			pthread_join (torqueThread, NULL);
			return 0;
		}
	};
}

// Torque Thread --------------------------------------------------------------------------------------------------------------

void* torqueThreadEntrypoint (void* arg)
{
	amkIndex_t deviceIndex = *((amkIndex_t*) arg);

	while (!stop)
	{

	}

	return NULL;
}