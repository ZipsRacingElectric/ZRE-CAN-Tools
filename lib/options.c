// Header
#include "options.h"

// Includes
#include "debug.h"
#include "error_codes.h"

// C Standard Library
#include <errno.h>
#include <string.h>
#include <stdlib.h>

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

int handleOptions (int* argc, char*** argv, handleOptionsParams_t* params)
{
	int optionIndex;
	for (optionIndex = 1; optionIndex < *argc; ++optionIndex)
	{
		// TODO(Barach): Const casting?
		char* option;
		switch (handleOption ((*argv) [optionIndex], (const char**) &option, params->fprintHelp))
		{
		case OPTION_CHAR:
		{
			// Find the correct handler
			size_t charIndex;
			for (charIndex = 0; charIndex < params->charCount; ++charIndex)
			{
				// If not the correct handler, skip and try the next.
				if (option [0] != params->chars [charIndex])
					continue;

				// If an equal sign is present, pass in the value.
				if (option [1] == '=')
					params->charHandlers [charIndex] (option [0], option + 2);
				else
					params->charHandlers [charIndex] (option [0], NULL);

				// Handled correctly, break the loop.
				break;
			}

			// If the option was not handled, return failure.
			if (charIndex == params->charCount)
			{
				debugPrintf ("Invalid char option '%s'.\n", (*argv) [optionIndex]);
				errno = ERRNO_INVALID_OPTION;
				return errno;
			}

			break;
		}

		case OPTION_STRING:
		{
			// If the option is "--", stop parsing the args.
			if (option [0] == '\0')
			{
				// Update argc/argv to the arg just after this one.
				*argc -= optionIndex + 1;
				*argv += optionIndex + 1;
				return 0;
			}

			// If an equal sign is present, split at the sign and get the value after.
			size_t splitIndex = strcspn (option, "=");
			char* value = NULL;
			if (option [splitIndex] == '=')
			{
				option [splitIndex] = '\0';
				value = option + splitIndex + 1;
			}

			// Find the correct handler
			size_t stringIndex;
			for (stringIndex = 0; stringIndex < params->stringCount; ++stringIndex)
			{
				// If not the correct handler, skip and try the next.
				if (strcmp (option, params->strings [stringIndex]) != 0)
					continue;

				// Call the handler
				params->stringHandlers [stringIndex] (option, value);

				// Handled correctly, break the loop.
				break;
			}

			// If the option was not handled, return failure.
			if (stringIndex == params->stringCount)
			{
				debugPrintf ("Invalid string option '%s'.\n", (*argv) [optionIndex]);
				errno = ERRNO_INVALID_OPTION;
				return errno;
			}

			break;
		}

		case OPTION_QUIT:
			// Quit the program.
			exit (0);

		case OPTION_INVALID:
			// Update argc/argv to this arg.
			*argc -= optionIndex;
			*argv += optionIndex;
			return 0;

		case OPTION_HANDLED:
			// If handled, do nothing and continue parsing.
			break;
		}
	}

	*argc -= optionIndex;
	*argv += optionIndex;
	return 0;
}

int fprintOptionHelp (FILE* stream, char* indent)
{
	return fprintf (stream, ""
		"%s-h, --help            - Help, prints this page.\n"
		"%s--verbose             - Verbose debugging, prints more detailed information\n"
		"%s                        to standard output.\n"
		"%s-v, --version         - Version, prints the application version.\n"
		"%s--                    - Stop parsing options. All arguments after this are\n"
		"%s                        parsed as arguments, not options.\n\n",
		indent, indent, indent, indent, indent, indent);
}