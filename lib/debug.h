#ifndef DEBUG_H
#define DEBUG_H

// C Standard Library
#include <stdio.h>

// TODO(Barach): Docs
void debugInit (void);

// TODO(Barach): Docs
void debugSetStream (FILE* stream);

// TODO(Barach): Docs
void debugPrintf (const char* message, ...);

// TODO(Barach): Rework docs
/**
 * @brief Prints an error message to @c stderr . The resulting message takes the following format:
 * "<User Message>: <Error Message>"
 *
 * For example:
 * "Failed to open file 'test.txt': No such file or directory."
 *
 * @param message The user message to preface the error message with. Note this can be a format string, in which case the
 * following arguments should be the values to be inserted in place of the format specifiers.
 * @param ... The variadic arguments to insert into the format string. Same convention as the @c printf family of functions.
 * @return The error code associated with the error, that is, the value of @c errno upon entry to the function. Note this
 * resets @c errno , so this return code must be used to determine what the error was.
 */
int errorPrintf (const char* message, ...);

#endif // DEBUG_H