#ifndef CAN_DATABASE_H
#define CAN_DATABASE_H

// CAN Database ---------------------------------------------------------------------------------------------------------------
//
// Author: Cole Barach
// Date Created: 2023.07.21
//
// Description: An database-oriented interface for CAN bus communication. Received messages are parsed using a DBC file and
//   stored in a relational database for random access.

// Includes -------------------------------------------------------------------------------------------------------------------

// Includes
#include "can_device/can_device.h"
#include "can_signals.h"
#include "time_port.h"

// POSIX Libraries
#include <pthread.h>

// C Standard Library
#include <float.h>

// Constants ------------------------------------------------------------------------------------------------------------------

/// @brief The maximum number of messages in the database.
#define CAN_DATABASE_MESSAGE_COUNT_MAX	128

/// @brief The maximum number of signals in the database.
#define CAN_DATABASE_SIGNAL_COUNT_MAX	1024

// Datatypes ------------------------------------------------------------------------------------------------------------------

/// @brief Enum indicating the possible states of a signal in a CAN database.
typedef enum
{
	/// @brief Indicates the signal is present in the database and up to date.
	CAN_DATABASE_VALID = 0,

	/// @brief Indicates the signal is not present in the database.
	CAN_DATABASE_MISSING = 1,

	/// @brief Indicates the signal is present in the database, but out of date.
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

	/// @brief The array indicating whether the value of each CAN message is valid.
	bool messagesValid [CAN_DATABASE_MESSAGE_COUNT_MAX];

	/// @brief Array of deadlines for the values each CAN message's signals.
	struct timeval messageDeadlines [CAN_DATABASE_MESSAGE_COUNT_MAX];
} canDatabase_t;

// Functions ------------------------------------------------------------------------------------------------------------------

/**
 * @brief Initializes a CAN database bound to the specified device using the specified database file.
 * @param database The database to initialize.
 * @param device The CAN device to bind to.
 * @param dbcPath The database file to import from.
 * @return 0 if successful, the error code otherwise.
 */
int canDatabaseInit (canDatabase_t* database, canDevice_t* device, const char* dbcPath);

/**
 * @brief Converts a local signal index (index within a message) to a global signal index (index within the database).
 * @param database Pointer to the database the signal belongs to.
 * @param messageIndex The index of the message the signal belongs to.
 * @param signalIndex The (local) index of the signal within the message.
 * @return The global index of the signal.
 */
#define canDatabaseGetGlobalIndex(database, messageIndex, signalIndex)	\
	((signalIndex) + ((database)->messages [messageIndex]).signals - (database)->signals)

/**
 * @brief Finds the global index of a signal based off its name.
 * @param database The database to search from.
 * @param name The name of the signal to find.
 * @return The index if successful, -1 otherwise. Note errno is set on error.
 */
ssize_t canDatabaseFindSignal (canDatabase_t* database, const char* name);

/**
 * @brief Finds the message based off its name.
 * @param database The database to search from.
 * @param name The name of the message to find.
 * @return A pointer of the message if successful, @c NULL otherwise. Note errno is set on error.
 */
canMessage_t* canDatabaseFindMessage (canDatabase_t* database, const char* name);

/**
 * @brief Gets the value of a signal in a CAN database, as a @c uint32_t .
 * @param database The database to get from.
 * @param index The global index of the signal to get. Note that a local index can be transformed using the
 *     @c canDatabaseGetGlobalIndex function.
 * @param value Buffer to write the data into.
 * @return The state of the signal. Note that @c value is only written if the return is @c CAN_DATABASE_VALID .
 */
canDatabaseSignalState_t canDatabaseGetUint32 (canDatabase_t* database, ssize_t index, uint32_t* value);

/**
 * @brief Gets the value of a signal in a CAN database, as an @c int32_t .
 * @param database The database to get from.
 * @param index The global index of the signal to get. Note that a local index can be transformed using the
 *     @c canDatabaseGetGlobalIndex function.
 * @param value Buffer to write the data into.
 * @return The state of the signal. Note that @c value is only written if the return is @c CAN_DATABASE_VALID .
 */
canDatabaseSignalState_t canDatabaseGetInt32 (canDatabase_t* database, ssize_t index, int32_t* value);

/**
 * @brief Gets the value of a signal in a CAN database, as a @c float .
 * @param database The database to get from.
 * @param index The global index of the signal to get. Note that a local index can be transformed using the
 *     @c canDatabaseGetGlobalIndex function.
 * @param value Buffer to write the data into.
 * @return The state of the signal. Note that @c value is only written if the return is @c CAN_DATABASE_VALID .
 */
canDatabaseSignalState_t canDatabaseGetFloat (canDatabase_t* database, ssize_t index, float* value);

/**
 * @brief Gets a reference to a CAN signal, from its global index.
 * @param database The database to get from.
 * @param index The global index of the signal.
 * @return A refernce to the CAN signal if successful, @c NULL otherwise.
 */
static inline canSignal_t* canDatabaseGetSignal (canDatabase_t* database, ssize_t index);

/**
 * @brief Gets the value of a signal in a CAN database, as a @c bool .
 * @param database The database to get from.
 * @param index The global index of the signal to get. Note that a local index can be transformed using the
 *     @c canDatabaseGetGlobalIndex function.
 * @param value Buffer to write the data into.
 * @return The state of the signal. Note that @c value is only written if the return is @c CAN_DATABASE_VALID .
 */
canDatabaseSignalState_t canDatabaseGetBool (canDatabase_t* database, ssize_t index, bool* value);

#endif // CAN_DATABASE_H