// Header
#include "can_database.h"

// Includes -------------------------------------------------------------------------------------------------------------------

// Includes
#include "can_dbc.h"
#include "debug.h"
#include "error_codes.h"

// C Standard Library
#include <stdio.h>
#include <errno.h>
#include <string.h>

/// @brief The amount of time a message's data is considered valid before timing out.
const struct timeval CAN_MESSAGE_TIMEOUT =
{
	.tv_sec = 2,
	.tv_usec = 0
};

// Macros ---------------------------------------------------------------------------------------------------------------------

#define signalToMessageIndex(database, signal) ((signal)->message - (database)->messages)
#define messageToSignalOffset(database, message) ((message)->signals - (database)->signals)

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

	// Invalidate all the messages.
	for (size_t index = 0; index < database->messageCount; ++index)
		database->messagesValid [index] = false;

	return 0;
}

ssize_t canDatabaseFindSignal (canDatabase_t* database, const char* name)
{
	for (size_t index = 0; index < database->signalCount; ++index)
		if (strcmp (name, database->signals [index].name) == 0)
			return (ssize_t) index;

	ERROR_PRINTF ("Could not find signal '%s' in CAN database.\n", name);
	errno = ERRNO_CAN_DATABASE_SIGNAL_MISSING;
	return -1;
}

canMessage_t* canDatabaseFindMessage (canDatabase_t* database, const char* name)
{
	for (size_t index = 0; index < database->messageCount; ++index)
		if (strcmp (database->messages [index].name, name) == 0)
			return &database->messages [index];

	errno = ERRNO_CAN_DATABASE_MESSAGE_MISSING;
	return NULL; 
}

canDatabaseSignalState_t canDatabaseGetUint32 (canDatabase_t* database, ssize_t index, uint32_t* value)
{
	if (index < 0)
		return CAN_DATABASE_MISSING;

	size_t messageIndex = signalToMessageIndex (database, &database->signals [index]);
	if (!database->messagesValid [messageIndex])
		return CAN_DATABASE_TIMEOUT;

	*value = (uint32_t) database->signalValues [index];
	return CAN_DATABASE_VALID;
}

canDatabaseSignalState_t canDatabaseGetInt32 (canDatabase_t* database, ssize_t index, int32_t* value)
{
	if (index < 0)
		return CAN_DATABASE_MISSING;

	size_t messageIndex = signalToMessageIndex (database, &database->signals [index]);
	if (!database->messagesValid [messageIndex])
		return CAN_DATABASE_TIMEOUT;

	*value = (int32_t) database->signalValues [index];
	return CAN_DATABASE_VALID;
}

canDatabaseSignalState_t canDatabaseGetFloat (canDatabase_t* database, ssize_t index, float* value)
{
	if (index < 0) {
		return CAN_DATABASE_MISSING;
	}

	size_t messageIndex = signalToMessageIndex (database, &database->signals [index]);
	if (!database->messagesValid [messageIndex])
		return CAN_DATABASE_TIMEOUT;

	*value = database->signalValues [index];
	return CAN_DATABASE_VALID;
}

canDatabaseSignalState_t canDatabaseGetBool (canDatabase_t* database, ssize_t index, bool* value)
{
	if (index < 0)
		return CAN_DATABASE_MISSING;

	size_t messageIndex = signalToMessageIndex (database, &database->signals [index]);
	if (!database->messagesValid [messageIndex])
		return CAN_DATABASE_TIMEOUT;

	// C-style bool definition. If value != 0, then true.
	*value = database->signalValues [index] >= FLT_EPSILON || database->signalValues [index] <= -FLT_EPSILON;
	return CAN_DATABASE_VALID;
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

		ssize_t messageIndex = -1;
		for (size_t index = 0; index < database->messageCount; ++index)
		{
			if (frame.id == database->messages [index].id)
			{
				messageIndex = index;
				break;
			}
		}
		if (messageIndex < 0)
			continue;

		canMessage_t* message = &database->messages [messageIndex];

		// Decode the message

		uint64_t payload = *((uint64_t*) frame.data);
		size_t signalOffset = messageToSignalOffset (database, message);

		for (size_t index = 0; index < message->signalCount; ++index)
		{
			canSignal_t* signal = message->signals + index;
			database->signalValues [signalOffset + index] = signalDecode (signal, payload);
		}

		// Postpone the message's timeout deadline and validate the message.

		struct timeval timeCurrent;
		gettimeofday (&timeCurrent, NULL);

		timeradd (&timeCurrent, &CAN_MESSAGE_TIMEOUT, &database->messageDeadlines [messageIndex]);
		database->messagesValid [messageIndex] = true;
	}
}

void canDatabaseCheckTimeouts (canDatabase_t* database)
{
	struct timeval timeCurrent;
	gettimeofday (&timeCurrent, NULL);

	// TODO(Barach): This should use a stack to only check valid messages and in the order they are expected to expire.
	for (uint16_t messageIndex = 0; messageIndex < database->messageCount; ++messageIndex)
	{
		canMessage_t* message = database->messages + messageIndex;

		// If the message is already invalid, skip the timeout check.
		if (!database->messagesValid [messageIndex])
			continue;

		// If the deadline has expired, invalidate the message.
		if (timercmp (&timeCurrent, database->messageDeadlines + messageIndex, >))
			database->messagesValid [messageIndex] = false;
	}
}