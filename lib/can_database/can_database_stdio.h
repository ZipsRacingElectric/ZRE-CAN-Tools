#ifndef CAN_DATABASE_STDIO_H
#define CAN_DATABASE_STDIO_H

// CAN Database Standard Input / Output ---------------------------------------------------------------------------------------
//
// Author: Cole Barach
// Date Created: 2025.11.18
//
// Description: Helper functions for using standard input & output with the can_database library.

// Includes -------------------------------------------------------------------------------------------------------------------

// C Standard Library
#include <stdio.h>

// Functions ------------------------------------------------------------------------------------------------------------------

/**
 * @brief Prints the help page for the DBC File parameter.
 * @param stream The I/O stream to print to.
 * @param indent The indentation to put before each line. Note that spaces are preferred to tabs.
 * @return The number of bytes written if successful, a negative value otherwise.
 */
int fprintCanDbcFileHelp (FILE* stream, char* indent);

#endif // CAN_DATABASE_STDIO_H