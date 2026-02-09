// Header
#include "can_dbc.h"

// Includes
#include "debug.h"
#include "error_codes.h"
#include "list.h"

// C Standard Library
#include <ctype.h>
#include <errno.h>
#include <stdio.h>
#include <string.h>

// Constants ------------------------------------------------------------------------------------------------------------------

/// @brief Bitmask for the IDE bit in the CAN ID field. This bit indicates whether the ID is a standard identifer or an
/// extended identifer. This mask is 31st bit set to 1.
#define ID_IDE_BIT_MASK 0x80000000

/// @brief Bitmask for the CAN ID inside the CAN ID field. This masks out the upper 3 bits which are not part of the ID. This
/// mask is 1s for the first 29 bits.
#define ID_ID_BIT_MASK 0x1FFFFFFF

#define KEYWORD_NETWORK_NODE	"BU_:"			// CAN ECU
#define KEYWORD_MESSAGE			"BO_"			// CAN message
#define KEYWORD_SIGNAL			"SG_"			// CAN signal
#define KEYWORD_ENV_VARIABLE	"EV_"			// Environment variable, ignored
#define KEYWORD_SIG_GROUP		"SIG_GROUP_"	// Signal group, ignored
#define KEYWORD_VAL_TABLE		"VAL_TABLE_"	// Value table, ignored
#define KEYWORD_VERSION			"VERSION"		// Version Number, ignored
#define KEYWORD_BIT_TIMING		"BS_:"			// Network baudrate, ignored
#define KEYWORD_COMMENT			"CM_"			// Comments, ignored for now
#define KEYWORD_NS				"NS_"			// Purpose unknown, ignored for now

// Datatypes ------------------------------------------------------------------------------------------------------------------

listDefine (canMessage_t);
listDefine (canSignal_t);

// Functions ------------------------------------------------------------------------------------------------------------------

/**
 * @brief Function for splitting a string into 2 substrings. This function behaves very similarly to @c strtok , with slightly
 * modified behavior. The string @c str is split at the first occurence of a character in @c delims .
 * @param str The string to split. Modified to end at the first delimiter.
 * @param delims The delimiter characters to search for.
 * @param consecutuveDelims Indicates how consecutive delimiter characters should be handled. If true, the 2nd string will
 * start at the first non-delimiter character. If false, the 2nd string will start immediately after the first delimiter found.
 * @return The second string that was split, that is, the string occurring after the delimiter(s). @c NULL if there was no
 * delimiter or no string after the delimiter(s).
 */
static char* stringSplit (char* str, char* delims, bool consecutiveDelims)
{
	size_t delimCount = strlen (delims);

	bool delimFound = false;

	while (true)
	{
		if (*str == '\0')
			return NULL;

		bool delim = false;
		for (size_t index = 0; index < delimCount; ++index)
			if (*str == delims [index])
			{
				delim = true;
				break;
			}

		if (delim)
			*str = '\0';

		if (delimFound && !delim)
			return str;

		if (delim && !delimFound)
			delimFound = true;

		++str;

		if (delimFound && !consecutiveDelims)
			return str;
	}
}

/// @brief Handles a missing field in a DBC file.
static inline int handleMissing (const char* dbcFile, size_t lineNumber, const char* fieldName)
{
	errno = EINVAL;
	debugPrintf ("Missing value for %s in DBC file '%s', line %lu.\n", fieldName, dbcFile, (long unsigned) lineNumber);
	return errno;
}

/// @brief Handles an invalid field value in a DBC file.
static inline int handleInvalid (const char* dbcFile, size_t lineNumber, const char* fieldName, const char* fieldValue)
{
	errno = EINVAL;
	debugPrintf ("Invalid value for %s, '%s', in DBC file '%s', line %lu.\n", fieldName, fieldValue, dbcFile, (long unsigned) lineNumber);
	return errno;
}

/**
 * @brief Parses a message line. Message lines should take the following format:
 *   BO_ <id> <name>: <DLC> <Network Node>
 * @param line The line to parse. Should omit whitespace and the keyword.
 * @param dbcFile The path of the DBC file.
 * @param lineNumber The number of this line.
 * @return The index of the message that was parsed, if successful, -1 otherwise.
 */
ssize_t parseMessage (list_t (canMessage_t)* messages, char* line, const char* dbcFile, size_t lineNumber)
{
	canMessage_t* message = listAppendUninit (canMessage_t) (messages);
	if (message == NULL)
		return -1;

	char* id = line;
	if (id == NULL)
	{
		handleMissing (dbcFile, lineNumber, "message ID");
		return -1;
	}

	char* name = stringSplit (id, ": ", true);
	if (name == NULL)
	{
		handleMissing (dbcFile, lineNumber, "message name");
		return -1;
	}

	// Parse the ID
	char* endPtr;
	unsigned long result = strtoul (id, &endPtr, 0);
	if (endPtr == id)
	{
		handleInvalid (dbcFile, lineNumber, "message ID", id);
		return -1;
	}
	message->id = (uint32_t) result & ID_ID_BIT_MASK;
	message->ide = (result & ID_IDE_BIT_MASK) == ID_IDE_BIT_MASK;

	char* dlc = stringSplit (name, ": ", true);
	if (dlc == NULL)
	{
		handleMissing (dbcFile, lineNumber, "message DLC");
		return -1;
	}

	// Copy the name
	message->name = strdup (name);
	if (message->name == NULL)
		return -1;

	char* networkNode = stringSplit (dlc, " ", false);
	if (networkNode == NULL)
	{
		handleMissing (dbcFile, lineNumber, "message network node");
		return -1;
	}

	// Parse the DLC
	result = strtoul (dlc, &endPtr, 0);
	if (endPtr == dlc || result > 8)
	{
		handleInvalid (dbcFile, lineNumber, "message DLC", dlc);
		return -1;
	}
	message->dlc = result;

	message->signalCount = 0;

	return listSize (canMessage_t) (messages) - 1;
}

/**
 * @brief Parses a signal line. Signal lines should take the following format:
     SG_ <Name> : <Bit position>|<Bit length>@<Endianness><Signedness> (<Scale factor>,<Offset>) [<Min>|<Max>] "<Unit>" <Network Node>
 * @param signals The list of signals to append to.
 * @param message The message this signal belongs to.
 * @param line The line to parse. Should omit whitespace and the keyword.
 * @param dbcFile The path of the DBC file.
 * @param lineNumber The number of this line.
 * @return 0 if successful, the error code otherwise.
 */
int parseSignal (list_t (canSignal_t)* signals, canMessage_t* message, char* line, const char* dbcFile, size_t lineNumber)
{
	canSignal_t* signal = listAppendUninit (canSignal_t) (signals);
	if (signal == NULL)
		return errno;

	char* name = line;
	if (name == NULL)
		return handleMissing (dbcFile, lineNumber, "signal name");

	char* bitPosition = stringSplit (name, " :@|,()[]", true);
	if (bitPosition == NULL)
		return handleMissing (dbcFile, lineNumber, "signal bit position");

	// Copy the signal name
	signal->name = strdup (name);
	if (signal->name == NULL)
		return errno;

	char* bitLength = stringSplit (bitPosition, " :@|,()[]", true);
	if (bitLength == NULL)
		return handleMissing (dbcFile, lineNumber, "signal bit length");

	// Parse the bit position
	char* endPtr;
	unsigned long result = strtoul (bitPosition, &endPtr, 0);
	if (endPtr == bitPosition || result >= 64)
		return handleInvalid (dbcFile, lineNumber, "signal bit position", bitPosition);
	signal->bitPosition = result;

	char* endiannessSignedness = stringSplit (bitLength, " :@|,()[]", true);
	if (endiannessSignedness == NULL)
		return handleMissing (dbcFile, lineNumber, "signal endianness / signedness");

	// Parse the bit length
	result = strtoul (bitLength, &endPtr, 0);
	if (endPtr == bitLength || result >= 64)
		return handleInvalid (dbcFile, lineNumber, "signal bit length", bitLength);
	signal->bitLength = result;

	char* scaleFactor = stringSplit (endiannessSignedness, " :@|,()[]", true);
	if (scaleFactor == NULL)
		return handleMissing (dbcFile, lineNumber, "signal scale factor");

	// Parse the endinness
	if (endiannessSignedness [0] != '0' && endiannessSignedness [0] != '1')
		return handleInvalid (dbcFile, lineNumber, "signal endianness", endiannessSignedness);
	signal->endianness = endiannessSignedness [0] == '1';

	// Parse the signedness
	if (endiannessSignedness [1] != '+' && endiannessSignedness [1] != '-')
		return handleInvalid (dbcFile, lineNumber, "signal signedness", endiannessSignedness);
	signal->signedness = endiannessSignedness [0] == '-';

	char* offset = stringSplit (scaleFactor, " :@|,()[]", true);
	if (offset == NULL)
		return handleMissing (dbcFile, lineNumber, "signal offset");

	// Parse the scale factor
	float resultf = strtof (scaleFactor, &endPtr);
	if (endPtr == scaleFactor)
		return handleInvalid (dbcFile, lineNumber, "signal scale factor", scaleFactor);
	signal->scaleFactor = resultf;

	char* min = stringSplit (offset, " :@|,()[]", true);
	if (min == NULL)
		return handleMissing (dbcFile, lineNumber, "signal minimum");

	// Parse the signal offset
	resultf = strtof (offset, &endPtr);
	if (endPtr == offset)
		return handleInvalid (dbcFile, lineNumber, "signal offset", offset);
	signal->offset = resultf;

	char* max = stringSplit (min, " :@|,()[]", true);
	if (max == NULL)
		return handleMissing (dbcFile, lineNumber, "signal maximum");

	// Parse the signal minimum
	resultf = strtof (min, &endPtr);
	if (endPtr == min)
		return handleInvalid (dbcFile, lineNumber, "signal minimum", min);

	char* unit = stringSplit (max, " :@|,()[]", true);
	if (unit == NULL)
		return handleMissing (dbcFile, lineNumber, "signal unit");

	// Parse the signal maximum
	resultf = strtof (max, &endPtr);
	if (endPtr == max)
		return handleInvalid (dbcFile, lineNumber, "signal maximum", min);

	unit = stringSplit (unit, "\"", false);
	if (unit == NULL)
		return handleMissing (dbcFile, lineNumber, "signal unit");

	char* networkNode = stringSplit (unit, "\"", false);
	if (networkNode == NULL)
		return handleMissing (dbcFile, lineNumber, "signal network node");

	// Copy the signal's unit
	signal->unit = strdup (unit);
	if (signal->unit == NULL)
		return errno;

	// Handle motorola format
	if (!signal->endianness)
	{
		if (signal->bitLength >= 8)
			signal->bitPosition -= 7;
		else
			signal->bitPosition -= signal->bitLength - 1;
	}

	// Populate bitmask
	signal->bitmask = ((uint64_t) 1 << signal->bitLength) - 1;

	// Increment the message's signal count
	++message->signalCount;

	return 0;
}

/**
 * @brief Loads all the messages and signals of a DBC file into a pair of lists. Note that due to potential reallocation, this
 * function does not initialize any references. In order for the contents of the lists to be externally usable, they must be
 * linked via the @c linkDbc function.
 * @param dbcFile The DBC file to load.
 * @return 0 if successful, the error code otherwise.
 */
static int loadDbc (const char* dbcFile, list_t (canMessage_t)* messages, list_t (canSignal_t)* signals)
{
	FILE* file = fopen (dbcFile, "r");
	if (file == NULL)
		return errno;

	size_t lineNumber = 0;

	ssize_t message = -1;
	while (true)
	{
		char buffer [4096];
		char* line = fgets (buffer, sizeof (buffer), file);
		if (line == NULL)
		{
			if (feof (file))
				break;

			errno = ERRNO_CAN_DBC_LINE_LENGTH;
			return errno;
		}
		++lineNumber;

		// Skip whitespace
		while (isspace (line [0]))
			++line;
		if (line [0] == '\0')
			continue;

		// Trim newline
		line [strcspn (line, "\n")] = '\0';

		char* keyword = line;
		line = stringSplit (keyword, " ", true);

		if (strcmp (keyword, KEYWORD_MESSAGE) == 0)
		{
			message = parseMessage (messages, line, dbcFile, lineNumber);
			if (message < 0)
				return errno;
		}
		else if (strcmp (keyword, KEYWORD_SIGNAL) == 0)
		{
			if (message < 0)
			{
				errno = ERRNO_CAN_DBC_MESSAGE_MISSING;
				debugPrintf ("Signal detected before first message in DBC file '%s', line %lu.\n", dbcFile,
					(long unsigned) lineNumber);
				return errno;
			}

			if (parseSignal (signals, listGetReference (canMessage_t) (messages, message), line, dbcFile, lineNumber) != 0)
				return errno;
		}
		else
		{
			debugPrintf ("Warning, ignoring unknown keyword '%s' in DBC file.\n", keyword);
		}
	}

	return 0;
}

/**
 * @brief Links all the internal references of a pair of messages and signals. Note that the arrays cannot be reallocated after
 * this, therefore they can no longer be used as lists.
 * @param messages The array of messages to link.
 * @param messageCount The number of elements in @c messages .
 * @param signals The array of signals to link.
 */
static void linkDbc (canMessage_t* messages, size_t messageCount, canSignal_t* signals)
{
	size_t signalIndex = 0;
	for (canMessage_t* message = messages; message < messages + messageCount; ++message)
	{
		message->signals = signals + signalIndex;
		signalIndex += message->signalCount;

		for (canSignal_t* signal = message->signals; signal < message->signals + message->signalCount; ++signal)
			signal->message = message;
	}
}

int canDbcLoad (char* dbcFile, canMessage_t** messages, size_t* messageCount,
	canSignal_t** signals, size_t* signalCount)
{
	// Call the array version of this function with all arrays of size 1.
	size_t dbcMessageIndices [1];
	return canDbcsLoad (&dbcFile, 1, messages, messageCount, signals, signalCount, dbcMessageIndices);
}

int canDbcsLoad (char** dbcFiles, size_t dbcCount, canMessage_t** messages, size_t* messageCount,
	canSignal_t** signals, size_t* signalCount, size_t* dbcMessageIndices)
{
	// Create lists for the messages and signals

	list_t (canMessage_t) messageList;
	if (listInit (canMessage_t) (&messageList, 512) != 0)
		return errno;

	list_t (canSignal_t) signalList;
	if (listInit (canSignal_t) (&signalList, 512) != 0)
		return errno;

	// Load each DBC individually

	for (size_t index = 0; index < dbcCount; ++index)
	{
		dbcMessageIndices [index] = listSize (canMessage_t) (&messageList);
		if (loadDbc (dbcFiles [index], &messageList, &signalList) != 0)
			return errno;
	}

	// Convert the lists into arrays

	*messages = listDestroy (canMessage_t) (&messageList, messageCount);
	if (messages == NULL)
		return errno;

	*signals = listDestroy (canSignal_t) (&signalList, signalCount);
	if (signals == NULL)
		return errno;

	// Link the DBC
	linkDbc (*messages, *messageCount, *signals);

	return 0;
}

void canDbcsDealloc (canMessage_t* messages, size_t messageCount, canSignal_t* signals)
{
	for (canMessage_t* message = messages; message < messages + messageCount; ++message)
	{
		for (canSignal_t* signal = message->signals; signal < message->signals + message->signalCount; ++signal)
		{
			free (signal->name);
			free (signal->unit);
		}

		free (message->name);
	}

	free (messages);
	free (signals);
}