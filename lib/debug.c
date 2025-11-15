// Header
#include "debug.h"

// C Standard Library
#include <signal.h>
#include <stdlib.h>

FILE* errorStream = NULL;

#if ZRE_CANTOOLS_OS_linux

// POSIX
#include <execinfo.h>

static void printBacktrace (void)
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

#endif // ZRE_CANTOOLS_OS_linux

void debugInit (void)
{
	// TODO(Barach): Implement this for windows.

	#ifdef ZRE_CANTOOLS_OS_linux

	signal (SIGSEGV, segvHandler);
	signal (SIGABRT, abrtHandler);

	#endif // ZRE_CANTOOLS_OS_linux
}