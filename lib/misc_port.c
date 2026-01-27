// Header
#include "misc_port.h"

// POSIX
#include <sys/stat.h>

int mkdirPort (const char* path)
{
	#ifdef ZRE_CANTOOLS_OS_windows
	return mkdir (path);
	#else
	return mkdir (path, S_IRWXU | S_IRGRP | S_IROTH);
	#endif
}