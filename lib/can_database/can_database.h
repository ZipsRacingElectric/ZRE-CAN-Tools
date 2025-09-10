#ifndef CAN_DATABASE_H
#define CAN_DATABASE_H

// CAN Database ---------------------------------------------------------------------------------------------------------------
//
// Author: Cole Barach
// Date Created: 2023.07.21
//
// Description: An database-oriented interface for CAN bus communication. Received messages are parsed using a DBC file and
//   stored in a relational database for random access.

// TODO(Barach): Documentation

// Includes -------------------------------------------------------------------------------------------------------------------

// Includes
#include "can_device/can_device.h"
#include "can_signals.h"
#include "time_port.h"

// POSIX Libraries
#include <pthread.h>

// C Standard Library
#include <stdio.h>
#include <float.h>

// Constants ------------------------------------------------------------------------------------------------------------------

/// @brief The maximum number of messages in the database.
#define CAN_DATABASE_MESSAGE_COUNT_MAX	128

/// @brief The maximum number of signals in the database.
#define CAN_DATABASE_SIGNAL_COUNT_MAX	1024

// Datatypes ------------------------------------------------------------------------------------------------------------------

typedef enum
{
	CAN_DATABASE_VALID = 0,
	CAN_DATABASE_MISSING = 1,
	CAN_DATABASE_TIMEOUT = 2
} canDatabaseSignalState_t;

/// @brief Structure representing a CAN database.
typedef struct
{
	/// @brief The CAN device to receive from.
	canDevice_t* device;

	/// @brief The thread for receiving CAN messages.
	pthread_t rxThread;

	/// @brief The array of CAN messages forming the database.
	canMessage_t messages [CAN_DATABASE_MESSAGE_COUNT_MAX];

	/// @brief The number of used elements in @c messages .
	size_t messageCount;

	/// @brief The array of CAN signals forming the database.
	canSignal_t signals [CAN_DATABASE_SIGNAL_COUNT_MAX];

	/// @brief The number of used elements in @c signals .
	size_t signalCount;

	/// @brief The array of values associated with each CAN signal.
	float signalValues [CAN_DATABASE_SIGNAL_COUNT_MAX];

	/// @brief The array indicating whether the value of each CAN signal is valid.
	bool signalsValid [CAN_DATABASE_SIGNAL_COUNT_MAX];

	/// @brief Array of deadlines for the values each CAN message's signals.
	struct timeval messageDeadlines [CAN_DATABASE_MESSAGE_COUNT_MAX];
} canDatabase_t;

// Function -------------------------------------------------------------------------------------------------------------------

/**
 * @brief Initializes a CAN database bound to the specified device using the specified database file.
 * @param database The database to initialize.
 * @param device The CAN device to bind to.
 * @param dbcPath The database file to import from.
 * @return 0 if successful, the error code otherwise.
 */
int canDatabaseInit (canDatabase_t* database, canDevice_t* device, const char* dbcPath);

/**
 * @brief Finds a signal's index based off name.
 * @param database The database to search from.
 * @param name The name of the signal to find.
 * @param index If successful, written to contain the index of the signal.
 * @return The index if successful, -1 otherwise. Note errno is set on error.
 */
ssize_t canDatabaseFindSignal (canDatabase_t* database, const char* name);

canDatabaseSignalState_t canDatabaseGetUint32 (canDatabase_t* database, ssize_t index, uint32_t* value);

canDatabaseSignalState_t canDatabaseGetInt32 (canDatabase_t* database, ssize_t index, int32_t* value);

canDatabaseSignalState_t canDatabaseGetFloat (canDatabase_t* database, ssize_t index, float* value);

canDatabaseSignalState_t canDatabaseGetBool (canDatabase_t* database, ssize_t index, bool* value);

// Standard I/O ---------------------------------------------------------------------------------------------------------------

/**
 * @brief Prompts the user to select a database message.
 * @param database The database to search from.
 * @return The index of the selected message.
 */
size_t canDatabaseMessageNamePrompt (canDatabase_t* database);

/**
 * @brief Prints all of the data of a database.
 * @param stream The stream to write to.
 * @param database The database to print.
 */
void canDatabasePrint (FILE* stream, canDatabase_t* database);

/**
 * @brief Prints the name of each message in the database.
 * @param stream The stream to print to.
 * @param database The database to search from.
 */
void canDatabaseMessagesPrint (FILE* stream, canDatabase_t* database);

/**
 * @brief Prints the last read values of a CAN message from the database.
 * @param stream The stream to print to.
 * @param database The database to read from.
 * @param index The index of the message to print.
 */
void canDatabaseMessageValuePrint (FILE* stream, canDatabase_t* database, size_t index);

#endif // CAN_DATABASE_H