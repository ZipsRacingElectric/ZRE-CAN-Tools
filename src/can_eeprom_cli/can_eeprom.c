// Header
#include "can_eeprom.h"

// Includes
#include "error_codes.h"
#include "can_eeprom_operations.h"

// C Standard Library
#include <errno.h>

// Macros ---------------------------------------------------------------------------------------------------------------------

#if CAN_EEPROM_DEBUG
	#include <stdio.h>
	#define DEBUG_PRINTF(...) fprintf (stderr, __VA_ARGS__)
#else
	#define DEBUG_PRINTF(...) while (false)
#endif // CAN_DATABASE_DEBUG

// Constants ------------------------------------------------------------------------------------------------------------------

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

// Function Prototypes --------------------------------------------------------------------------------------------------------

/// @brief Parses a string to get a variable's value.
void valueParse (canEepromVariable_t* variable, const char* string, void* buffer);

// Functions ------------------------------------------------------------------------------------------------------------------

int canEepromInit (canEeprom_t* eeprom, cJSON* configJson)
{
	if (jsonGetString (configJson, "name", &eeprom->name) != 0)
		return errno;

	if (jsonGetUint16_t (configJson, "canId", &eeprom->canId) != 0)
		return errno;

	cJSON* variables;
	if (jsonGetObject (configJson, "variables", &variables) != 0)
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

	eeprom->buffer = canEepromAllocateBuffer (eeprom);
	return (eeprom->buffer != NULL) ? 0 : errno;
}

int canEepromWriteVariable (canEeprom_t* eeprom, canSocket_t* socket, canEepromVariable_t* variable, void* buffer)
{
	return canEepromWrite (eeprom->canId, socket, variable->address, VARIABLE_SIZES [variable->type], buffer);
}

int canEepromReadVariable (canEeprom_t* eeprom, canSocket_t* socket, canEepromVariable_t* variable, void* buffer)
{
	return canEepromRead (eeprom->canId, socket, variable->address, VARIABLE_SIZES [variable->type], buffer);
}

int canEepromWriteValid (canEeprom_t* eeprom, canSocket_t* socket, bool isValid)
{
	return canEepromValidate (eeprom->canId, socket, isValid);
}

int canEepromReadValid (canEeprom_t* eeprom, canSocket_t* socket, bool* isValid)
{
	canEepromIsValid (eeprom->canId, socket, isValid);
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

			valueParse (variable, string, eeprom->buffer);

			if (canEepromWriteVariable (eeprom, socket, variable, eeprom->buffer) != 0)
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

		if (canEepromReadVariable (eeprom, socket, variable, eeprom->buffer) != 0)
			return errno;

		if (index == 0)
			fprintf (stream, "{\n");

		fprintf (stream, "\t\"%s\": ", variable->name);

		switch (variable->type)
		{
		case CAN_EEPROM_TYPE_UINT8_T:
			fprintf (stream, "\"%u\"", *((uint8_t*) eeprom->buffer));
			break;
		case CAN_EEPROM_TYPE_UINT16_T:
			fprintf (stream, "\"%u\"", *((uint16_t*) eeprom->buffer));
			break;
		case CAN_EEPROM_TYPE_UINT32_T:
			fprintf (stream, "\"%u\"", *((uint32_t*) eeprom->buffer));
			break;
		case CAN_EEPROM_TYPE_FLOAT:
			fprintf (stream, "\"%f\"", *((float*) eeprom->buffer));
			break;
		}

		if (index == eeprom->variableCount - 1)
			fprintf (stream, "\n}\n");
		else
			fprintf (stream, ",\n");
	}

	return 0;
}

void* canEepromAllocateBuffer (canEeprom_t* eeprom)
{
	uint16_t bufferSize = 0;

	for (uint16_t index = 0; index < eeprom->variableCount; ++index)
	{
		uint16_t size = VARIABLE_SIZES [eeprom->variables [index].type];
		if (size > bufferSize)
			bufferSize = size;
	}

	return malloc (bufferSize);
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
	if (canEepromIsValid (eeprom->canId, socket, &isValid) != 0)
		return errno;

	fprintf (stream, "%s Memory Map: %s\n", eeprom->name, isValid ? "Valid" : "Invalid");
	fprintf (stream, "---------------------------------------------------\n");
	fprintf (stream, "%24s | %10s | %10s\n", "Variable", "Type", "Value");
	fprintf (stream, "-------------------------|------------|------------\n");

	for (uint16_t index = 0; index < eeprom->variableCount; ++index)
	{
		canEepromVariable_t* variable = eeprom->variables + index;

		if (canEepromReadVariable (eeprom, socket, variable, eeprom->buffer) != 0)
			return errno;

		fprintf (stream, "%24s | %10s | ", variable->name, VARIABLE_NAMES [variable->type]);
		canEepromPrintVariable (variable, eeprom->buffer, stream);
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