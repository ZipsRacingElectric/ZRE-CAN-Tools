// Header
#include "can_dbc_v2.h"

// Includes
#include "debug.h"
#include "error_codes.h"

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

// Functions ------------------------------------------------------------------------------------------------------------------

static char* stringSplit (char* str, char* delims, bool consecutuveDelims)
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

		if (delimFound && !delim)
			return str;

		if (delim && !delimFound)
			delimFound = true;

		if (delimFound && !consecutuveDelims)
			return str;
	}
}

static inline int handleMissing (const char* dbcFile, size_t lineNumber, const char* fieldName)
{
	errno = EINVAL;
	debugPrintf ("DBC file '%s' line %lu is missing %s.\n", dbcFile, (long unsigned) lineNumber, fieldName);
	return errno;
}

int canDbcLoad (const char* dbcFile)
{
	FILE* file = fopen (dbcFile, "r");
	if (file == NULL)
		return errno;

	size_t lineNumber = 0;

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

		char* strtokSavePtr;
		char* keyword = strtok_r (line, " ", &strtokSavePtr);
		if (keyword == NULL)
			return handleMissing (dbcFile, lineNumber, "keyword");

		if (strcmp (keyword, KEYWORD_MESSAGE) == 0)
		{
			// BO_ <id> <name>: <DLC> <Network Node>
			char* id = strtok_r (NULL, " ", &strtokSavePtr);
			if (id == NULL)
				return handleMissing (dbcFile, lineNumber, "message ID");

			char* name = strtok_r (NULL, ": ", &strtokSavePtr);
			if (name == NULL)
				return handleMissing (dbcFile, lineNumber, "message name");

			char* dlc = strtok_r (NULL, " ", &strtokSavePtr);
			if (dlc == NULL)
				return handleMissing (dbcFile, lineNumber, "message DLC");

			printf ("MESSAGE: '%s', ID: %s, DLC %s.\n", name, id, dlc);
		}
		else if (strcmp (keyword, KEYWORD_SIGNAL) == 0)
		{
			// Format: SG_ <Name> : <Bit position>|<Bit length>@<Endianness><Signedness> (<Scale factor>,<Offset>) [<Min>|<Max>] "<Unit>" <Network Node>
			char* name = strtok_r (NULL, " :@|,()[]\"", &strtokSavePtr);
			if (name == NULL)
				return handleMissing (dbcFile, lineNumber, "signal name");

			char* bitPosition = strtok_r (NULL, " :@|,()[]", &strtokSavePtr);
			if (bitPosition == NULL)
				return handleMissing (dbcFile, lineNumber, "signal bit position");

			char* bitLength = strtok_r (NULL, " :@|,()[]", &strtokSavePtr);
			if (bitLength == NULL)
				return handleMissing (dbcFile, lineNumber, "signal bit length");

			char* endiannessSignedness = strtok_r (NULL, " :@|,()[]", &strtokSavePtr);

			char* scaleFactor = strtok_r (NULL, " :@|,()[]", &strtokSavePtr);
			if (scaleFactor == NULL)
				return handleMissing (dbcFile, lineNumber, "signal scale factor");

			char* offset = strtok_r (NULL, " :@|,()[]", &strtokSavePtr);
			if (offset == NULL)
				return handleMissing (dbcFile, lineNumber, "signal offset");

			char* min = strtok_r (NULL, " :@|,()[]", &strtokSavePtr);
			if (min == NULL)
				return handleMissing (dbcFile, lineNumber, "signal minimum");

			char* max = strtok_r (NULL, " :@|,()[]", &strtokSavePtr);
			if (max == NULL)
				return handleMissing (dbcFile, lineNumber, "signal maximum");

			char* unit = strtok_r (NULL, "", &strtokSavePtr);
			if (unit == NULL)
				return handleMissing (dbcFile, lineNumber, "signal unit");

			for (size_t index = 0; index < 2; ++index)
			{
				if (*unit != '\0')
					++unit;
			}

			printf ("UNIT = '%s'\n", unit);

			if (unit == NULL)
				return handleMissing (dbcFile, lineNumber, "signal unit");

			char* networkNode = strtok_r (NULL, " \n", &strtokSavePtr);
			if (networkNode == NULL)
				return handleMissing (dbcFile, lineNumber, "signal network node");

			// printf ("SIGNAL: '%s, pos: '%s', len: '%s', endianSign: '%s', sf: '%s', off: '%s', min: '%s', max: '%s', unit: '%s', node: '%s'\n'",
			// 	name, bitPosition, bitLength, endiannessSignedness, scaleFactor, offset, min, max, unit, networkNode);
		}
		else
		{
			debugPrintf ("Warning, ignoring unknown keyword '%s' in DBC file.\n", keyword);
		}
	}

	return 0;
}