// Header
#include "can_dbc.h"

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

// DBC Keywords ---------------------------------------------------------------------------------------------------------------

#define DBC_KEYWORD_NETWORK_NODE	"BU_:"			// CAN ECU
#define DBC_KEYWORD_MESSAGE			"BO_"			// CAN message
#define DBC_KEYWORD_SIGNAL			"SG_"			// CAN signal
#define DBC_KEYWORD_ENV_VARIABLE	"EV_"			// Environment variable
#define DBC_KEYWORD_SIG_GROUP		"SIG_GROUP_"	// Signal group
#define DBC_KEYWORD_VAL_TABLE		"VAL_TABLE_"	// Value table
#define DBC_KEYWORD_VERSION			"VERSION"		// Version Number
#define DBC_KEYWORD_BIT_TIMING		"BS_:"			// Network baudrate, obsolete
#define DBC_KEYWORD_COMMENT			"CM_"			// Comments, ignored for now
#define DBC_KEYWORD_NS				"NS_"			// Purpose unknown, ignored for now

// Functions ------------------------------------------------------------------------------------------------------------------

int dbcFileParse (const char* path, canMessage_t* messages, size_t* messageCount, canSignal_t* signals, size_t* signalCount)
{
	char dataBuffer0 [CAN_DBC_LINE_LENGTH_MAX];
	char dataBuffer1 [CAN_DBC_LINE_LENGTH_MAX];
	char dataBuffer2 [CAN_DBC_LINE_LENGTH_MAX];

	size_t maxMessages = *messageCount;
	size_t maxSignals = *signalCount;
	*messageCount = 0;
	*signalCount = 0;

	// Open the file for reading
	FILE* file = fopen (path, "r");
	if (file == NULL)
		return errno;

	canMessage_t* message = NULL;

	while (true)
	{
		// Read the next keyword in the file.
		// TODO(Barach): Unbounded scan
		int code = fscanf (file, "%s", dataBuffer0);

		// Check for the end of the file.
		if (feof (file))
			break;

		// Check the keyword was read
		if (code != 1)
			return errno;

		// Identify the keyword.
		if (strcmp (dataBuffer0, DBC_KEYWORD_NETWORK_NODE) == 0)
		{
			// Ignore remainder of line.
			fgets (dataBuffer0, CAN_DBC_LINE_LENGTH_MAX, file);
		}
		else if (strcmp(dataBuffer0, DBC_KEYWORD_MESSAGE) == 0)
		{
			if (*messageCount == maxMessages)
			{
				errno = ERRNO_CAN_DBC_MESSAGE_COUNT;
				return errno;
			}

			// Create data buffers
			unsigned int messageId;
			unsigned int messageDlc;

			// BO_ <id> <name>: <DLC> <Network Node>
			// TODO(Barach): Overflow
			code = fscanf(file, "%u %s %u %s", &messageId, dataBuffer0, &messageDlc, dataBuffer1);

			// Remove semicolon from message name
			size_t charIndex = 0;
			while (dataBuffer0 [charIndex] != '\0')
				++charIndex;
			if (charIndex > 0) dataBuffer0 [charIndex - 1] = '\0';

			// Validate input
			if (code != 4)
			{
				// Ignore remainder of line
				fgets (dataBuffer0, CAN_DBC_LINE_LENGTH_MAX, file);
			}
			else
			{
				// Add message to array
				message = messages + *messageCount;
				++(*messageCount);

				message->signals = signals + *signalCount;
				message->signalCount = 0;

				// Allocate and copy the message name
				size_t nameSize = strlen (dataBuffer0);
				message->name = malloc (nameSize + 1);
				strcpy (message->name, dataBuffer0);

				// Copy message metadata
				message->id = (uint32_t) messageId & ID_ID_BIT_MASK;
				message->ide = (messageId & ID_IDE_BIT_MASK) == ID_IDE_BIT_MASK;
				message->dlc = (uint8_t) messageDlc;
			}
		}
		else if (strcmp (dataBuffer0, DBC_KEYWORD_SIGNAL) == 0)
		{
			if(message == NULL)
			{
				errno = ERRNO_CAN_DBC_MESSAGE_MISSING;
				return errno;
			}

			if(*signalCount == maxSignals)
			{
				errno = ERRNO_CAN_DBC_SIGNAL_COUNT;
				return errno;
			}

			// Create data buffers
			unsigned int bitPosition;
			unsigned int bitLength;
			unsigned int endianness;
			char signedness;
			double scaleFactor;
			double offset;
			double min;
			double max;

			// Format: SG_ <Name> : <Bit position>|<Bit length>@<Endianness><Signedness> (<Scale factor>,<Offset>) [<Min>|<Max>] "<Unit>" <Network Node>
			code = fscanf (file, "%s : %u|%u@%u%c (%lf,%lf) [%lf|%lf] \"%s %s", dataBuffer0, &bitPosition, &bitLength, &endianness, &signedness, &scaleFactor, &offset, &min, &max, dataBuffer1, dataBuffer2);

			// Remove ending quote from unit name
			size_t charIndex = 0;
			while (dataBuffer1 [charIndex] != '\0')
				++charIndex;
			if (charIndex > 0) dataBuffer1 [charIndex - 1] = '\0';

			// Validate input
			if (code != 11)
			{
				// Ignore remainder of line
				fgets(dataBuffer0, CAN_DBC_LINE_LENGTH_MAX, file);
			}
			else
			{
				// Add signal to array
				canSignal_t* signal = signals + *signalCount;
				++(*signalCount);
				++message->signalCount;

				signal->message = message;

				// Allocate and copy signal name
				size_t nameSize = strlen (dataBuffer0);
				signal->name = malloc (nameSize + 1);
				strcpy (signal->name, dataBuffer0);

				if (scaleFactor == 0)
					scaleFactor = 1;

				// Copy signal metadata
				signal->bitPosition = (uint8_t) (bitPosition);
				signal->bitLength   = (uint8_t) (bitLength);
				signal->scaleFactor = scaleFactor;
				signal->offset      = offset;
				signal->signedness  = (signedness == '-');
				signal->endianness  = (endianness == 1);

				// Handle motorola format
				if (!endianness)
				{
					if (signal->bitLength >= 8)
						signal->bitPosition -= 7;
					else
						signal->bitPosition -= signal->bitLength - 1;
				}

				// Populate bitmask
				signal->bitmask = ((uint64_t) 1 << signal->bitLength) - 1;
			}
		}
		else if (strcmp (dataBuffer0, DBC_KEYWORD_ENV_VARIABLE) == 0)
		{
			// TODO(Barach): Figure out format
		}
		else if (strcmp(dataBuffer0, DBC_KEYWORD_SIG_GROUP) == 0)
		{
			// TODO(Barach): Figure out format
		}
		else if (strcmp (dataBuffer0, DBC_KEYWORD_VAL_TABLE) == 0)
		{
			// Ignore remainder of line
			fgets (dataBuffer0, CAN_DBC_LINE_LENGTH_MAX, file);
		}
		else if (strcmp (dataBuffer0, DBC_KEYWORD_VERSION) == 0)
		{
			// Ignore remainder of line
			fgets (dataBuffer0, CAN_DBC_LINE_LENGTH_MAX, file);
		}
		else if (strcmp (dataBuffer0, DBC_KEYWORD_BIT_TIMING) == 0)
		{
			// Ignore remainder of line
			fgets (dataBuffer0, CAN_DBC_LINE_LENGTH_MAX, file);
		}
		else if (strcmp (dataBuffer0, DBC_KEYWORD_COMMENT) == 0)
		{
			// Ignore remainder of line
			fgets (dataBuffer0, CAN_DBC_LINE_LENGTH_MAX, file);
		}
		else if (strcmp (dataBuffer0, DBC_KEYWORD_NS) == 0)
		{
			// Ignore every following line starting with whitespace
			while(true)
			{
				// Read first char and check to see if it is whitespace
				code = fgetc (file);
				if (code == EOF)
					break;

				unsigned char startingChar = (unsigned char) (code);
				if (!isspace (startingChar) || startingChar == '\n')
				{
					// Return the stream to its previous state and return to normal loop
					ungetc (startingChar, file);
					break;
				}

				// Ignore remainder of line
				char* errorString = fgets (dataBuffer0, CAN_DBC_LINE_LENGTH_MAX, file);
				if(errorString == NULL)
					break;
			}
		}
		else
		{
			// Ignore remainder of line
			fgets (dataBuffer0, CAN_DBC_LINE_LENGTH_MAX, file);
		}

		// Check for the end of the file
		if(feof(file))
			break;
	}

	// Close the file
	if(file != NULL)
		fclose(file);

	return 0;
}