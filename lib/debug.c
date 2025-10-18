// Header
#include "debug.h"

// C Standard Library
#include <signal.h>
#include <stdlib.h>

#if __unix__
#include <execinfo.h>
#endif // __unix__

FILE* errorStream = NULL;

// TODO(Barach): Figure this out for windows.
#if __unix__

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
	for (int index = 1; index < backtraceArraySize; ++index)
		fprintf (stderr, "%s\n", backtraceSymbols [index]);

	free (backtraceSymbols);

	fprintf (stderr, "\n- END BACKTRACE ----------------------------------------------------------------\n\n");


	// End the program
	exit (sig);
}

#endif // __unix__

void debugInit ()
{
	#if __unix__
	signal (SIGSEGV, segvHandler);
	#endif // __unix__
}