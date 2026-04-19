// For asprintf. Note this must be the first include in this file.
#include <stdint.h>
#define _GNU_SOURCE
#include <stdio.h>

// Header
#include "misc_port.h"

// Includes
#include "debug.h"

// C Standard Library
#include <stdarg.h>
#include <stdlib.h>
#include <string.h>

// POSIX
#include <libgen.h>
#include <sys/stat.h>
#include <unistd.h>
#include <dirent.h>
#include <sys/sysinfo.h>

#ifdef ZRE_CANTOOLS_OS_linux
#include <sys/vfs.h>
#endif // ZRE_CANTOOLS_OS_linux

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

int systemf (char* format, ...)
{
	// Expand the format string into a dynamically-allocated buffer.
	char* command;
	va_list args;
	va_start (args, format);
	int code = vasprintf (&command, format, args);
	va_end (args);

	// If the allocation failed, return error.
	if (code < 0)
		return -1;

	// Execute the command
	debugPrintf ("Executing command '%s'...\n", command);
	code = system (command);
	debugPrintf ("Return code = %i.\n", code);

	// Free the allocated memory
	free (command);

	// Return the error code, as specified by system.
	return code;
}

char* getBaseName (char* path)
{
	char* pathCopy = strdup (path);
	if (pathCopy == NULL)
		return NULL;

	char* dirName = basename (pathCopy);
	if (dirName == NULL)
	{
		free (pathCopy);
		return NULL;
	}

	char* dirNameCopy = strdup (dirName);
	free (pathCopy);
	return dirNameCopy;
}

int getStorageUtilization (size_t* storageUsed, size_t* storageTotal, char* dir)
{
	// Checks that the OS defined is not Windows
	#ifdef ZRE_CANTOOLS_OS_windows
	errno = ERRNO_OS_NOT_SUPPORTED;
	return -1;
	#endif

	struct statfs statfsBuffer;

	if (statfs (dir, &statfsBuffer) != 0)
		return -1;

	// used storage = (the total amount of blocks - free blocks) * block size
	(*storageUsed) = (statfsBuffer.f_blocks - statfsBuffer.f_bfree) * statfsBuffer.f_bsize;

	// total storage = the total amount of blocks * block size
	(*storageTotal) = statfsBuffer.f_blocks * statfsBuffer.f_bsize;

	return 0;
}

int getMemoryUtilization (size_t* memoryUsed, size_t* memoryTotal)
{

	// Checks that the OS defined is not Windows
	#ifdef ZRE_CANTOOLS_OS_windows
	errno = ERRNO_OS_NOT_SUPPORTED;
	return -1;
	#endif

	struct sysinfo info;

	if (sysinfo (&info) != 0)
		return -1;

	(*memoryUsed) = info.totalram - info.freeram;

	(*memoryTotal) = info.totalram;

	return 0;
}

int getCpuUtilization (size_t* cpuUsed, size_t* cpuTotal)
{
	// Checks that the OS defined is not Windows
	#ifdef ZRE_CANTOOLS_OS_windows
	errno = ERRNO_OS_NOT_SUPPORTED;
	return -1;
	#endif

	// Reads file providing a real-time snapshot of CPU usage since the system boot
	FILE* file = fopen ("/proc/stat", "r");
	if (file == NULL)
		return -1;

	size_t cpuInfo [10];

	fscanf (file, "cpu  %zu %zu %zu %zu %zu %zu %zu %zu %zu %zu",
	&cpuInfo[0], &cpuInfo[1], &cpuInfo[2], &cpuInfo[3], &cpuInfo[4],
	&cpuInfo[5], &cpuInfo[6], &cpuInfo[7], &cpuInfo[8], &cpuInfo[9]);

	fclose (file);

	// The amount of idling CPU time
	size_t idle = cpuInfo[3];

	// The amount of idling CPU time waiting for I/O
	size_t iowait = cpuInfo[4];

	// The total amount of idling CPU time
	size_t idleTime = idle + iowait;

	// The total amount of CPU time
	for (size_t i = 0; i < 10; ++i)
		(*cpuTotal) += cpuInfo[i];

	// The amount of CPU used time
	(*cpuUsed) = (*cpuTotal) - idleTime;

	return 0;
}

int getCpuTemperature (size_t* temp)
{
	// Checks that the OS defined is not Windows
	#ifdef ZRE_CANTOOLS_OS_windows
	errno = ERRNO_OS_NOT_SUPPORTED;
	return -1;
	#endif

	FILE* file;
	char type [] = "######";
	char path [] = "/sys/class/thermal/#############/####";

	struct dirent* direntStruct;
	DIR* directory = opendir ("/sys/class/thermal");

	if (directory)

		// Enumerates the files in the "/sys/class/thermal" directory
		while ((direntStruct = readdir (directory)) != NULL)

			// Checks that the file in the directory has the "thermal_zone" prefix
			// (Note: there can be more than one "thermal_zone" depending on the device)
			if (strstr (direntStruct->d_name, "thermal_zone"))
			{

				// Opens & reads the content of the "thermal_zone" file
				snprintf (path, sizeof (path), "/sys/class/thermal/%s/type", direntStruct->d_name);

				file = fopen (path, "r");
				if (file == NULL) return -1;

				fscanf (file, "%s", type);
				fclose (file);

				// Indicates that the "thermal_zone" monitors the CPU socket
				if (strcmp(type, "acpitz") == 0)
				{

					// Opens & reads the content of the "temp" (temperature) file
					snprintf (path, sizeof (path), "/sys/class/thermal/%s/temp", direntStruct->d_name);

					file = fopen (path, "r");
					if (file == NULL) return -1;

					fscanf (file, "%zu", temp);
					fclose (file);

					closedir (directory);
					return 0;
				}
			}

	closedir (directory);

	// Indicates that the temperature was not found
	return -1;
}