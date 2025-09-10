// Header
#include "debug.h"

// C Standard Library
#include <execinfo.h>
#include <signal.h>
#include <stdlib.h>

FILE* errorStream = NULL;

static void segvHandler (int sig)
{
	fprintf (stderr, "\n\nSegmentation Fault Occurred!\n");
	fprintf (stderr, "\n- BEGIN BACKTRACE --------------------------------------------------------------\n\n");

	// Get the program's backtrace.
	void* backtraceArray [1024];
	int backtraceArraySize = backtrace (backtraceArray, sizeof (backtraceArray) / sizeof (void*));

	// Convert the backtrace to "user friendly" symbols.
	char** backtraceSymbols;
	backtraceSymbols = backtrace_symbols (backtraceArray, backtraceArraySize);

	// Print the backtrace to stderr
	// - Start at 1 to skip the call to this function
	for (size_t index = 1; index < backtraceArraySize; ++index)
		printf ("%s\n", backtraceSymbols [index]);

	free (backtraceSymbols);
	fprintf (stderr, "\n- END BACKTRACE ----------------------------------------------------------------\n\n");

	// End the program
	exit (sig);
}

void debugInit ()
{
	signal (SIGSEGV, segvHandler);
}