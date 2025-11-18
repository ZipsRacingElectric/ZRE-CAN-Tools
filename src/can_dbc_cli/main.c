// CAN Database CLI -----------------------------------------------------------------------------------------------------------
//
// Author: Cole Barach
// Date Created: 2024.10.27
//
// Description: See help page.

// Includes -------------------------------------------------------------------------------------------------------------------

// Includes
#include "can_database/can_database.h"
#include "can_database/can_database_stdio.h"
#include "can_device/can_device.h"
#include "can_device/can_device_stdio.h"
#include "debug.h"
#include "options.h"

// C Standard Library
#include <inttypes.h>

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

// Functions ------------------------------------------------------------------------------------------------------------------

void fprintUsage (FILE* stream)
{
	fprintf (stream, "Usage: can-dbc-cli <Options> <Device Name> <DBC file path>\n");
}

void fprintHelp (FILE* stream)
{
	fprintf (stream, ""
		"can-dbc-cli - Command-line interface used to interact with a CAN bus. Received\n"
		"              messages are parsed and stored in a relational database which can\n"
		"              be queried. Arbitrary messages can be transmitted by the user.\n\n");

	fprintUsage (stream);

	fprintf (stream, "\nParameters:\n\n");
	fprintCanDeviceNameHelp (stream, "    ");
	fprintCanDbcFileHelp (stream, "    ");

	fprintf (stream, "Options:\n\n");
	fprintOptionHelp (stream, "    ");
}

// Entrypoint -----------------------------------------------------------------------------------------------------------------

int main (int argc, char** argv)
{
	// Debug initialization
	debugInit ();

	// Check standard arguments
	for (int index = 1; index < argc; ++index)
	{
		switch (handleOption (argv [index], NULL, fprintHelp))
		{
		case OPTION_CHAR:
		case OPTION_STRING:
			fprintf (stderr, "Unknown argument '%s'.\n", argv [index]);
			return -1;

		case OPTION_QUIT:
			return 0;

		default:
			break;
		}
	}

	// Validate usage
	if (argc < 3)
	{
		fprintUsage (stderr);
		return -1;
	}

	// Initialize the CAN device
	char* deviceName = argv [argc - 2];
	canDevice_t* device = canInit (deviceName);
	if (device == NULL)
		return errorPrintf ("Failed to initialize CAN device '%s'", deviceName);

	// Initialize the CAN database
	char* dbcPath = argv [argc - 1];
	canDatabase_t database;
	if (canDatabaseInit (&database, device, dbcPath) != 0)
		return errorPrintf ("Failed to initialize CAN database");

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
			 	errorPrintf ("Failed to transmit CAN frame");
			break;
		case 'r':
			messageIndex = promptMessageName (&database);
			printMessageValue (stdout, &database, messageIndex);
			break;
		case 'p':
			printDatabase (stdout, &database);
			break;
		case 'q':
			canDealloc (device);
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
		.ide = message->ide,
		.rtr = false,
		.dlc = message->dlc
	};
	uint64_t* payload = (uint64_t*) frame.data;

	printf ("- Message: (");
	fprintCanId (stdout, message->id, message->ide, false);
	printf (") -\n");

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

		fprintf (stream, "%s - ID: ", message->name);
		fprintCanId (stream, message->id, message->ide, false);
		fprintf (stream, "\n");

		for (size_t signalIndex = 0; signalIndex < message->signalCount; ++signalIndex)
		{
			canSignal_t* signal = &message->signals [signalIndex];
			ssize_t globalIndex = canDatabaseGetGlobalIndex (database, messageIndex, signalIndex);

			char buffer [11] = "--";

			float value;
			if (canDatabaseGetFloat (database, globalIndex, &value) == CAN_DATABASE_VALID)
				snprintf (buffer, sizeof (buffer), "%.3f", value);

			fprintf (stream, "%32s | %10s | %8"PRIX64" | %10u | %12u | %12f | %12f | %9u | %6u\n",
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

	fprintf (stream, "- Message %s (", message->name);
	fprintCanId (stream, message->id, message->ide, false);
	fprintf (stream, ") -\n");
	for (size_t index = 0; index < message->signalCount; ++index)
		fprintf (stream, "%s: %f\n", message->signals [index].name, signalValues [index]);
}