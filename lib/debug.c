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

static void printBacktrace ()
{
	fprintf (stderr, "\n- BEGIN BACKTRACE --------------------------------------------------------------\n\n");

	// Get the program's backtrace.
	void* backtraceArray [1024];
	int backtraceArraySize = backtrace (backtraceArray, sizeof (backtraceArray) / sizeof (void*));

	// Convert the backtrace to "user friendly" symbols.
	char** backtraceSymbols;
	backtraceSymbols = backtrace_symbols (backtraceArray, backtraceArraySize);

	// Print the backtrace to stderr
	// - Start at 2 to skip the call to this function and the signal handler
	for (int index = 2; index < backtraceArraySize; ++index)
		fprintf (stderr, "%s\n", backtraceSymbols [index]);

	free (backtraceSymbols);

	fprintf (stderr, "\n- END BACKTRACE ----------------------------------------------------------------\n\n");
	fprintf (stderr, "Programmer's note: Use addr2line to convert binary addresses into source code line numbers.\n\n");
	fprintf (stderr, "addr2line -e <Program Name, ex. bin/program> <Address, ex. +0x2df5\n\n");
}

static void abrtHandler (int sig)
{
	fprintf (stderr, "\n\nProgram Aborted!\n");
	printBacktrace ();

	// End the program
	exit (sig);
}

static void segvHandler (int sig)
{
	fprintf (stderr, "\n\nSegmentation Fault Occurred!\n");
	printBacktrace ();

	// End the program
	exit (sig);
}

#endif // __unix__

void debugInit ()
{
	#if __unix__
	signal (SIGSEGV, segvHandler);
	signal (SIGABRT, abrtHandler);
	#endif // __unix__
}