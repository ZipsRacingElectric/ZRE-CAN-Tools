// Header
#include "can_eeprom.h"
#include "cjson_util.h"

// cJSON
#include <cjson.h>

// C Standard Library
#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <sys/time.h>

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

bool readMessageParse (canEeprom_t* eeprom, struct can_frame* frame, uint16_t variableIndex, void* data);

struct can_frame validateMessageEncode (canEeprom_t* eeprom, bool isValid);

struct can_frame isValidMessageEncode (canEeprom_t* eeprom);

bool isValidMessageParse (canEeprom_t* eeprom, struct can_frame* frame, bool* isValid);

// Functions ------------------------------------------------------------------------------------------------------------------

bool canEepromInit (canEeprom_t* eeprom, cJSON* json)
{
	// TODO(Barach): Read functions

	if (!jsonGetString (json, "name", &eeprom->name))
		return false;

	if (!jsonGetUint16_t (json, "canAddress", &eeprom->canAddress))
		return false;

	cJSON* variableMap;
	if (!jsonGetObject (json, "variableMap", &variableMap))
		return false;
	eeprom->variableCount = cJSON_GetArraySize (variableMap);
	eeprom->variables = malloc (eeprom->variableCount * sizeof (canEepromVariable_t));

	for (uint16_t index = 0; index < eeprom->variableCount; ++index)
	{
		cJSON* element = cJSON_GetArrayItem (variableMap, index);
		if (!jsonGetUint16_t (element, "address", &(eeprom->variables [index].address)))
			return false;

		if (!jsonGetString (element, "name", &(eeprom->variables [index].name)))
			return false;

		char* variableType;
		if (!jsonGetString (element, "type", &variableType))
			return false;

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
			fprintf (stderr, "Failed to load CAN EEPROM from JSON: Unknown type '%s'.\n", variableType);
			return false;
		}
	}

	return true;
}

bool canEepromWrite (canEeprom_t* eeprom, canSocket_t* socket, uint16_t variableIndex, void* data)
{
	struct can_frame commandFrame = writeMessageEncode (eeprom, variableIndex, data);

	uint8_t readData [4];
	for (uint8_t attempt = 0; attempt < RESPONSE_ATTEMPT_COUNT; ++attempt)
	{
		uint8_t size = VARIABLE_SIZES [eeprom->variables [variableIndex].type];
		canSocketTransmit (socket, &commandFrame);
		if (!canEepromRead (eeprom, socket, variableIndex, readData))
		{
			printf ("Failed to write to EEPROM: Read operation failed.\n");
			return false;
		}

		if (memcmp (data, readData, size) == 0)
			return true;
	}

	printf ("Failed to write to EEPROM: Did not read correct value back.\n");
	return false;
}

bool canEepromRead (canEeprom_t* eeprom, canSocket_t* socket, uint16_t variableIndex, void* data)
{
	struct can_frame commandFrame = readMessageEncode (eeprom, variableIndex);

	for (uint8_t attempt = 0; attempt < RESPONSE_ATTEMPT_COUNT; ++attempt)
	{
		if (!canSocketTransmit (socket, &commandFrame))
			return false;

		struct timeval deadline;
		struct timeval timeCurrent;
		gettimeofday (&timeCurrent, NULL);
		timeradd (&timeCurrent, &RESPONSE_ATTEMPT_TIMEOUT, &deadline);

		while (timercmp (&timeCurrent, &deadline, <))
		{
			gettimeofday (&timeCurrent, NULL);

			struct can_frame response;
			if (!canSocketReceive (socket, &response))
				continue;

			if (readMessageParse (eeprom, &response, variableIndex, data))
				return true;
		}
	}

	printf ("Failed to read from EEPROM: No response received.\n");
	return false;
}

bool canEepromValidate (canEeprom_t* eeprom, canSocket_t* socket, bool isValid)
{
	struct can_frame commandFrame = validateMessageEncode (eeprom, isValid);

	bool readIsValid;
	for (uint8_t attempt = 0; attempt < RESPONSE_ATTEMPT_COUNT; ++attempt)
	{
		canSocketTransmit (socket, &commandFrame);
		if (!canEepromIsValid (eeprom, socket, &readIsValid))
		{
			printf ("Failed to write EEPROM validity: Read operation failed.\n");
			return false;
		}

		if (readIsValid == isValid)
			return true;
	}

	printf ("Failed to write to EEPROM: Did not read correct value back.\n");
	return false;
}

bool canEepromIsValid (canEeprom_t* eeprom, canSocket_t* socket, bool* isValid)
{
	struct can_frame commandFrame = isValidMessageEncode (eeprom);

	for (uint8_t attempt = 0; attempt < RESPONSE_ATTEMPT_COUNT; ++attempt)
	{
		if (!canSocketTransmit (socket, &commandFrame))
			return false;

		struct timeval deadline;
		struct timeval timeCurrent;
		gettimeofday (&timeCurrent, NULL);
		timeradd (&timeCurrent, &RESPONSE_ATTEMPT_TIMEOUT, &deadline);

		while (timercmp (&timeCurrent, &deadline, <))
		{
			gettimeofday (&timeCurrent, NULL);

			struct can_frame response;
			if (!canSocketReceive (socket, &response))
				continue;

			if (isValidMessageParse (eeprom, &response, isValid))
				return true;
		}
	}

	printf ("Failed to check EEPROM validity: No response received.\n");
	return false;
}

// Standard I/O ---------------------------------------------------------------------------------------------------------------

uint16_t canEepromVariablePrompt (canEeprom_t* eeprom)
{
	char buffer [256];

	while (true)
	{
		printf ("Enter the key of the variable: ");
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

	printf ("Enter the value of the variable (%s): ", VARIABLE_NAMES [variable->type]);

	switch (variable->type)
	{
	case CAN_EEPROM_TYPE_UINT8_T:
		fscanf (stdin, "%u%*1[\n]", (uint32_t*) data);
		break;
	case CAN_EEPROM_TYPE_UINT16_T:
		fscanf (stdin, "%u%*1[\n]", (uint32_t*) data);
		break;
	case CAN_EEPROM_TYPE_UINT32_T:
		fscanf (stdin, "%u%*1[\n]", (uint32_t*) data);
		break;
	case CAN_EEPROM_TYPE_FLOAT:
		fscanf (stdin, "%f%*1[\n]", (float*) data);
		break;
	}
}

void canEepromPrintVariable (canEeprom_t* eeprom, uint16_t variableIndex, void* data)
{
	canEepromVariable_t* variable = eeprom->variables + variableIndex;
	printf ("%s (%s): ", variable->name, VARIABLE_NAMES [variable->type]);

	switch (variable->type)
	{
	case CAN_EEPROM_TYPE_UINT8_T:
		printf ("%u\n", *((uint8_t*) data));
		break;
	case CAN_EEPROM_TYPE_UINT16_T:
		printf ("%u\n", *((uint16_t*) data));
		break;
	case CAN_EEPROM_TYPE_UINT32_T:
		printf ("%u\n", *((uint32_t*) data));
		break;
	case CAN_EEPROM_TYPE_FLOAT:
		printf ("%f\n", *((float*) data));
		break;
	}
}

void canEepromPrintMap (canEeprom_t* eeprom, canSocket_t* socket)
{
	bool isValid;
	if (!canEepromIsValid (eeprom, socket, &isValid))
		return;

	printf ("%s Memory Map: %s\n", eeprom->name, isValid ? "Valid" : "Invalid");
	printf ("---------------------------------------------------\n");
	printf ("%24s | %10s | %10s\n", "Variable", "Type", "Value");
	printf ("-------------------------|------------|------------\n");

	for (uint16_t index = 0; index < eeprom->variableCount; ++index)
	{
		uint8_t data [4];
		if (!canEepromRead (eeprom, socket, index, data))
			return;

		canEepromVariable_t* variable = eeprom->variables + index;
		printf ("%24s | %10s | ", variable->name, VARIABLE_NAMES [variable->type]);

		switch (variable->type)
		{
		case CAN_EEPROM_TYPE_UINT8_T:
			printf ("%10u\n", *((uint8_t*) data));
			break;
		case CAN_EEPROM_TYPE_UINT16_T:
			printf ("%10u\n", *((uint16_t*) data));
			break;
		case CAN_EEPROM_TYPE_UINT32_T:
			printf ("%10u\n", *((uint32_t*) data));
			break;
		case CAN_EEPROM_TYPE_FLOAT:
			printf ("%10f\n", *((float*) data));
			break;
		}
	}

	printf ("\n");
}

void canEepromPrintEmptyMap (canEeprom_t* eeprom)
{
	printf ("%s Memory Map:\n", eeprom->name);
	printf ("-----------------------------------------------------\n");
	printf ("%24s | %10s | %10s\n", "Variable", "Type", "Address");
	printf ("-------------------------|------------|--------------\n");

	for (uint16_t index = 0; index < eeprom->variableCount; ++index)
	{
		canEepromVariable_t* variable = eeprom->variables + index;
		printf ("%24s | %10s |       0x%04X\n", variable->name, VARIABLE_NAMES [variable->type], variable->address);
	}

	printf ("\n");
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

bool readMessageParse (canEeprom_t* eeprom, struct can_frame* frame, uint16_t variableIndex, void* data)
{
	canEepromVariable_t* variable = eeprom->variables + variableIndex;

	uint16_t instruction = frame->data [0] | (frame->data [1] << 8);

	// Check message ID is correct
	if (frame->can_id != (uint16_t) (eeprom->canAddress + 1))
		return false;

	// Check message is a read response
	if (!EEPROM_RESPONSE_MESSAGE_READ_NOT_WRITE (instruction))
	{
		fprintf (stderr, "Warning: Received EEPROM response with invalid read/write flag.\n");
		return false;
	}

	// Check message is a data response
	if (!EEPROM_RESPONSE_MESSAGE_DATA_NOT_VALIDATION (instruction))
	{
		fprintf (stderr, "Warning: Received EEPROM response with invalid data/validation flag.\n");
		return false;
	}

	uint16_t address = frame->data [2] | (frame->data [3] << 8);

	// Check message is the correct address
	if (address != variable->address)
	{
		fprintf (stderr, "Warning: Received EEPROM response with invalid data address. Expected '0x%X', received '0x%X'.\n",
			variable->address, address);
		return false;
	}

	// Check data count is correct
	uint8_t variableSize = VARIABLE_SIZES [variable->type];
	uint8_t dataCount = EEPROM_RESPONSE_MESSAGE_DATA_COUNT (instruction);
	if (dataCount != variableSize)
	{
		fprintf (stderr, "Warning: Received EEPROM response with invalid data count. Expected '%u', received '%u'\n",
			variableSize, dataCount);
		return false;
	}

	// Success, copy the data into the buffer
	memcpy (data, frame->data + 4, variableSize);
	return true;
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

bool isValidMessageParse (canEeprom_t* eeprom, struct can_frame* frame, bool* isValid)
{
	uint16_t instruction = frame->data [0] | (frame->data [1] << 8);

	// Check message ID is correct
	if (frame->can_id != (uint16_t) (eeprom->canAddress + 1))
		return false;

	// Check message is a read response
	if (!EEPROM_RESPONSE_MESSAGE_READ_NOT_WRITE (instruction))
	{
		fprintf (stderr, "Warning: Received EEPROM response with invalid read/write flag.\n");
		return false;
	}

	// Check message is a validation response
	if (EEPROM_RESPONSE_MESSAGE_DATA_NOT_VALIDATION (instruction))
	{
		fprintf (stderr, "Warning: Received EEPROM response with invalid data/validation flag.\n");
		return false;
	}

	// Success, read the isValid flag
	*isValid = EEPROM_RESPONSE_MESSAGE_IS_VALID (instruction);
	return true;
}