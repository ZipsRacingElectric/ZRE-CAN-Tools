// Header
#include "can_database.h"

// Includes -------------------------------------------------------------------------------------------------------------------

// Includes
#include "can_dbc.h"

// C Standard Library
#include <string.h>

// Debugging ------------------------------------------------------------------------------------------------------------------

#if CAN_DATABASE_DEBUG
	#include <stdio.h>
	#define DEBUG_PRINTF(...) printf(__VA_ARGS__)
#else
	#define DEBUG_PRINTF(...) while (false)
#endif // CAN_DATABASE_DEBUG

#if CAN_DATABASE_RX_PRINT
	#include <stdio.h>
	#define RX_PRINTF(...) printf(__VA_ARGS__)
#else
	#define RX_PRINTF(...) while (false)
#endif // CAN_DATABASE_RX_PRINT

// Functions ------------------------------------------------------------------------------------------------------------------

void* canDatabaseRxThreadEntrypoint (void* arg);

bool canDatabaseInit (canDatabase_t* database, const char* deviceName, const char* dbcPath)
{
	// Parse the DBC file.
	database->messageCount	= CAN_DATABASE_MESSAGE_COUNT_MAX;
	database->signalCount	= CAN_DATABASE_SIGNAL_COUNT_MAX;
	if (!dbcFileParse (dbcPath, database->messages, &database->messageCount, database->signals, &database->signalCount))
		return false;

	// Initialize the TX socket.
	if (!canSocketInit (&database->txSocket, deviceName))
		return false;

	// Initialize the RX socket.
	if (!canSocketInit (&database->rxSocket, deviceName))
		return false;

	// Start the RX thread.
	if (pthread_create (&database->rxThread, NULL, canDatabaseRxThreadEntrypoint, database) != 0)
		return false;

	// Invalidate all the signals.
	for (size_t index = 0; index < database->signalCount; ++index)
		database->signalsValid [index] = false;

	return true;
}

void canDatabasePrint (canDatabase_t* database)
{
	size_t signalOffset = 0;

	for (size_t messageIndex = 0; messageIndex < database->messageCount; ++messageIndex)
	{
		canMessage_t* message = database->messages + messageIndex;

		printf ("- %s (0x%3X) ", message->name, message->id);

		for (size_t charIndex = strlen (message->name) - 8; charIndex < 64; ++charIndex)
			printf ("-");

		printf ("\n");

		for (size_t signalIndex = 0; signalIndex < message->signalCount; ++signalIndex)
		{
			canSignal_t* signal = message->signals + signalIndex;
			bool valid = database->signalsValid [signalOffset + signalIndex];
			float value = database->signalValues [signalOffset + signalIndex];
			
			if (valid)
				printf ("    %-48s = %f\n", signal->name, value);
			else
				printf ("    %-48s = -\n", signal->name);
		}

		printf ("\n");

		signalOffset += message->signalCount;
	}
}

void canDatabaseMessagesPrint (canDatabase_t* database)
{
	for (size_t index = 0; index < database->messageCount; ++index)
		printf ("%s\n", database->messages [index].name);
}

size_t canDatabaseMessageNamePrompt (canDatabase_t* database)
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

void canDatabaseMessageValuePrint (canDatabase_t* database, size_t index)
{
	canMessage_t* message = database->messages + index;

	float* signalValues = database->signalValues + (size_t) (message->signals - database->signals);

	printf ("- Message %s (0x%X) -\n", message->name, message->id);
	for (size_t index = 0; index < message->signalCount; ++index)
		printf ("%s: %f\n", message->signals [index].name, signalValues [index]);
}

struct can_frame canDatabaseMessageValuePrompt (canDatabase_t* database, size_t index)
{
	return messagePrompt (database->messages + index);
}

void* canDatabaseRxThreadEntrypoint (void* arg)
{
	canDatabase_t* database = (canDatabase_t*) arg;

	while (true)
	{
		struct can_frame frame;
		if (!canSocketReceive (&database->rxSocket, &frame))
		{
			DEBUG_PRINTF ("CAN RX thread failed to receive frame.\n");
			continue;
		}

		canMessage_t* message = NULL;
		for (size_t index = 0; index < database->messageCount; ++index)
		{
			if (frame.can_id == database->messages [index].id)
			{
				message = database->messages + index;
				break;
			}
		}
		if (message == NULL)
			continue;

		#if CAN_DATABASE_RX_PRINT
		RX_PRINTF ("Received CAN frame: ");
		framePrint (&frame);
		#endif // CAN_DATABASE_RX_PRINT

		uint64_t payload = *((uint64_t*) frame.data);
		size_t signalOffset = message->signals - database->signals;

		for (size_t index = 0; index < message->signalCount; ++index)
		{
			canSignal_t* signal = message->signals + index;
			database->signalValues [signalOffset + index] = signalDecode (signal, payload);
			database->signalsValid [signalOffset + index] = true;
		}
	}
}