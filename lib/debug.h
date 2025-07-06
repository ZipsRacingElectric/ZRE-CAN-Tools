#ifndef DEBUG_H
#define DEBUG_H

// C Standard Library
#include <stdio.h>

#define ERROR_PRINTF(...)						\
	if (errorStream != NULL)					\
		fprintf (errorStream, __VA_ARGS__)

extern FILE* errorStream;

#endif // DEBUG_H