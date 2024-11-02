// Header
#include "can_dbc.h"

// C Standard Library
#include <ctype.h>
#include <errno.h>
#include <string.h>
#include <math.h>

// Debugging ------------------------------------------------------------------------------------------------------------------

#if CAN_DBC_DEBUG_INFO
	#include <stdio.h>
	#define INFO_PRINTF(...) printf(__VA_ARGS__)
#else
	#define INFO_PRINTF(...) while (false)
#endif // CAN_DBC_DEBUG_INFO

#if CAN_DBC_DEBUG_PARSING
	#include <stdio.h>
	#define PARSE_PRINTF(...) printf(__VA_ARGS__)
#else
	#define PARSE_PRINTF(...) while (false)
#endif // CAN_DBC_DEBUG_PARSING

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

bool dbcFileParse (const char* path, canMessage_t* messages, size_t* messageCount, canSignal_t* signals, size_t* signalCount)
{
	int code;
	char dataBuffer0 [LINE_LENGTH_MAX];
	char dataBuffer1 [LINE_LENGTH_MAX];
	char dataBuffer2 [LINE_LENGTH_MAX];

	INFO_PRINTF ("Parsing DBC file '%s'...\n", path);

	size_t maxMessages = *messageCount;
	size_t maxSignals = *signalCount;
	*messageCount = 0;
	*signalCount = 0;

	// Open the file for reading
	FILE* file = fopen (path, "r");
	if (file == NULL)
	{
		code = errno;
		INFO_PRINTF ("Failed to parse DBC file: %s\n", strerror (code));
		return false;
	}

	canMessage_t* message = NULL;

	while (true)
	{
		// Read the next keyword in the file.
		// TODO(Barach): Unbounded scan
		code = fscanf (file, "%s", dataBuffer0);		
		
		// Check for the end of the file.
		if (feof (file))
			break;
		
		if (code != 1)
		{   
			code = errno;
			INFO_PRINTF ("Failed to parse DBC file: %s\n", strerror(code));
			return false;
		}


		// Identify the keyword.
		if (strcmp (dataBuffer0, DBC_KEYWORD_NETWORK_NODE) == 0)
		{
			// Ignore remainder of line.
			fgets (dataBuffer0, LINE_LENGTH_MAX, file);
		}
		else if (strcmp(dataBuffer0, DBC_KEYWORD_MESSAGE) == 0)
		{
			if (*messageCount == maxMessages)
			{
				INFO_PRINTF ("Failed to parse DBC file: The file exceeds the maximum number of messages (%lu).\n",
					maxMessages);
				return false;
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
				INFO_PRINTF ("Failed to parse DBC message '%s': Invalid format.\n", dataBuffer0);

				// Ignore remainder of line
				fgets (dataBuffer0, LINE_LENGTH_MAX, file);
			}
			else
			{
				PARSE_PRINTF ("Message name: %s, ID: %u, DLC: %u, ECU: %s\n", dataBuffer0, messageId, messageDlc, dataBuffer1);
				
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
				message->id = (uint16_t) messageId;
				message->dlc = (uint8_t) messageDlc;
			}
		}
		else if (strcmp (dataBuffer0, DBC_KEYWORD_SIGNAL) == 0)
		{
			if(message == NULL)
			{
				INFO_PRINTF ("Failed to parse DBC file: Read a signal before reading the first message.\n");
				return false;
			}

			if(*signalCount == maxSignals)
			{
				INFO_PRINTF ("Failed to parse DBC file: The file exceeds the maximum number of signals (%lu).\n", maxSignals);
				return false;
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
			code = fscanf(file, "%s : %u|%u@%u%c (%lf,%lf) [%lf|%lf] \"%s %s", dataBuffer0, &bitPosition, &bitLength, &endianness, &signedness, &scaleFactor, &offset, &min, &max, dataBuffer1, dataBuffer2);

			// Remove ending quote from unit name
			size_t charIndex = 0;
			while (dataBuffer1 [charIndex] != '\0')
				++charIndex;
			if (charIndex > 0) dataBuffer1 [charIndex - 1] = '\0';

			// Validate input
			if(code != 11)
			{
				INFO_PRINTF ("Failed to parse DBC signal '%s': Invalid format.\n", dataBuffer0);
				
				// Ignore remainder of line
				fgets(dataBuffer0, LINE_LENGTH_MAX, file);
			}
			else
			{
				PARSE_PRINTF ("Signal name: %s, Bit Pos: %i, Bit Len: %i, Endianness: %i, Sign: %c, Scale: %f, Offset: %f, Min: %f, Max: %f, Unit: %s, ECU: %s\n", dataBuffer0, bitPosition, bitLength, endianness, signedness, scaleFactor, offset, min, max, dataBuffer1, dataBuffer2);

				// Add signal to array
				canSignal_t* signal = signals + *signalCount;
				++(*signalCount);
				++message->signalCount;

				signal->message = message;
				
				// Allocate and copy signal name
				size_t nameSize = strlen (dataBuffer0);
				signal->name = malloc (nameSize + 1);
				strcpy (signal->name, dataBuffer0);

				// Copy signal metadata
				signal->bitPosition = (uint8_t) (bitPosition);
				signal->bitLength   = (uint8_t) (bitLength);
				signal->scaleFactor = scaleFactor;
				signal->offset      = offset;
				signal->signedness  = (signedness == '-');
				// TODO(Barach): Random values
				signal->endianness  = (endianness == 1);

				// Populate bitmask
				signal->bitmask = ((uint64_t) 1 << signal->bitLength) - 1;
			}
		}
		else if (strcmp (dataBuffer0, DBC_KEYWORD_ENV_VARIABLE) == 0)
		{
			// TODO(Barach): Figure out format
			INFO_PRINTF ("Environment variables are not implemented. Ignoring...\n");
		}
		else if (strcmp(dataBuffer0, DBC_KEYWORD_SIG_GROUP) == 0)
		{
			// TODO(Barach): Figure out format
			INFO_PRINTF ("Signal groups are not implemented. Ignoring...\n");
		}
		else if (strcmp (dataBuffer0, DBC_KEYWORD_VAL_TABLE) == 0)
		{
			INFO_PRINTF ("Value tables are not implemented. Ignoring...\n");

			// Ignore remainder of line
			fgets (dataBuffer0, LINE_LENGTH_MAX, file);
		}
		else if (strcmp (dataBuffer0, DBC_KEYWORD_VERSION) == 0)
		{
			// Ignore remainder of line
			fgets (dataBuffer0, LINE_LENGTH_MAX, file);
		}
		else if (strcmp (dataBuffer0, DBC_KEYWORD_BIT_TIMING) == 0)
		{
			// Ignore remainder of line
			fgets (dataBuffer0, LINE_LENGTH_MAX, file);
		}
		else if (strcmp (dataBuffer0, DBC_KEYWORD_COMMENT) == 0)
		{
			// Ignore remainder of line
			fgets (dataBuffer0, LINE_LENGTH_MAX, file);
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
				char* errorString = fgets (dataBuffer0, LINE_LENGTH_MAX, file);
				if(errorString == NULL)
					break;
			}
		}
		else
		{
			INFO_PRINTF ("Unknown keyword '%s'. Ignoring line...\n", dataBuffer0);

			// Ignore remainder of line
			fgets (dataBuffer0, LINE_LENGTH_MAX, file);
		}

		// Check for the end of the file
		if(feof(file))
			break;
	}

	INFO_PRINTF ("Finished parsing DBC file '%s'.\n", path);

	// Close the file
	if(file != NULL)
		fclose(file);

	return true;
}