// Header
#include "debug.h"

// Includes
#include "error_codes.h"

// C Standard Library
#include <errno.h>
#include <signal.h>
#include <stdarg.h>
#include <stdlib.h>

#ifdef ZRE_CANTOOLS_OS_linux

// POSIX
#include <execinfo.h>

#endif // ZRE_CANTOOLS_OS_linux

// Globals --------------------------------------------------------------------------------------------------------------------

FILE* debugStream = NULL;

// Functions ------------------------------------------------------------------------------------------------------------------

#ifdef ZRE_CANTOOLS_OS_linux

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

void debugSetStream (FILE* stream)
{
	debugStream = stream;
}

void debugPrintf (const char* message, ...)
{
	// If debug output is disabled, ignore this.
	if (debugStream == NULL)
		return;

	// Print the user message, along with variadic arguments.
	va_list args;
    va_start(args, message);
    vfprintf(debugStream, message, args);
    va_end(args);
}

int errorPrintf (const char* message, ...)
{
	// Store the error that caused the issue and reset errno for later usage.
	int code = errno;
	errno = 0;

	// Print the user message, along with variadic arguments.
	va_list args;
    va_start(args, message);
    vfprintf(stderr, message, args);
    va_end(args);

	// Print the error message.
	fprintf (stderr, ": %s.\n", errorCodeToMessage (code));

	// Return the errno value.
	return code;
}