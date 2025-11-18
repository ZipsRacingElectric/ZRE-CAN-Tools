#ifndef OPTIONS_H
#define OPTIONS_H

// Common Standard Options ----------------------------------------------------------------------------------------------------
//
// Author: Cole Barach
// Date Created: 2025.11.17
//
// Description: Common standard options shared by all applications. See doc/common_library.md for more info.

// Includes -------------------------------------------------------------------------------------------------------------------

// C Standard Library
#include <stdio.h>

// Functions ------------------------------------------------------------------------------------------------------------------

typedef enum
{
	/// @brief Indicates an option is invalid. This does not indicate the arguement is invalid, simply that it is not an
	/// option.
	OPTION_INVALID = 0,

	/// @brief Indicates an option was handled correctly, and the application should proceed normally.
	OPTION_HANDLED = 1,

	/// @brief Indicates an option was handled correctly, and the application should be terminated gracefully.
	OPTION_QUIT = 2,

	/// @brief Indicates the option was not handled, and a character option was returned.
	OPTION_CHAR = 3,

	/// @brief Indicates the option was not handled, and a string option was returned.
	OPTION_STRING = 4
} optionReturn_t;

/**
 * @brief User-provided callback for printing to an output stream.
 * @param stream The I/O stream to print to.
 */
typedef void (fprintCallback_t) (FILE* stream);

/**
 * @brief Checks and handles a standard argment. If the argument is a common option, the behavior will be executed. If the
 * argument is not a known option, the option will be returned for the user to handle. If the option is not valid, such will be
 * indicated.
 * @param arg The standard argument to check.
 * @param option Buffer to write the option into. Use @c NULL to ignore.
 * @param fprintHelp Callback for printing the help page. Use @c NULL to use the default help page.
 * @return Indicates how / if the option was handled.
 */
optionReturn_t handleOption (const char* arg, const char** option, fprintCallback_t* fprintHelp);

/**
 * @brief Prints the help page for the common application options.
 * @param stream The I/O stream to print to.
 * @param indent The indentation to put before each line. Note that spaces are preferred to tabs.
 * @return The number of bytes written if successful, a negative value otherwise.
 */
int fprintOptionHelp (FILE* stream, char* indent);

#endif // OPTIONS_H