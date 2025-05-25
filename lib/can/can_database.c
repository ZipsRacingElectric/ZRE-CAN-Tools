// Header
#include "can_database.h"

// Includes -------------------------------------------------------------------------------------------------------------------

// Includes
#include "can_dbc.h"
#include "error_codes.h"

// C Standard Library
#include <errno.h>
#include <string.h>

// Debugging ------------------------------------------------------------------------------------------------------------------

#if CAN_DATABASE_DEBUG
	#include <stdio.h>
	#define DEBUG_PRINTF(...) fprintf (stderr, __VA_ARGS__)
#else
	#define DEBUG_PRINTF(...) while (false)
#endif // CAN_DATABASE_DEBUG

#if CAN_DATABASE_RX_PRINT
	#include <stdio.h>
	#define RX_PRINTF(...) fprintf (stderr, __VA_ARGS__)
#else
	#define RX_PRINTF(...) while (false)
#endif // CAN_DATABASE_RX_PRINT

// Functions ------------------------------------------------------------------------------------------------------------------

void* canDatabaseRxThreadEntrypoint (void* arg);

int canDatabaseInit (canDatabase_t* database, const char* deviceName, const char* dbcPath)
{
	// Parse the DBC file.
	database->messageCount	= CAN_DATABASE_MESSAGE_COUNT_MAX;
	database->signalCount	= CAN_DATABASE_SIGNAL_COUNT_MAX;

	if (dbcFileParse (dbcPath, database->messages, &database->messageCount, database->signals, &database->signalCount) != 0)
		return errno;

	// Initialize the TX socket.
	if (canSocketInit (&database->txSocket, deviceName) != 0)
		return errno;

	// Initialize the RX socket.
	if (canSocketInit (&database->rxSocket, deviceName) != 0)
		return errno;

	// Start the RX thread.
	int code = pthread_create (&database->rxThread, NULL, canDatabaseRxThreadEntrypoint, database);
	if (code != 0)
	{
		errno = code;
		return errno;
	}

	// Invalidate all the signals.
	for (size_t index = 0; index < database->signalCount; ++index)
		database->signalsValid [index] = false;

	return 0;
}

void canDatabasePrint (FILE* stream, canDatabase_t* database)
{
	size_t signalOffset = 0;

	fprintf (stream, "%32s | %10s | %8s | %10s | %12s | %12s | %12s | %9s | %6s\n\n", "Signal Name", "Value", "Bit Mask",
		"Bit Length", "Bit Position", "Scale Factor", "Offset", "Is Signed", "Endian");

	for (size_t messageIndex = 0; messageIndex < database->messageCount; ++messageIndex)
	{
		canMessage_t* message = database->messages + messageIndex;

		fprintf (stream, "%s - ID: %3X\n", message->name, message->id);

		for (size_t signalIndex = 0; signalIndex < message->signalCount; ++signalIndex)
		{
			canSignal_t* signal = message->signals + signalIndex;
			bool valid = database->signalsValid [signalOffset + signalIndex];
			float value = database->signalValues [signalOffset + signalIndex];

			if (valid)
				fprintf (stream, "%32s | %10.3f | %8lX | %10i | %12i | %12f | %12f | %9u | %6u\n", signal->name, value,
					signal->bitmask, signal->bitLength, signal->bitPosition, signal->scaleFactor, signal->offset,
					signal->signedness, signal->endianness);
			else
				fprintf (stream, "%32s | %10s | %8lX | %10i | %12i | %12f | %12f | %9u | %6u\n", signal->name, "--",
					signal->bitmask, signal->bitLength, signal->bitPosition, signal->scaleFactor, signal->offset,
					signal->signedness, signal->endianness);
		}

		fprintf (stream, "\n");

		signalOffset += message->signalCount;
	}
}

void canDatabaseMessagesPrint (FILE* stream, canDatabase_t* database)
{
	for (size_t index = 0; index < database->messageCount; ++index)
		fprintf (stream, "%s\n", database->messages [index].name);
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

void canDatabaseMessageValuePrint (FILE* stream, canDatabase_t* database, size_t index)
{
	canMessage_t* message = database->messages + index;

	float* signalValues = database->signalValues + (size_t) (message->signals - database->signals);

	fprintf (stream, "- Message %s (0x%X) -\n", message->name, message->id);
	for (size_t index = 0; index < message->signalCount; ++index)
		fprintf (stream, "%s: %f\n", message->signals [index].name, signalValues [index]);
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
		if (canSocketReceive (&database->rxSocket, &frame) != 0)
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

int canDatabaseFindSignal (canDatabase_t* database, const char* name, size_t* index)
{
	for (uint16_t signalIndex = 0; signalIndex < database->signalCount; ++signalIndex)
	{
		if (strcmp (name, database->signals [signalIndex].name) == 0)
		{
			*index = signalIndex;
			return 0;
		}
	}

	errno = ERRNO_CAN_DATABASE_SIGNAL_MISSING;
	return errno;
}