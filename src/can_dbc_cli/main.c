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
#include "can_device/slcan.h"

// C Standard Library
#include <errno.h>

// Function Prototypes --------------------------------------------------------------------------------------------------------

/**
 * @brief Prompts the user to input a value for the given signal.
 * @param signal The signal to prompt for.
 * @return The encoded payload.
 */
uint64_t promptSignalValue (canSignal_t* signal);

/**
 * @brief Prompts the user to input a value for every signal in a CAN message.
 * @param message The message to prompt for.
 * @return The encoded CAN frame.
 */
canFrame_t promptMessageValue (canMessage_t* message);

/**
 * @brief Prompts the user to select a database message.
 * @param database The database to select the message from.
 * @return The index of the selected message.
 */
size_t promptMessageName (canDatabase_t* database);

/**
 * @brief Prints all of the data of a database.
 * @param stream The stream to write to.
 * @param database The database to print.
 */
void printDatabase (FILE* stream, canDatabase_t* database);

/**
 * @brief Prints the last read values of a CAN message from the database.
 * @param stream The stream to print to.
 * @param database The database to read from.
 * @param index The index of the message to print.
 */
void printMessageValue (FILE* stream, canDatabase_t* database, size_t index);

// Entrypoint -----------------------------------------------------------------------------------------------------------------

int main (int argc, char** argv)
{
	char* deviceName;
	char* dbcPath; 

	canDevice_t* device = NULL;

	// Enumerate devices if one is not provided
	if (argc == 2)
	{
		// TODO(DiBacco): might want to call the slcanEnumerateDevices() function in the findCanDevice() so that slcan.h is not included everywhere?
		char* deviceNames [5];
		size_t deviceCount = 0;

		if (slcanEnumerateDevices (deviceNames, &deviceCount, "1000000") == 0)
			device = findCanDevice (deviceNames, deviceCount);	

		dbcPath = argv [1];
	}
	// Validate standard arguments
	else if (argc != 3)
	{
		fprintf (stderr, "Format: can-dbc-cli <device name> <DBC file path>\n");
		return -1;
	}
	else 
	{
		deviceName = argv [1];
		dbcPath    = argv [2];
	}

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
			messageIndex = promptMessageName (&database);
			canFrame_t frame = promptMessageValue (&database.messages [messageIndex]);
			if (canTransmit (database.device, &frame) == 0)
				printf ("Success.\n");
			else
			 	printf ("Error: %s.\n", errorMessage (errno));
			break;
		case 'r':
			messageIndex = promptMessageName (&database);
			printMessageValue (stdout, &database, messageIndex);
			break;
		case 'p':
			printDatabase (stdout, &database);
			break;
		case 'q':
			return 0;
		}
	};
}

// Function Definitions -------------------------------------------------------------------------------------------------------

uint64_t promptSignalValue (canSignal_t* signal)
{
	while (true)
	{
		printf ("%s: ", signal->name);

		float value;
		if (fscanf (stdin, "%f%*1[\n]", &value) == 1)
			return signalEncode (signal, value);

		printf ("Invalid value.\n");
	}
}

canFrame_t promptMessageValue (canMessage_t* message)
{
	canFrame_t frame =
	{
		.id = message->id,
		.dlc = message->dlc
	};
	uint64_t* payload = (uint64_t*) frame.data;

	printf ("- Message %s (0x%X) -\n", message->name, message->id);

	for (size_t index = 0; index < message->signalCount; ++index)
		*payload |= promptSignalValue (message->signals + index);

	return frame;
}

size_t promptMessageName (canDatabase_t* database)
{
	char buffer [512];

	while (true)
	{
		printf ("Enter the name of the message: ");
		fgets (buffer, sizeof (buffer), stdin);
		buffer [strcspn (buffer, "\r\n")] = '\0';

		for (size_t index = 0; index < database->messageCount; ++index)
			if (strcmp (buffer, database->messages [index].name) == 0)
				return index;

		printf ("Invalid name.\n");
	}
}

void printDatabase (FILE* stream, canDatabase_t* database)
{
	fprintf (stream, "%32s | %10s | %8s | %10s | %12s | %12s | %12s | %9s | %6s\n\n",
		"Signal Name",
		"Value",
		"Bit Mask",
		"Bit Length",
		"Bit Position",
		"Scale Factor",
		"Offset",
		"Is Signed",
		"Endian");

	for (size_t messageIndex = 0; messageIndex < database->messageCount; ++messageIndex)
	{
		canMessage_t* message = database->messages + messageIndex;

		fprintf (stream, "%s - ID: %3X\n", message->name, message->id);

		for (size_t signalIndex = 0; signalIndex < message->signalCount; ++signalIndex)
		{
			canSignal_t* signal = &message->signals [signalIndex];
			ssize_t globalIndex = canDatabaseGetGlobalIndex (database, messageIndex, signalIndex);

			char buffer [11] = "--";

			float value;
			if (canDatabaseGetFloat (database, globalIndex, &value) == CAN_DATABASE_VALID)
				snprintf (buffer, sizeof (buffer), "%.3f", value);

			fprintf (stream, "%32s | %10s | %8lX | %10i | %12i | %12f | %12f | %9u | %6u\n",
				signal->name,
				buffer,
				signal->bitmask,
				signal->bitLength,
				signal->bitPosition,
				signal->scaleFactor,
				signal->offset,
				signal->signedness,
				signal->endianness
			);
		}

		fprintf (stream, "\n");
	}
}

void printMessageValue (FILE* stream, canDatabase_t* database, size_t index)
{
	canMessage_t* message = database->messages + index;

	float* signalValues = database->signalValues + (size_t) (message->signals - database->signals);

	fprintf (stream, "- Message %s (0x%X) -\n", message->name, message->id);
	for (size_t index = 0; index < message->signalCount; ++index)
		fprintf (stream, "%s: %f\n", message->signals [index].name, signalValues [index]);
}