#ifndef DEBUG_H
#define DEBUG_H

// Common Debugging Header ----------------------------------------------------------------------------------------------------

// Author: Cole Barach
// Date Created: 2025.07.07
//
// Description: Common functions for debugging applications and libraries.

// Includes -------------------------------------------------------------------------------------------------------------------

// C Standard Library
#include <stdio.h>

// Functions ------------------------------------------------------------------------------------------------------------------

/**
 * @brief Initializes the debugging module. This should be called at the beginning of an application's life-time.
 */
void debugInit (void);

/**
 * @brief Sets the destination for anything printed to @c debugPrintf . Note this should only be used by applications, not
 * libraries. All future calls to @c debugPrintf will use this stream.
 * @param stream The output stream to direct to.
 */
void debugSetStream (FILE* stream);

/**
 * @brief Prints diagnostic information to the debugging output stream.
 * @note See @c debugSetStream for how to choose the output destination.
 * @param message The message to print. Note this can be a format string, in which case the following arguments should be the
 * values to be inserted in place of the format specifiers.
 * @param ... The variadic arguments to insert into the format string. Same convention as the @c printf family of functions.
 */
void debugPrintf (const char* message, ...);

/**
 * @brief Prints an error message to @c stderr based on the current value of @c errno . The resulting message takes the
 * following format:
 *   "<User Message>: <Error Message>"
 *
 * For example:
 *   "Failed to open file 'test.txt': No such file or directory."
 *
 * @param message The user message to preface the error message with. Note this can be a format string, in which case the
 * following arguments should be the values to be inserted in place of the format specifiers.
 * @param ... The variadic arguments to insert into the format string. Same convention as the @c printf family of functions.
 * @return The error code associated with the error, that is, the value of @c errno upon entry to the function. Note this
 * resets @c errno , so this return code must be used to determine what the error was.
 */
int errorPrintf (const char* message, ...);

#endif // DEBUG_H