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

// Datatypes ------------------------------------------------------------------------------------------------------------------

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
 * @brief User-provided callback for handling a character option.
 * @param option The character option to handle. Note one handler function can be used for multiple different options, hence
 * the need for this. If not being re-used, this can be ignored.
 * @param value The value passed into this option, if any. @c NULL if no value was specified. Values are specified by using:
 * -<Option>=<Value>
 */
typedef void (optionCharCallback_t) (char option, char* value);

/**
 * @brief User-provided callback for handling a string option.
 * @param option The string option to handle. Note one handler function can be used for multiple different options, hence
 * the need for this. If not being re-used, this can be ignored.
 * @param value The value passed into this option, if any. @c NULL if no value was specified. Values are specified by using:
 * --<Option>=<Value>
 */
typedef void (optionStringCallback_t) (char* option, char* value);

typedef struct
{
	/// @brief Callback for printing the help page. Use @c NULL to use the default help page.
	fprintCallback_t* fprintHelp;

	/// @brief Array of handlers to call for each character option. 1:1 with the characters in @c chars . See
	/// @c optionCharCallback_t for how the parameters are used.
	optionCharCallback_t** charHandlers;

	/// @brief Array of characters matching each character option. 1:1 with the handlers in @c charHandlers .
	char* chars;

	/// @brief The number of elements in both @c charHandlers and @c chars .
	size_t charCount;

	/// @brief Array of handlers for each string option. 1:1 with the strings in @c strings . See @c optionStringCallback_t for
	/// how the parameters are used.
	optionStringCallback_t** stringHandlers;

	/// @brief Array of handlers for each string option. 1:1 with the handlers in @c stringHandlers .
	char** strings;

	/// @brief The number of elements in both @c stringHandlers and @c strings .
	size_t stringCount;
} handleOptionsParams_t;

// Functions ------------------------------------------------------------------------------------------------------------------

/**
 * @brief Checks and handles a single standard argument. If the argument is a common option, the correct behavior will be
 * executed. If the argument is not a known option, the option will be returned for the user to handle. If the option is not
 * valid, such will be indicated.
 * @param arg The standard argument to check.
 * @param option Buffer to write the option into. Use @c NULL to ignore.
 * @param fprintHelp Callback for printing the help page. Use @c NULL to use the default help page.
 * @return Indicates how / if the option was handled.
 */
optionReturn_t handleOption (const char* arg, const char** option, fprintCallback_t* fprintHelp);

/**
 * @brief Checks and handles a program's standard arguments. If the argument is a common option, the correct behavior will be
 * executed. If the argument is not a known option, the handlers provided in @c params will be checked for a correct handler.
 * Upon parsing an argument that is not an option (doesn't start with "-" or "--") the parsing will be stopped and argc / argv
 * are upated to contain the unparsed arguments.
 * @param argc A pointer to the number of arguments in @c argv (should be @c &argc of main). Updated to contain the number of
 * unparsed arguments in argv, or 0 is none are left.
 * @param argv A pointer to the array of argument strings to parse. The number of values in the array pointed to is given by
 * the value pointed to by @c argc (should be @c &argv of main). Updated to point to the array of unparsed strings on return.
 * @param params Structure of parameters to use. See @c handleOptionsParams_t for more details.
 * @return 0 if successful, the error code otherwise. Note @c errno is set on failure.
 */
int handleOptions (int* argc, char*** argv, handleOptionsParams_t* params);

/**
 * @brief Prints the help page for the common application options.
 * @param stream The I/O stream to print to.
 * @param indent The indentation to put before each line. Note that spaces are preferred to tabs.
 * @return The number of bytes written if successful, a negative value otherwise.
 */
int fprintOptionHelp (FILE* stream, char* indent);

#endif // OPTIONS_H