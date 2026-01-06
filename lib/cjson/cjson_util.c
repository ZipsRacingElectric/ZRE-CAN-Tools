// Header
#include "cjson_util.h"

// Includes
#include "debug.h"
#include "error_codes.h"

// C Standard Library
#include <errno.h>
#include <stdlib.h>
#include <string.h>

// Constants ------------------------------------------------------------------------------------------------------------------

/// @brief The maximum size of a json file to load, in bytes.
#define JSON_SIZE_MAX 16384

// Functions ------------------------------------------------------------------------------------------------------------------

cJSON* jsonLoad (const char* path)
{
	// Open the file
	FILE* file = fopen (path, "r");
	if (file == NULL)
		return NULL;

	// Read the JSON from the file
	cJSON* json = jsonRead (file);

	// Close the file, preserving errno
	int code = errno;
	fclose (file);
	errno = code;

	return json;
}

cJSON* jsonRead (FILE* stream)
{
	char buffer [JSON_SIZE_MAX];
	int c;

	// Read until the first bracket is read.
	do
	{
		c = fgetc (stream);
		if (c == EOF)
		{
			errno = ERRNO_CJSON_EOF;
			return NULL;
		}
	} while (c != '{');

	// First character of the buffer is the opening bracket.
	uint16_t bracketCount = 1;
	buffer [0] = '{';
	char* head = buffer + 1;

	// Read until the last closing bracket is read.
	while (bracketCount > 0)
	{
		c = fgetc (stream);
		if (c == EOF)
		{
			errno = ERRNO_CJSON_EOF;
			return NULL;
		}

		*head = c;
		++head;

		if (head - buffer == JSON_SIZE_MAX)
		{
			errno = ERRNO_CJSON_MAX_SIZE;
			return NULL;
		}

		// Count the number of brackets
		if (c == '{')
			++bracketCount;
		else if (c == '}')
			--bracketCount;
	}
	*head = '\0';

	// Parse the JSON data
	cJSON* json = cJSON_Parse (buffer);
	if (json == NULL)
		errno = ERRNO_CJSON_PARSE_FAIL;
	return json;
}

int jsonGetObject (cJSON* json, const char* key, cJSON** value)
{
	cJSON* item = cJSON_GetObjectItem (json, key);
	if (item == NULL)
	{
		debugPrintf ("JSON key '%s' does not exist.\n", key);
		errno = ERRNO_CJSON_MISSING_KEY;
		return errno;
	}

	*value = item;
	return 0;
}

int jsonGetString (cJSON* json, const char* key, char** value)
{
	cJSON* item = cJSON_GetObjectItem (json, key);
	if (item == NULL)
	{
		debugPrintf ("JSON key '%s' does not exist.\n", key);
		errno = ERRNO_CJSON_MISSING_KEY;
		return errno;
	}

	*value = cJSON_GetStringValue (item);
	return 0;
}

int jsonGetUint16_t (cJSON* json, const char* key, uint16_t* value)
{
	cJSON* item = cJSON_GetObjectItem (json, key);
	if (item == NULL)
	{
		debugPrintf ("JSON key '%s' does not exist.\n", key);
		errno = ERRNO_CJSON_MISSING_KEY;
		return errno;
	}

	*value = strtol (cJSON_GetStringValue (item), NULL, 0);
	return 0;
}

int jsonGetBool (cJSON* json, const char* key, bool* value)
{
	cJSON* item = cJSON_GetObjectItem (json, key);
	if (item == NULL)
	{
		debugPrintf ("JSON key '%s' does not exist.\n", key);
		errno = ERRNO_CJSON_MISSING_KEY;
		return errno;
	}

	char* str = cJSON_GetStringValue (item);
	if (strcmp (str, "true") == 0)
	{
		*value = true;
		return 0;
	}

	if (strcmp (str, "false") == 0)
	{
		*value = false;
		return 0;
	}

	debugPrintf ("Invalid value in boolean JSON '%s'\n", str);
	return ERRNO_CJSON_PARSE_FAIL;
}

int jsonGetFloat (cJSON* json, const char* key, float* value)
{
	cJSON* item = cJSON_GetObjectItem (json, key);
	if (item == NULL)
	{
		debugPrintf ("JSON key '%s' does not exist.\n", key);
		errno = ERRNO_CJSON_MISSING_KEY;
		return errno;
	}

	*value = strtof (cJSON_GetStringValue (item), NULL);
	return 0;
}