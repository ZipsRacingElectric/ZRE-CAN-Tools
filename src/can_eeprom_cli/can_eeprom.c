// Header
#include "can_eeprom.h"

// Includes
#include "error_codes.h"

// C Standard Library
#include <errno.h>
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

/// @brief Encodes the command frame for a data write operation.
struct can_frame writeMessageEncode (canEeprom_t* eeprom, uint16_t address, uint8_t count, void* buffer);

/// @brief Encodes the command frame for a data read operation.
struct can_frame readMessageEncode (canEeprom_t* eeprom, uint16_t address, uint8_t count);

/// @brief Parses a CAN frame to check whether it is the response to a data read command.
int readMessageParse (canEeprom_t* eeprom, struct can_frame* frame, uint16_t address, uint8_t count, void* buffer);

/// @brief Performs a single write operation on the EEPROM.
int writeSingle (canEeprom_t* eeprom, canSocket_t* socket, uint16_t address, uint8_t count, void* buffer);

/// @brief Performs a single read operation on the EEPROM.
int readSingle (canEeprom_t* eeprom, canSocket_t* socket, uint16_t address, uint8_t count, void* buffer);

struct can_frame validateMessageEncode (canEeprom_t* eeprom, bool isValid);

struct can_frame isValidMessageEncode (canEeprom_t* eeprom);

int isValidMessageParse (canEeprom_t* eeprom, struct can_frame* frame, bool* isValid);

void valueParse (canEepromVariable_t* variable, const char* string, void* buffer);

// Functions ------------------------------------------------------------------------------------------------------------------

int canEepromInit (canEeprom_t* eeprom, cJSON* json)
{
	if (jsonGetString (json, "name", &eeprom->name) != 0)
		return errno;

	if (jsonGetUint16_t (json, "canAddress", &eeprom->canAddress) != 0)
		return errno;

	cJSON* variables;
	if (jsonGetObject (json, "variables", &variables) != 0)
		return errno;

	eeprom->variableCount = cJSON_GetArraySize (variables);
	eeprom->variables = malloc (eeprom->variableCount * sizeof (canEepromVariable_t));
	if (eeprom->variables == NULL)
		return errno;

	for (uint16_t index = 0; index < eeprom->variableCount; ++index)
	{
		canEepromVariable_t* variable = eeprom->variables + index;
		cJSON* element = cJSON_GetArrayItem (variables, index);

		if (jsonGetUint16_t (element, "address", &variable->address) != 0)
			return errno;

		if (jsonGetString (element, "name", &variable->name) != 0)
			return errno;

		char* variableType;
		if (jsonGetString (element, "type", &variableType) != 0)
			return errno;

		if (strcmp (variableType, "uint8_t") == 0)
			variable->type = CAN_EEPROM_TYPE_UINT8_T;
		else if (strcmp (variableType, "uint16_t") == 0)
			variable->type = CAN_EEPROM_TYPE_UINT16_T;
		else if (strcmp (variableType, "uint32_t") == 0)
			variable->type = CAN_EEPROM_TYPE_UINT32_T;
		else if (strcmp (variableType, "float") == 0)
			variable->type = CAN_EEPROM_TYPE_FLOAT;
		else
		{
			errno = ERRNO_CAN_EEPROM_INVALID_TYPE;
			return errno;
		}
	}

	return 0;
}

int canEepromWrite (canEeprom_t* eeprom, canSocket_t* socket, uint16_t address, uint16_t count, void* buffer)
{
	// Write all of the full messages
	while (count > 4)
	{
		if (writeSingle (eeprom, socket, address, 4, buffer) != 0)
			return errno;

		buffer += 4;
		address += 4;
		count -= 4;
	}

	// Write the last message
	return writeSingle (eeprom, socket, address, count, buffer);
}

int canEepromRead (canEeprom_t* eeprom, canSocket_t* socket, uint16_t address, uint16_t count, void* buffer)
{
	// Read all of the full messages
	while (count > 4)
	{
		if (readSingle (eeprom, socket, address, 4, buffer) != 0)
			return errno;

		buffer += 4;
		address += 4;
		count -= 4;
	}

	// Read the last message
	return readSingle (eeprom, socket, address, count, buffer);
}

int canEepromWriteVariable (canEeprom_t* eeprom, canSocket_t* socket, canEepromVariable_t* variable, void* buffer)
{
	return writeSingle (eeprom, socket, variable->address, VARIABLE_SIZES [variable->type], buffer);
}

int canEepromReadVariable (canEeprom_t* eeprom, canSocket_t* socket, canEepromVariable_t* variable, void* buffer)
{
	return readSingle (eeprom, socket, variable->address, VARIABLE_SIZES [variable->type], buffer);
}

int canEepromWriteJson (canEeprom_t* eeprom, canSocket_t* socket, cJSON* dataJson)
{
	size_t keyCount = cJSON_GetArraySize (dataJson);

	for (size_t keyIndex = 0; keyIndex < keyCount; ++keyIndex)
	{
		cJSON* key = cJSON_GetArrayItem (dataJson, keyIndex);

		uint16_t variableIndex = 0;
		for (; variableIndex < eeprom->variableCount; ++variableIndex)
		{
			canEepromVariable_t* variable = eeprom->variables + variableIndex;

			if (strcmp (key->string, variable->name) != 0)
				continue;

			char* string = cJSON_GetStringValue (key);
			if (string == NULL)
			{
				errno = ERRNO_CAN_EEPROM_BAD_VALUE;
				return errno;
			}

			uint8_t buffer [4];
			valueParse (variable, string, buffer);

			if (canEepromWriteVariable (eeprom, socket, variable, buffer) != 0)
				return errno;

			break;
		}

		if (variableIndex == eeprom->variableCount)
		{
			DEBUG_PRINTF ("canEepromProgram failed: Unknown key '%s'.\n", key->string);
			errno = ERRNO_CAN_EEPROM_BAD_KEY;
			return errno;
		}
	}

	return 0;
}

int canEepromReadJson (canEeprom_t* eeprom, canSocket_t* socket, FILE* stream)
{
	for (uint16_t index = 0; index < eeprom->variableCount; ++index)
	{
		canEepromVariable_t* variable = eeprom->variables + index;

		uint8_t buffer [4];
		if (canEepromReadVariable (eeprom, socket, variable, buffer) != 0)
			return errno;

		if (index == 0)
			fprintf (stream, "{\n");

		fprintf (stream, "\t\"%s\": ", variable->name);

		switch (variable->type)
		{
		case CAN_EEPROM_TYPE_UINT8_T:
			fprintf (stream, "\"%u\"", *((uint8_t*) buffer));
			break;
		case CAN_EEPROM_TYPE_UINT16_T:
			fprintf (stream, "\"%u\"", *((uint16_t*) buffer));
			break;
		case CAN_EEPROM_TYPE_UINT32_T:
			fprintf (stream, "\"%u\"", *((uint32_t*) buffer));
			break;
		case CAN_EEPROM_TYPE_FLOAT:
			fprintf (stream, "\"%f\"", *((float*) buffer));
			break;
		}

		if (index == eeprom->variableCount - 1)
			fprintf (stream, "\n}\n");
		else
			fprintf (stream, ",\n");
	}

	return 0;
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

// Standard I/O ---------------------------------------------------------------------------------------------------------------

canEepromVariable_t* canEepromPromptVariable (canEeprom_t* eeprom, FILE* streamIn, FILE* streamOut)
{
	char buffer [256];

	while (true)
	{
		if (streamOut != NULL)
			fprintf (streamOut, "Enter the key of the variable: ");
		fgets (buffer, sizeof (buffer), streamIn);
		buffer [strcspn (buffer, "\r\n")] = '\0';

		for (uint16_t index = 0; index < eeprom->variableCount; ++index)
		{
			if (strcmp (buffer, eeprom->variables [index].name) != 0)
				continue;

			return eeprom->variables + index;
		}

		if (streamOut != NULL)
			fprintf (streamOut, "Unknown key '%s'\n", buffer);
	}
}

void canEepromPromptValue (canEepromVariable_t* variable, void* buffer, FILE* streamIn, FILE* streamOut)
{
	if (streamOut != NULL)
		fprintf (streamOut, "Enter the value (%s): ", VARIABLE_NAMES [variable->type]);

	switch (variable->type)
	{
	case CAN_EEPROM_TYPE_UINT8_T:
	case CAN_EEPROM_TYPE_UINT16_T:
	case CAN_EEPROM_TYPE_UINT32_T:
		fscanf (streamIn, "%u%*1[\n]", (uint32_t*) buffer);
		break;
	case CAN_EEPROM_TYPE_FLOAT:
		fscanf (streamIn, "%f%*1[\n]", (float*) buffer);
		break;
	}
}

void canEepromPrintVariable (canEepromVariable_t* variable, void* buffer, FILE* stream)
{
	switch (variable->type)
	{
	case CAN_EEPROM_TYPE_UINT8_T:
		fprintf (stream, "%u", *((uint8_t*) buffer));
		break;
	case CAN_EEPROM_TYPE_UINT16_T:
		fprintf (stream, "%u", *((uint16_t*) buffer));
		break;
	case CAN_EEPROM_TYPE_UINT32_T:
		fprintf (stream, "%u", *((uint32_t*) buffer));
		break;
	case CAN_EEPROM_TYPE_FLOAT:
		fprintf (stream, "%f", *((float*) buffer));
		break;
	}
}

int canEepromPrintMap (canEeprom_t* eeprom, canSocket_t* socket, FILE* stream)
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
		canEepromVariable_t* variable = eeprom->variables + index;
		uint8_t buffer [4];

		if (canEepromReadVariable (eeprom, socket, variable, buffer) != 0)
			return errno;

		fprintf (stream, "%24s | %10s | ", variable->name, VARIABLE_NAMES [variable->type]);
		canEepromPrintVariable (variable, buffer, stream);
		fprintf (stream, "\n");
	}

	fprintf (stream, "\n");

	return 0;
}

void canEepromPrintEmptyMap (canEeprom_t* eeprom, FILE* stream)
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

struct can_frame writeMessageEncode (canEeprom_t* eeprom, uint16_t address, uint8_t count, void* buffer)
{
	uint16_t instruction = EEPROM_COMMAND_MESSAGE_READ_NOT_WRITE (false)
		| EEPROM_COMMAND_MESSAGE_DATA_NOT_VALIDATION (true)
		| EEPROM_COMMAND_MESSAGE_DATA_COUNT (count);

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
	memcpy (frame.data + 4, buffer, count);

	return frame;
}

struct can_frame readMessageEncode (canEeprom_t* eeprom, uint16_t address, uint8_t count)
{
	uint16_t instruction = EEPROM_COMMAND_MESSAGE_READ_NOT_WRITE (true)
		| EEPROM_COMMAND_MESSAGE_DATA_NOT_VALIDATION (true)
		| EEPROM_COMMAND_MESSAGE_DATA_COUNT (count);

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

int readMessageParse (canEeprom_t* eeprom, struct can_frame* frame, uint16_t address, uint8_t count, void* buffer)
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

	// Check message is a data response
	if (!EEPROM_RESPONSE_MESSAGE_DATA_NOT_VALIDATION (instruction))
	{
		errno = ERRNO_CAN_EEPROM_MALFORMED_RESPONSE;
		return errno;
	}

	uint16_t frameAddress = frame->data [2] | (frame->data [3] << 8);

	// Check message is the correct address
	if (frameAddress != address)
	{
		errno = ERRNO_CAN_EEPROM_MALFORMED_RESPONSE;
		return errno;
	}

	// Check data count is correct
	uint8_t frameCount = EEPROM_RESPONSE_MESSAGE_DATA_COUNT (instruction);
	if (frameCount != count)
	{
		errno = ERRNO_CAN_EEPROM_MALFORMED_RESPONSE;
		return errno;
	}

	// Success, copy the data into the buffer
	memcpy (buffer, frame->data + 4, count);
	return 0;
}

int writeSingle (canEeprom_t* eeprom, canSocket_t* socket, uint16_t address, uint8_t count, void* buffer)
{
	struct can_frame commandFrame = writeMessageEncode (eeprom, address, count, buffer);

	uint8_t readBuffer [4];
	for (uint8_t attempt = 0; attempt < RESPONSE_ATTEMPT_COUNT; ++attempt)
	{
		canSocketTransmit (socket, &commandFrame);
		if (canEepromRead (eeprom, socket, address, count, readBuffer) != 0)
			return errno;

		if (memcmp (buffer, readBuffer, count) == 0)
			return 0;
	}

	errno = ERRNO_CAN_EEPROM_WRITE_TIMEOUT;
	return errno;
}

int readSingle (canEeprom_t* eeprom, canSocket_t* socket, uint16_t address, uint8_t count, void* buffer)
{
	struct can_frame commandFrame = readMessageEncode (eeprom, address, count);

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

			if (readMessageParse (eeprom, &response, address, count, buffer) == 0)
				return 0;
		}
	}

	errno = ERRNO_CAN_EEPROM_READ_TIMEOUT;
	return errno;
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

void valueParse (canEepromVariable_t* variable, const char* string, void* buffer)
{
	switch (variable->type)
	{
	case CAN_EEPROM_TYPE_UINT8_T:
		*((uint8_t*) buffer) = strtol (string, NULL, 0);
		break;
	case CAN_EEPROM_TYPE_UINT16_T:
		*((uint16_t*) buffer) = strtol (string, NULL, 0);
		break;
	case CAN_EEPROM_TYPE_UINT32_T:
		*((uint32_t*) buffer) = strtol (string, NULL, 0);
		break;
	case CAN_EEPROM_TYPE_FLOAT:
		*((float*) buffer) = strtof (string, NULL);
		break;
	}
}