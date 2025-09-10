// Header
#include "can_database.h"

// Includes -------------------------------------------------------------------------------------------------------------------

// Includes
#include "can_dbc.h"
#include "debug.h"
#include "error_codes.h"

// C Standard Library
#include <errno.h>
#include <string.h>

/// @brief The amount of time a message's data is considered valid before timing out.
const struct timeval CAN_MESSAGE_TIMEOUT =
{
	.tv_sec = 1,
	.tv_usec = 0
};

// Functions ------------------------------------------------------------------------------------------------------------------

void* canDatabaseRxThreadEntrypoint (void* arg);

void canDatabaseCheckTimeouts (canDatabase_t* database);

int canDatabaseInit (canDatabase_t* database, canDevice_t* device, const char* dbcPath)
{
	database->device = device;

	// Parse the DBC file.
	database->messageCount	= CAN_DATABASE_MESSAGE_COUNT_MAX;
	database->signalCount	= CAN_DATABASE_SIGNAL_COUNT_MAX;

	if (dbcFileParse (dbcPath, database->messages, &database->messageCount, database->signals, &database->signalCount) != 0)
		return errno;

	// Set the RX timeout (required for RX thread)
	canSetTimeout (device, 100);

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

void* canDatabaseRxThreadEntrypoint (void* arg)
{
	canDatabase_t* database = (canDatabase_t*) arg;

	while (true)
	{
		// Try to read a CAN frame (will timeout so we can check message timeouts).
		canFrame_t frame;
		int code = canReceive (database->device, &frame);

		// Check if any messages have timed out.
		canDatabaseCheckTimeouts (database);

		// If no frame was received, try reading again.
		if (code != 0)
			continue;

		// Try to identify the message, if unrecognized ignore.
		canMessage_t* message = NULL;
		struct timeval* deadline = NULL;
		for (size_t index = 0; index < database->messageCount; ++index)
		{
			if (frame.id == database->messages [index].id)
			{
				message = database->messages + index;
				deadline = database->messageDeadlines + index;
				break;
			}
		}
		if (message == NULL)
			continue;

		// Postpone the message's timeout deadline.
		struct timeval timeCurrent;
		gettimeofday (&timeCurrent, NULL);
		timeradd (&timeCurrent, &CAN_MESSAGE_TIMEOUT, deadline);

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

ssize_t canDatabaseFindSignal (canDatabase_t* database, const char* name)
{
	for (ssize_t index = 0; index < database->signalCount; ++index)
		if (strcmp (name, database->signals [index].name) == 0)
			return index;

	ERROR_PRINTF ("Could not find signal '%s' in CAN database.\n", name);
	errno = ERRNO_CAN_DATABASE_SIGNAL_MISSING;
	return -1;
}

canDatabaseSignalState_t canDatabaseGetUint32 (canDatabase_t* database, ssize_t index, uint32_t* value)
{
	if (index < 0)
		return CAN_DATABASE_MISSING;

	if (!database->signalsValid [index])
		return CAN_DATABASE_TIMEOUT;

	*value = (uint32_t) database->signalValues [index];
	return CAN_DATABASE_VALID;
}

canDatabaseSignalState_t canDatabaseGetInt32 (canDatabase_t* database, ssize_t index, int32_t* value)
{
	if (index < 0)
		return CAN_DATABASE_MISSING;

	if (!database->signalsValid [index])
		return CAN_DATABASE_TIMEOUT;

	*value = (int32_t) database->signalValues [index];
	return CAN_DATABASE_VALID;
}

canDatabaseSignalState_t canDatabaseGetFloat (canDatabase_t* database, ssize_t index, float* value)
{
	if (index < 0)
		return CAN_DATABASE_MISSING;

	if (!database->signalsValid [index])
		return CAN_DATABASE_TIMEOUT;

	*value = database->signalValues [index];
	return CAN_DATABASE_VALID;
}

canDatabaseSignalState_t canDatabaseGetBool (canDatabase_t* database, ssize_t index, bool* value)
{
	if (index < 0)
		return CAN_DATABASE_MISSING;

	if (!database->signalsValid [index])
		return CAN_DATABASE_TIMEOUT;

	// C-style bool definition. If value != 0, then true.
	*value = database->signalValues [index] >= FLT_EPSILON || database->signalValues [index] <= -FLT_EPSILON;
	return CAN_DATABASE_VALID;
}

void canDatabaseCheckTimeouts (canDatabase_t* database)
{
	struct timeval timeCurrent;
	gettimeofday (&timeCurrent, NULL);

	// TODO(Barach): This should use a stack to only check valid messages and in the order they are expected to expire.
	for (uint16_t messageIndex = 0; messageIndex < database->messageCount; ++messageIndex)
	{
		// TODO(Barach): This indexing is terrible.
		canMessage_t* message = database->messages + messageIndex;
		size_t signalOffset = message->signals - database->signals;

		// If the message is already invalid, skip the timeout check.
		// TODO(Barach): This isn't foolproof. Should move the validity into the message itself.
		if (message->signalCount == 0 || !database->signalsValid [signalOffset])
			continue;

		// If the deadline has expired, invalidate all the signals.
		if (timercmp (&timeCurrent, database->messageDeadlines + messageIndex, >))
			for (uint16_t signalIndex = 0; signalIndex < message->signalCount; ++signalIndex)
				database->signalsValid [signalOffset + signalIndex] = false;
	}
}