// Header
#include "misc_port.h"

// POSIX
#include <sys/stat.h>
#include <unistd.h>

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