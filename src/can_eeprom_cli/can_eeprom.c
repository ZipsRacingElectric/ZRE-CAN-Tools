// Header
#include "can_eeprom.h"
#include "error_codes.h"

// C Standard Library
#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <sys/time.h>

// Macros ---------------------------------------------------------------------------------------------------------------------

#if CAN_EEPROM_DEBUG
	#include <stdio.h>
	#define DEBUG_PRINTF(...) fprintf (stderr, __VA_ARGS__)
#else
	#define DEBUG_PRINTF(...) while (false)
#endif // CAN_DATABASE_DEBUG

// Constants ------------------------------------------------------------------------------------------------------------------

#define RESPONSE_ATTEMPT_COUNT 7
static const struct timeval RESPONSE_ATTEMPT_TIMEOUT =
{
	.tv_sec		= 0,
	.tv_usec	= 200000
};

#define VARIABLE_COUNT (sizeof (VARIABLE_SIZES) / sizeof (uint16_t))
static const uint16_t VARIABLE_SIZES [] =
{
	sizeof (uint8_t),
	sizeof (uint16_t),
	sizeof (uint32_t),
	sizeof (float)
};

static const char* VARIABLE_NAMES [] =
{
	"uint8_t",
	"uint16_t",
	"uint32_t",
	"float"
};

// Macros ---------------------------------------------------------------------------------------------------------------------

#define EEPROM_COMMAND_MESSAGE_READ_NOT_WRITE(rnw)			(((uint16_t) (rnw))	<< 0)
#define EEPROM_COMMAND_MESSAGE_DATA_NOT_VALIDATION(dnv)		(((uint16_t) (dnv))	<< 1)
#define EEPROM_COMMAND_MESSAGE_IS_VALID(iv)					(((uint16_t) (iv))	<< 2)
#define EEPROM_COMMAND_MESSAGE_DATA_COUNT(dc)				(((uint16_t) ((dc) - 1)) << 2)

#define EEPROM_RESPONSE_MESSAGE_READ_NOT_WRITE(word)		(((word) & 0b00000001) == 0b00000001)
#define EEPROM_RESPONSE_MESSAGE_DATA_NOT_VALIDATION(word)	(((word) & 0b00000010) == 0b00000010)
#define EEPROM_RESPONSE_MESSAGE_IS_VALID(word)				(((word) & 0b00000100) == 0b00000100)
#define EEPROM_RESPONSE_MESSAGE_DATA_COUNT(word)			((((word) & 0b00001100) >> 2) + 1)

// Function Prototypes --------------------------------------------------------------------------------------------------------

struct can_frame writeMessageEncode (canEeprom_t* eeprom, uint16_t variableIndex, void* data);

struct can_frame readMessageEncode (canEeprom_t* eeprom, uint16_t variableIndex);

int readMessageParse (canEeprom_t* eeprom, struct can_frame* frame, uint16_t variableIndex, void* data);

struct can_frame validateMessageEncode (canEeprom_t* eeprom, bool isValid);

struct can_frame isValidMessageEncode (canEeprom_t* eeprom);

int isValidMessageParse (canEeprom_t* eeprom, struct can_frame* frame, bool* isValid);

void valueParse (canEeprom_t* eeprom, uint16_t variableIndex, const char* string, void* data);

// Functions ------------------------------------------------------------------------------------------------------------------

int canEepromInit (canEeprom_t* eeprom, cJSON* json)
{
	if (jsonGetString (json, "name", &eeprom->name) != 0)
		return errno;

	if (jsonGetUint16_t (json, "canAddress", &eeprom->canAddress) != 0)
		return errno;

	cJSON* variableMap;
	if (jsonGetObject (json, "variableMap", &variableMap) != 0)
		return errno;

	eeprom->variableCount = cJSON_GetArraySize (variableMap);
	eeprom->variables = malloc (eeprom->variableCount * sizeof (canEepromVariable_t));
	if (eeprom->variables == NULL)
		return errno;

	for (uint16_t index = 0; index < eeprom->variableCount; ++index)
	{
		cJSON* element = cJSON_GetArrayItem (variableMap, index);
		if (jsonGetUint16_t (element, "address", &(eeprom->variables [index].address)) != 0)
			return errno;

		if (jsonGetString (element, "name", &(eeprom->variables [index].name)) != 0)
			return errno;

		char* variableType;
		if (jsonGetString (element, "type", &variableType) != 0)
			return errno;

		if (strcmp (variableType, "uint8_t") == 0)
			eeprom->variables [index].type = CAN_EEPROM_TYPE_UINT8_T;
		else if (strcmp (variableType, "uint16_t") == 0)
			eeprom->variables [index].type = CAN_EEPROM_TYPE_UINT16_T;
		else if (strcmp (variableType, "uint32_t") == 0)
			eeprom->variables [index].type = CAN_EEPROM_TYPE_UINT32_T;
		else if (strcmp (variableType, "float") == 0)
			eeprom->variables [index].type = CAN_EEPROM_TYPE_FLOAT;
		else
		{
			errno = ERRNO_CAN_EEPROM_INVALID_TYPE;
			return errno;
		}
	}

	return 0;
}

int canEepromWrite (canEeprom_t* eeprom, canSocket_t* socket, uint16_t variableIndex, void* data)
{
	struct can_frame commandFrame = writeMessageEncode (eeprom, variableIndex, data);

	uint8_t readData [4];
	for (uint8_t attempt = 0; attempt < RESPONSE_ATTEMPT_COUNT; ++attempt)
	{
		uint8_t size = VARIABLE_SIZES [eeprom->variables [variableIndex].type];
		canSocketTransmit (socket, &commandFrame);
		if (canEepromRead (eeprom, socket, variableIndex, readData) != 0)
			return errno;

		if (memcmp (data, readData, size) == 0)
			return 0;
	}

	errno = ERRNO_CAN_EEPROM_WRITE_TIMEOUT;
	return errno;
}

int canEepromRead (canEeprom_t* eeprom, canSocket_t* socket, uint16_t variableIndex, void* data)
{
	struct can_frame commandFrame = readMessageEncode (eeprom, variableIndex);

	for (uint8_t attempt = 0; attempt < RESPONSE_ATTEMPT_COUNT; ++attempt)
	{
		if (canSocketTransmit (socket, &commandFrame) != 0)
			return errno;

		struct timeval deadline;
		struct timeval timeCurrent;
		gettimeofday (&timeCurrent, NULL);
		timeradd (&timeCurrent, &RESPONSE_ATTEMPT_TIMEOUT, &deadline);

		while (timercmp (&timeCurrent, &deadline, <))
		{
			gettimeofday (&timeCurrent, NULL);

			struct can_frame response;
			if (canSocketReceive (socket, &response) != 0)
				continue;

			if (readMessageParse (eeprom, &response, variableIndex, data) == 0)
				return 0;
		}
	}

	errno = ERRNO_CAN_EEPROM_READ_TIMEOUT;
	return errno;
}

int canEepromValidate (canEeprom_t* eeprom, canSocket_t* socket, bool isValid)
{
	struct can_frame commandFrame = validateMessageEncode (eeprom, isValid);

	bool readIsValid;
	for (uint8_t attempt = 0; attempt < RESPONSE_ATTEMPT_COUNT; ++attempt)
	{
		canSocketTransmit (socket, &commandFrame);
		if (canEepromIsValid (eeprom, socket, &readIsValid) != 0)
			return errno;

		if (readIsValid == isValid)
			return 0;
	}

	errno = ERRNO_CAN_EEPROM_VALIDATE_TIMEOUT;
	return errno;
}

int canEepromIsValid (canEeprom_t* eeprom, canSocket_t* socket, bool* isValid)
{
	struct can_frame commandFrame = isValidMessageEncode (eeprom);

	for (uint8_t attempt = 0; attempt < RESPONSE_ATTEMPT_COUNT; ++attempt)
	{
		if (canSocketTransmit (socket, &commandFrame) != 0)
			return errno;

		struct timeval deadline;
		struct timeval timeCurrent;
		gettimeofday (&timeCurrent, NULL);
		timeradd (&timeCurrent, &RESPONSE_ATTEMPT_TIMEOUT, &deadline);

		while (timercmp (&timeCurrent, &deadline, <))
		{
			gettimeofday (&timeCurrent, NULL);

			struct can_frame response;
			if (canSocketReceive (socket, &response) != 0)
				continue;

			if (isValidMessageParse (eeprom, &response, isValid) == 0)
				return 0;
		}
	}

	errno = ERRNO_CAN_EEPROM_IS_VALID_TIMEOUT;
	return errno;
}

int canEepromProgram (canEeprom_t* eeprom, canSocket_t* socket, cJSON* json)
{
	size_t keyCount = cJSON_GetArraySize (json);

	for (size_t keyIndex = 0; keyIndex < keyCount; ++keyIndex)
	{
		cJSON* key = cJSON_GetArrayItem (json, keyIndex);

		uint16_t variableIndex = 0;
		for (; variableIndex < eeprom->variableCount; ++variableIndex)
		{
			if (strcmp (key->string, eeprom->variables [variableIndex].name) != 0)
				continue;

			char* string = cJSON_GetStringValue (key);
			if (string == NULL)
			{
				errno = ERRNO_CAN_EEPROM_BAD_VALUE;
				return errno;
			}

			uint32_t data;
			valueParse(eeprom, variableIndex, string, &data);

			if (canEepromWrite (eeprom, socket, variableIndex, &data) != 0)
				return errno;

			break;
		}

		if (variableIndex == eeprom->variableCount)
		{
			DEBUG_PRINTF ("canEepromProgram failed: Bad key '%s'.\n", key->string);
			errno = ERRNO_CAN_EEPROM_BAD_KEY;
			return errno;
		}
	}

	return 0;
}

int canEepromRecover (canEeprom_t* eeprom, canSocket_t* socket, FILE* stream)
{
	for (uint16_t index = 0; index < eeprom->variableCount; ++index)
	{
		canEepromVariable_t* variable = eeprom->variables + index;

		uint32_t data;
		if (canEepromRead (eeprom, socket, index, &data) != 0)
			return errno;

		if (index == 0)
			fprintf (stream, "{");

		fprintf (stream, "\t\"%s\": ", variable->name);

		switch (variable->type)
		{
		case CAN_EEPROM_TYPE_UINT8_T:
			fprintf (stream, "\"%u\"", *((uint8_t*) &data));
			break;
		case CAN_EEPROM_TYPE_UINT16_T:
			fprintf (stream, "\"%u\"", *((uint16_t*) &data));
			break;
		case CAN_EEPROM_TYPE_UINT32_T:
			fprintf (stream, "\"%u\"", *((uint32_t*) &data));
			break;
		case CAN_EEPROM_TYPE_FLOAT:
			fprintf (stream, "\"%f\"", *((float*) &data));
			break;
		}

		if (index == eeprom->variableCount - 1)
			fprintf (stream, ",");
		else
			fprintf (stream, "\n}");
	}

	return 0;
}

// Standard I/O ---------------------------------------------------------------------------------------------------------------

uint16_t canEepromVariablePrompt (canEeprom_t* eeprom)
{
	char buffer [256];

	while (true)
	{
		fprintf (stderr, "Enter the key of the variable: ");
		fgets (buffer, sizeof (buffer), stdin);
		buffer [strcspn (buffer, "\r\n")] = '\0';

		for (uint16_t index = 0; index < eeprom->variableCount; ++index)
		{
			if (strcmp (buffer, eeprom->variables [index].name) != 0)
				continue;

			return index;
		}

		fprintf (stderr, "Unknown key '%s'\n", buffer);
	}
}

void canEepromValuePrompt (canEeprom_t* eeprom, uint16_t variableIndex, void* data)
{
	canEepromVariable_t* variable = eeprom->variables + variableIndex;

	fprintf (stderr, "Enter the value of the variable (%s): ", VARIABLE_NAMES [variable->type]);

	switch (variable->type)
	{
	case CAN_EEPROM_TYPE_UINT8_T:
	case CAN_EEPROM_TYPE_UINT16_T:
	case CAN_EEPROM_TYPE_UINT32_T:
		fscanf (stdin, "%u%*1[\n]", (uint32_t*) data);
		break;
	case CAN_EEPROM_TYPE_FLOAT:
		fscanf (stdin, "%f%*1[\n]", (float*) data);
		break;
	}
}

void canEepromPrintVariable (FILE* stream, canEeprom_t* eeprom, uint16_t variableIndex, void* data)
{
	canEepromVariable_t* variable = eeprom->variables + variableIndex;
	fprintf (stream, "%s (%s): ", variable->name, VARIABLE_NAMES [variable->type]);

	switch (variable->type)
	{
	case CAN_EEPROM_TYPE_UINT8_T:
		fprintf (stream, "%u\n", *((uint8_t*) data));
		break;
	case CAN_EEPROM_TYPE_UINT16_T:
		fprintf (stream, "%u\n", *((uint16_t*) data));
		break;
	case CAN_EEPROM_TYPE_UINT32_T:
		fprintf (stream, "%u\n", *((uint32_t*) data));
		break;
	case CAN_EEPROM_TYPE_FLOAT:
		fprintf (stream, "%f\n", *((float*) data));
		break;
	}
}

int canEepromPrintMap (FILE* stream, canEeprom_t* eeprom, canSocket_t* socket)
{
	bool isValid;
	if (canEepromIsValid (eeprom, socket, &isValid) != 0)
		return errno;

	fprintf (stream, "%s Memory Map: %s\n", eeprom->name, isValid ? "Valid" : "Invalid");
	fprintf (stream, "---------------------------------------------------\n");
	fprintf (stream, "%24s | %10s | %10s\n", "Variable", "Type", "Value");
	fprintf (stream, "-------------------------|------------|------------\n");

	for (uint16_t index = 0; index < eeprom->variableCount; ++index)
	{
		uint8_t data [4];
		if (canEepromRead (eeprom, socket, index, data) != 0)
			return errno;

		canEepromVariable_t* variable = eeprom->variables + index;
		fprintf (stream, "%24s | %10s | ", variable->name, VARIABLE_NAMES [variable->type]);

		switch (variable->type)
		{
		case CAN_EEPROM_TYPE_UINT8_T:
			fprintf (stream, "%10u\n", *((uint8_t*) data));
			break;
		case CAN_EEPROM_TYPE_UINT16_T:
			fprintf (stream, "%10u\n", *((uint16_t*) data));
			break;
		case CAN_EEPROM_TYPE_UINT32_T:
			fprintf (stream, "%10u\n", *((uint32_t*) data));
			break;
		case CAN_EEPROM_TYPE_FLOAT:
			fprintf (stream, "%10f\n", *((float*) data));
			break;
		}
	}

	fprintf (stream, "\n");

	return 0;
}

void canEepromPrintEmptyMap (FILE* stream, canEeprom_t* eeprom)
{
	fprintf (stream, "%s Memory Map:\n", eeprom->name);
	fprintf (stream, "-----------------------------------------------------\n");
	fprintf (stream, "%24s | %10s | %10s\n", "Variable", "Type", "Address");
	fprintf (stream, "-------------------------|------------|--------------\n");

	for (uint16_t index = 0; index < eeprom->variableCount; ++index)
	{
		canEepromVariable_t* variable = eeprom->variables + index;
		fprintf (stream, "%24s | %10s |       0x%04X\n", variable->name, VARIABLE_NAMES [variable->type], variable->address);
	}

	fprintf (stream, "\n");
}

// Private Functions ----------------------------------------------------------------------------------------------------------

struct can_frame writeMessageEncode (canEeprom_t* eeprom, uint16_t variableIndex, void* data)
{
	canEepromVariable_t* variable = eeprom->variables + variableIndex;

	uint16_t count = VARIABLE_SIZES [variable->type];
	uint16_t instruction = EEPROM_COMMAND_MESSAGE_READ_NOT_WRITE (false)
		| EEPROM_COMMAND_MESSAGE_DATA_NOT_VALIDATION (true)
		| EEPROM_COMMAND_MESSAGE_DATA_COUNT (count);
	uint16_t address = variable->address;

	struct can_frame frame =
	{
		.can_id		= eeprom->canAddress,
		.can_dlc	= 8,
		.data		=
		{
			instruction,
			instruction >> 8,
			address,
			address >> 8
		}
	};
	memcpy (frame.data + 4, data, count);

	return frame;
}

struct can_frame readMessageEncode (canEeprom_t* eeprom, uint16_t variableIndex)
{
	canEepromVariable_t* variable = eeprom->variables + variableIndex;

	uint16_t count = VARIABLE_SIZES [variable->type];
	uint16_t instruction = EEPROM_COMMAND_MESSAGE_READ_NOT_WRITE (true)
		| EEPROM_COMMAND_MESSAGE_DATA_NOT_VALIDATION (true)
		| EEPROM_COMMAND_MESSAGE_DATA_COUNT (count);
	uint16_t address = variable->address;

	struct can_frame frame =
	{
		.can_id		= eeprom->canAddress,
		.can_dlc	= 8,
		.data		=
		{
			instruction,
			instruction >> 8,
			address,
			address >> 8
		}
	};

	return frame;
}

int readMessageParse (canEeprom_t* eeprom, struct can_frame* frame, uint16_t variableIndex, void* data)
{
	canEepromVariable_t* variable = eeprom->variables + variableIndex;

	uint16_t instruction = frame->data [0] | (frame->data [1] << 8);

	// Check message ID is correct
	if (frame->can_id != (uint16_t) (eeprom->canAddress + 1))
	{
		errno = ERRNO_CAN_EEPROM_BAD_RESPONSE_ID;
		return errno;
	}

	// Check message is a read response
	if (!EEPROM_RESPONSE_MESSAGE_READ_NOT_WRITE (instruction))
	{
		errno = ERRNO_CAN_EEPROM_MALFORMED_RESPONSE;
		return errno;
	}

	// Check message is a data response
	if (!EEPROM_RESPONSE_MESSAGE_DATA_NOT_VALIDATION (instruction))
	{
		errno = ERRNO_CAN_EEPROM_MALFORMED_RESPONSE;
		return errno;
	}

	uint16_t address = frame->data [2] | (frame->data [3] << 8);

	// Check message is the correct address
	if (address != variable->address)
	{
		errno = ERRNO_CAN_EEPROM_MALFORMED_RESPONSE;
		return errno;
	}

	// Check data count is correct
	uint8_t variableSize = VARIABLE_SIZES [variable->type];
	uint8_t dataCount = EEPROM_RESPONSE_MESSAGE_DATA_COUNT (instruction);
	if (dataCount != variableSize)
	{
		errno = ERRNO_CAN_EEPROM_MALFORMED_RESPONSE;
		return errno;
	}

	// Success, copy the data into the buffer
	memcpy (data, frame->data + 4, variableSize);
	return 0;
}

struct can_frame validateMessageEncode (canEeprom_t* eeprom, bool isValid)
{
	uint16_t instruction = EEPROM_COMMAND_MESSAGE_READ_NOT_WRITE (false)
		| EEPROM_COMMAND_MESSAGE_DATA_NOT_VALIDATION (false)
		| EEPROM_COMMAND_MESSAGE_IS_VALID (isValid);

	struct can_frame frame =
	{
		.can_id		= eeprom->canAddress,
		.can_dlc	= 8,
		.data		=
		{
			instruction,
			instruction >> 8
		}
	};

	return frame;
}

struct can_frame isValidMessageEncode (canEeprom_t* eeprom)
{
	uint16_t instruction = EEPROM_COMMAND_MESSAGE_READ_NOT_WRITE (true)
		| EEPROM_COMMAND_MESSAGE_DATA_NOT_VALIDATION (false);

	struct can_frame frame =
	{
		.can_id		= eeprom->canAddress,
		.can_dlc	= 8,
		.data		=
		{
			instruction,
			instruction >> 8,
		}
	};

	return frame;
}

int isValidMessageParse (canEeprom_t* eeprom, struct can_frame* frame, bool* isValid)
{
	uint16_t instruction = frame->data [0] | (frame->data [1] << 8);

	// Check message ID is correct
	if (frame->can_id != (uint16_t) (eeprom->canAddress + 1))
	{
		errno = ERRNO_CAN_EEPROM_BAD_RESPONSE_ID;
		return errno;
	}

	// Check message is a read response
	if (!EEPROM_RESPONSE_MESSAGE_READ_NOT_WRITE (instruction))
	{
		errno = ERRNO_CAN_EEPROM_MALFORMED_RESPONSE;
		return errno;
	}

	// Check message is a validation response
	if (EEPROM_RESPONSE_MESSAGE_DATA_NOT_VALIDATION (instruction))
	{
		errno = ERRNO_CAN_EEPROM_MALFORMED_RESPONSE;
		return errno;
	}

	// Success, read the isValid flag
	*isValid = EEPROM_RESPONSE_MESSAGE_IS_VALID (instruction);
	return 0;
}

void valueParse (canEeprom_t* eeprom, uint16_t variableIndex, const char* string, void* data)
{
	switch (eeprom->variables [variableIndex].type)
	{
	case CAN_EEPROM_TYPE_UINT8_T:
		*((uint8_t*) (&data)) = strtol (string, NULL, 0);
		break;
	case CAN_EEPROM_TYPE_UINT16_T:
		*((uint16_t*) (&data)) = strtol (string, NULL, 0);
		break;
	case CAN_EEPROM_TYPE_UINT32_T:
		*((uint32_t*) (&data)) = strtol (string, NULL, 0);
		break;
	case CAN_EEPROM_TYPE_FLOAT:
		*((float*) (&data)) = strtof (string, NULL);
		break;
	}
}