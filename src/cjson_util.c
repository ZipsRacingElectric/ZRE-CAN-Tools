// Header
#include "cjson_util.h"

// C Standard Library
#include <errno.h>
#include <stdlib.h>
#include <string.h>

// Constants ------------------------------------------------------------------------------------------------------------------

/// @brief The maximum size of a json file to load, in bytes.
#define JSON_SIZE_MAX 8192

// Functions ------------------------------------------------------------------------------------------------------------------

cJSON* jsonLoadFile (const char* path)
{
	// Open the file for reading
	FILE* file = fopen (path, "r");
	if (file == NULL)
	{
		int code = errno;
		fprintf (stderr, "Failed to load JSON file: %s.\n", strerror (code));
		return false;
	}

	// Get the size of the file (including null terminator).
	fseek (file, 0, SEEK_END);
	size_t size = ftell (file) + 1;
	fseek (file, 0, SEEK_SET);

	char* buffer = malloc (size);
	if (buffer == NULL)
	{
		int code = errno;
		fprintf (stderr, "Failed to load JSON file: %s.\n", strerror (code));
		return false;
	}

	// Read the file into a string (appending null-terminator)
	fread (buffer, 1, size, file);
	fclose (file);
	buffer [size - 1] = '\0';

	// Parse the JSON
	return cJSON_Parse (buffer);
}

cJSON* jsonPrompt (FILE* stream)
{
	char buffer [JSON_SIZE_MAX + 1];

	char* head = buffer;

	// Read until the first bracket.
	int bracketCount = 0;
	while (bracketCount == 0)
	{
		int c = fgetc (stream);
		if (c == EOF)
		{
			fprintf (stderr, "Failed to prompt JSON file: Unexpected end of file.");
			return NULL;
		}

		if (c == '{')
		{
			++bracketCount;
			*head = c;
			++head;
		}
	}

	// Read until the last closing bracket is read.
	while (bracketCount > 0)
	{
		int c = fgetc (stream);
		if (c == EOF)
		{
			fprintf (stderr, "Failed to prompt JSON file: Unexpected end of file.");
			return NULL;
		}

		*head = c;
		++head;

		if (c == '{')
			++bracketCount;

		if (c == '}')
			--bracketCount;
	}

	return cJSON_Parse (buffer);
}

bool jsonGetObject (cJSON* json, const char* key, cJSON** value)
{
	cJSON* item = cJSON_GetObjectItem (json, key);
	if (item == NULL)
	{
		printf ("Failed to parse key '%s' from JSON: Missing key.\n", key);
		return false;
	}
	*value = item;
	return true;
}

bool jsonGetString (cJSON* json, const char* key, char** value)
{
	cJSON* item = cJSON_GetObjectItem (json, key);
	if (item == NULL)
	{
		printf ("Failed to parse key '%s' from JSON: Missing key.\n", key);
		return false;
	}
	*value = cJSON_GetStringValue (item);
	return true;
}

bool jsonGetUint16_t (cJSON* json, const char* key, uint16_t* value)
{
	cJSON* item = cJSON_GetObjectItem (json, key);
	if (item == NULL)
	{
		printf ("Failed to parse key '%s' from JSON: Missing key.\n", key);
		return false;
	}
	*value = strtol (cJSON_GetStringValue (item), NULL, 0);
	return true;
}