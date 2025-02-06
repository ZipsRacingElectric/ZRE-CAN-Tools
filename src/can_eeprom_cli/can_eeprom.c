// Header
#include "can_eeprom.h"

// Includes
#include "error_codes.h"
#include "can_eeprom_operations.h"

// C Standard Library
#include <errno.h>
#include <stdbool.h>

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

static const char* VARIABLE_TYPE_NAMES [] =
{
	"uint8_t",
	"uint16_t",
	"uint32_t",
	"float"
};

// Function Prototypes --------------------------------------------------------------------------------------------------------

/// @brief Parses a string to get a primative variable's value (single element only, not for matrices / arrays).
void parsePrimativeValue (canEepromVariable_t* variable, const char* string, void* buffer);

/// @brief Prints a primative variable's value to an output stream (single element only, not for matrices / arrays).
void printPrimativeValue (canEepromVariable_t* variable, void* buffer, FILE* stream);

/// @brief Prints the datatype and indices of an element of a matrix/array variable.
void printVariableIndex (canEepromVariable_t* variable, FILE* stream, uint16_t x, uint16_t y);

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

		// Check whether the variable is a matrix.
		if (jsonGetUint16_t (element, "width", &variable->width) == 0)
		{
			// If no height is specified, treat it as an array.
			if (jsonGetUint16_t (element, "height", &variable->height) != 0)
				variable->height = 1;
		}
		else
		{
			variable->width = 1;
			variable->height = 1;
		}
	}

	eeprom->buffer = canEepromAllocateBuffer (eeprom);
	return (eeprom->buffer != NULL) ? 0 : errno;
}

int canEepromWriteVariable (canEeprom_t* eeprom, canSocket_t* socket, canEepromVariable_t* variable, void* buffer)
{
	uint16_t count = VARIABLE_SIZES [variable->type] * variable->width * variable->height;
	return canEepromWrite (eeprom->canId, socket, variable->address, count, buffer);
}

int canEepromReadVariable (canEeprom_t* eeprom, canSocket_t* socket, canEepromVariable_t* variable, void* buffer)
{
	uint16_t count = VARIABLE_SIZES [variable->type] * variable->width * variable->height;
	return canEepromRead (eeprom->canId, socket, variable->address, count, buffer);
}

int canEepromWriteValid (canEeprom_t* eeprom, canSocket_t* socket, bool isValid)
{
	return canEepromValidate (eeprom->canId, socket, isValid);
}

int canEepromReadValid (canEeprom_t* eeprom, canSocket_t* socket, bool* isValid)
{
	return canEepromIsValid (eeprom->canId, socket, isValid);
}

int canEepromWriteJson (canEeprom_t* eeprom, canSocket_t* socket, cJSON* dataJson)
{
	size_t keyCount = cJSON_GetArraySize (dataJson);

	// Traverse every key in the data JSON and match it to the EEPROM variable
	for (size_t keyIndex = 0; keyIndex < keyCount; ++keyIndex)
	{
		cJSON* key = cJSON_GetArrayItem (dataJson, keyIndex);

		// Search for matching variable
		uint16_t variableIndex = 0;
		for (; variableIndex < eeprom->variableCount; ++variableIndex)
		{
			canEepromVariable_t* variable = eeprom->variables + variableIndex;
			void* buffer = eeprom->buffer;

			// Skip until the correct variable is found
			if (strcmp (key->string, variable->name) != 0)
				continue;

			// If the variable is a matrix, validate the height is correct
			if (variable->height != 1 && cJSON_GetArraySize (key) != variable->height)
			{
				errno = ERRNO_CAN_EEPROM_BAD_DIMENSION;
				return errno;
			}

			// Traverse the Y-dimension of the matrix
			for (uint16_t y = 0; y < variable->height; ++y)
			{
				// If the variable is a matrix, get the y'th element. Otherwise use the top level.
				cJSON* xDimension = key;
				if (variable->height != 1)
					xDimension = cJSON_GetArrayItem (key, y);

				// If the variable is a matrix or an array, validate the width is correct.
				if (variable->width != 1 && cJSON_GetArraySize (xDimension) != variable->width)
				{
					errno = ERRNO_CAN_EEPROM_BAD_DIMENSION;
					return errno;
				}

				// Traverse the X-dimension of the array/matrix
				for (uint16_t x = 0; x < variable->width; ++x)
				{
					// If the variable is an array/matrix, get the x'th element. Otherwise use the top level.
					cJSON* value = xDimension;
					if (variable->width != 1)
						value = cJSON_GetArrayItem (xDimension, x);

					// Parse the element's value as a string
					char* string = cJSON_GetStringValue (value);
					if (string == NULL)
					{
						errno = ERRNO_CAN_EEPROM_BAD_VALUE;
						return errno;
					}

					// Parse the value of the element
					// TODO(Barach): This method forces the user to use strings for everything. What a pain.
					parsePrimativeValue (variable, string, buffer);

					// Move to the next position in the buffer in case this is an array/matrix.
					buffer = buffer + VARIABLE_SIZES [variable->type];
				}
			}

			// After the buffer has been filled, write the data to the EEPROM.
			if (canEepromWriteVariable (eeprom, socket, variable, eeprom->buffer) != 0)
				return errno;

			break;
		}

		// If the key was not matched to a variable, exit early.
		if (variableIndex == eeprom->variableCount)
		{
			DEBUG_PRINTF ("canEepromProgram failed: Unknown key '%s'.\n", key->string);
			errno = ERRNO_CAN_EEPROM_BAD_KEY;
			return errno;
		}
	}

	// Success
	return 0;
}

int canEepromReadJson (canEeprom_t* eeprom, canSocket_t* socket, FILE* stream)
{
	// Start of JSON
	fprintf (stream, "{\n");

	// Traverse each variable
	for (uint16_t index = 0; index < eeprom->variableCount; ++index)
	{
		// Read the variable's value
		canEepromVariable_t* variable = eeprom->variables + index;
		void* buffer = eeprom->buffer;
		if (canEepromReadVariable (eeprom, socket, variable, buffer) != 0)
			return errno;

		// Print variable key
		fprintf (stream, "\t\"%s\": ", variable->name);

		// Start of matrix (if matrix type)
		if (variable->height != 1)
			fprintf (stream, "\n\t[\n\t\t");

		// Matrix Y-dimension
		for (uint16_t y = 0; y < variable->height; ++y)
		{
			// Start of matrix / array (if matrix / array type)
			if (variable->width != 1)
				fprintf (stream, "[");

			// Matrix / array X-dimension
			for (uint16_t x = 0; x < variable->width; ++x)
			{
				// Element value
				fprintf (stream, "\"");
				printPrimativeValue (variable, buffer, stream);
				fprintf (stream, "\"");
				buffer += VARIABLE_SIZES [variable->type];

				// Matrix / array separators
				if (x != variable->width - 1)
					fprintf (stream, ", ");

			}

			// End of matrix / array
			if (variable->width != 1)
				fprintf (stream, "]");

			// Matrix separators
			if (y != variable->height - 1)
				fprintf (stream, ",\n\t\t");
		}

		// End of matrix
		if (variable->height != 1)
			fprintf (stream, "\n\t]");

		// JSON separator
		if (index != eeprom->variableCount - 1)
			fprintf (stream, ",\n");
	}

	// End of JSON
	fprintf (stream, "\n}\n");

	return 0;
}

void* canEepromAllocateBuffer (canEeprom_t* eeprom)
{
	uint16_t bufferSize = 0;

	for (uint16_t index = 0; index < eeprom->variableCount; ++index)
	{
		canEepromVariable_t* variable = eeprom->variables + index;
		uint16_t size = VARIABLE_SIZES [variable->type] * variable->width * variable->height;
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
			fprintf (streamOut, "Enter the name of the variable: ");
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
	// TODO(Barach): This is incredibly impractical. In reality, this should prompt the user for a single element, not the
	// entire matrix. Doing this doesn't really fit the current model though, so for now it's this.

	for (uint16_t y = 0; y < variable->height; ++y)
	{
		for (uint16_t x = 0; x < variable->width; ++x)
		{
			// Prompt the user
			if (streamOut != NULL)
			{
				fprintf (streamOut, "Enter the value (");
				printVariableIndex (variable, streamOut, x, y);
				fprintf (streamOut, "): ");
			}

			switch (variable->type)
			{
			// TODO(Barach): This writes beyond the size of uint8_t and uint16_t
			case CAN_EEPROM_TYPE_UINT8_T:
			case CAN_EEPROM_TYPE_UINT16_T:
			case CAN_EEPROM_TYPE_UINT32_T:
				fscanf (streamIn, "%u%*1[\n]", (uint32_t*) buffer);
				break;
			case CAN_EEPROM_TYPE_FLOAT:
				fscanf (streamIn, "%f%*1[\n]", (float*) buffer);
				break;
			}

			buffer += VARIABLE_SIZES [variable->type];
		}
	}
}

void canEepromPrintVariableValue (canEepromVariable_t* variable, void* buffer, const char* indent, FILE* stream)
{
	for (uint16_t y = 0; y < variable->height; ++y)
	{
		for (uint16_t x = 0; x < variable->width; ++x)
		{
			printPrimativeValue (variable, buffer, stream);
			buffer += VARIABLE_SIZES [variable->type];

			if (x != variable->width - 1)
				fprintf (stream, ", ");
		}

		if (y != variable->height - 1)
			fprintf (stream, "\n%s", indent);
	}

	fprintf (stream, "\n");
}

int canEepromPrintMap (canEeprom_t* eeprom, canSocket_t* socket, FILE* stream)
{
	bool isValid;
	if (canEepromIsValid (eeprom->canId, socket, &isValid) != 0)
		return errno;

	fprintf (stream, "%s Memory Map: %s\n", eeprom->name, isValid ? "Valid" : "Invalid");
	fprintf (stream, "----------------------------------------------------------------\n");
	fprintf (stream, "%24s | %23s | %10s\n", "Variable", "Type", "Value");
	fprintf (stream, "-------------------------|-------------------------|------------\n");

	for (uint16_t index = 0; index < eeprom->variableCount; ++index)
	{
		canEepromVariable_t* variable = eeprom->variables + index;

		if (canEepromReadVariable (eeprom, socket, variable, eeprom->buffer) != 0)
			return errno;

		fprintf (stream, "%24s | ", variable->name);
		if (variable->height != 1)
			fprintf (stream, "%10s [%4u][%4u]", VARIABLE_TYPE_NAMES [variable->type], variable->height, variable->width);
		else if (variable->width != 1)
			fprintf (stream, "%16s [%4u]", VARIABLE_TYPE_NAMES [variable->type], variable->width);
		else
			fprintf (stream, "%23s", VARIABLE_TYPE_NAMES [variable->type]);
		fprintf (stream, " | ");
		canEepromPrintVariableValue (variable, eeprom->buffer,
			"                         |                         | ", stream);
	}

	fprintf (stream, "\n");

	return 0;
}

void canEepromPrintEmptyMap (canEeprom_t* eeprom, FILE* stream)
{
	fprintf (stream, "%s Memory Map:\n", eeprom->name);
	fprintf (stream, "------------------------------------------------------------------\n");
	fprintf (stream, "%24s | %23s | %10s\n", "Variable", "Type", "Address");
	fprintf (stream, "-------------------------|-------------------------|--------------\n");

	for (uint16_t index = 0; index < eeprom->variableCount; ++index)
	{
		canEepromVariable_t* variable = eeprom->variables + index;
		fprintf (stream, "%24s | ", variable->name);
		if (variable->height != 1)
			fprintf (stream, "%10s [%4u][%4u]", VARIABLE_TYPE_NAMES [variable->type], variable->height, variable->width);
		else if (variable->width != 1)
			fprintf (stream, "%16s [%4u]", VARIABLE_TYPE_NAMES [variable->type], variable->width);
		else
			fprintf (stream, "%23s", VARIABLE_TYPE_NAMES [variable->type]);
		fprintf (stream, " | 0x%04X\n", variable->address);
	}

	fprintf (stream, "\n");
}

// Private Functions ----------------------------------------------------------------------------------------------------------

void parsePrimativeValue (canEepromVariable_t* variable, const char* string, void* buffer)
{
	// TODO(Barach): Does this write stop at fixed-widths less than 32?
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

void printPrimativeValue (canEepromVariable_t* variable, void* buffer, FILE* stream)
{
	// TODO(Barach): Does this read stop at fixed-widths less than 32?

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
		fprintf (stream, "%.3f", *((float*) buffer));
		break;
	}
}

void printVariableIndex (canEepromVariable_t* variable, FILE* stream, uint16_t x, uint16_t y)
{
	if (variable->height != 1)
		fprintf (stream, "%s [%u][%u]", VARIABLE_TYPE_NAMES [variable->type], y, x);
	else if (variable->width != 1)
		fprintf (stream, "%s [%u]", VARIABLE_TYPE_NAMES [variable->type], x);
	else
		fprintf (stream, "%s", VARIABLE_TYPE_NAMES [variable->type]);
}