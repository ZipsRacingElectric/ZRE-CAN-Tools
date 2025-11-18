// Header
#include "options.h"

// Includes
#include "debug.h"

// C Standard Library
#include <string.h>

static void handleHelp (fprintCallback_t* fprintHelp)
{
	if (fprintHelp != NULL)
		fprintHelp (stdout);
	else
	{
		printf ("Options:\n\n");
		fprintOptionHelp (stdout, "    ");
		printf ("\n");
	}
}

static void handleVersion ()
{
	printf ("%s\n", ZRE_CANTOOLS_VERSION_FULL);
}

static void handleVerbose ()
{
	debugSetStream (stderr);
}

optionReturn_t handleOption (const char* arg, const char** option, fprintCallback_t* fprintHelp)
{
	if (arg [0] != '-')
	{
		// Allow just 'help' as an option, just to be nice.
		if (strcmp (arg, "help") == 0)
		{
			handleHelp (fprintHelp);
			return OPTION_QUIT;
		}

		return OPTION_INVALID;
	}

	if (arg [1] == '-')
	{
		// String option

		const char* string = arg + 2;

		if (strcmp (string, "help") == 0)
		{
			handleHelp (fprintHelp);
			return OPTION_QUIT;
		}

		if (strcmp (string, "verbose") == 0)
		{
			handleVerbose ();
			return OPTION_HANDLED;
		}

		if (strcmp (string, "version") == 0)
		{
			handleVersion ();
			return OPTION_QUIT;
		}

		if (option != NULL)
			*option = string;

		return OPTION_STRING;
	}

	// Character option

	if (arg [1] == '\0')
		return OPTION_INVALID;

	char character = arg [1];

	switch (character)
	{
	case 'h':
		handleHelp (fprintHelp);
		return OPTION_QUIT;

	case 'v':
		handleVersion ();
		return OPTION_QUIT;
	}

	if (option != NULL)
		*option = arg + 1;
	return OPTION_CHAR;
}

int fprintOptionHelp (FILE* stream, char* indent)
{
	return fprintf (stream, ""
		"%s-h, --help            - Help, prints this page.\n"
		"%s--verbose             - Verbose debugging, prints more detailed information\n"
		"%s                        to standard output.\n"
		"%s-v, --version         - Version, prints the application version.\n\n",
		indent, indent, indent, indent);
}