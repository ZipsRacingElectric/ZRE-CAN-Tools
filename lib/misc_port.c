// Header
#include "misc_port.h"

// Includes
#include "debug.h"

// C Standard Library
#include <stdlib.h>
#include <string.h>

// POSIX
#include <sys/stat.h>
#include <unistd.h>

char* expandEnv (const char* str)
{
	size_t variablePosition = strcspn (str, "$");
	size_t originalPathLength = strlen (str);

	// Check for a variable to expand. If none, simply duplicate the existing string.
	if (variablePosition == originalPathLength)
		return strdup (str);

	// Calculate the length of the variable name
	size_t variableLength = 1;
	while (variablePosition + variableLength != originalPathLength)
	{
		char c = str [variablePosition + variableLength];
		if (c == '$' || c == '/' || c == ' ')
			break;
		++variableLength;
	}

	// Copy the variable name into a buffer (+1 for terminator, -1 to exclude '$')
	char* variable = malloc (variableLength);
	if (variable == NULL)
		return NULL;
	strncpy (variable, str + variablePosition + 1, variableLength - 1);
	variable [variableLength - 1] = '\0';

	// Get the value of the environment variable
	size_t valueLength = 0;
	char* value = getenv (variable);
	if (value != NULL)
		valueLength = strlen (value);

	// Create a buffer for the expanded string
	size_t extendedPathLength = originalPathLength - variableLength + valueLength;
	char* extendedPath = malloc (extendedPathLength + 1);
	if (extendedPath == NULL)
	{
		free (variable);
		return NULL;
	}

	// Populate the extended path
	memcpy (extendedPath, str, variablePosition);
	memcpy (extendedPath + variablePosition, value, valueLength);
	memcpy (extendedPath + variablePosition + valueLength, str + variablePosition + variableLength, originalPathLength - variablePosition - variableLength);
	extendedPath [extendedPathLength] = '\0';

	debugPrintf ("Substituting environment variable '%s' with '%s' yields '%s'.\n", variable, value, extendedPath);

	free (variable);
	return extendedPath;
}

int mkdirPort (const char* path)
{
	#ifdef ZRE_CANTOOLS_OS_windows
	return mkdir (path);
	#else
	return mkdir (path, S_IRWXU | S_IRGRP | S_IROTH);
	#endif
}

int fsyncPort (FILE* file)
{
	#ifdef ZRE_CANTOOLS_OS_linux

	int fd = fileno (file);
	if (fd < 0)
		return -1;

	return fsync (fd);

	#else // ZRE_CANTOOLS_OS_linux

	(void) file;

	// Not aware of, nor concerned with a Windows equivalent.
	return 0;

	#endif // ZRE_CANTOOLS_OS_linux
}