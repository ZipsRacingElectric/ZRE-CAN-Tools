#ifndef BMS_STDIO_H
#define BMS_STDIO_H

// BMS Standard Input / Output ------------------------------------------------------------------------------------------------
//
// Author: Cole Barach
// Date Created: 2025.11.18
//
// Description: Helper functions for using standard input & output with the bms library.

// Includes -------------------------------------------------------------------------------------------------------------------

// C Standard Library
#include <stdio.h>

// Functions ------------------------------------------------------------------------------------------------------------------

/**
 * @brief Prints the help page for the BMS config file parameter.
 * @param stream The I/O stream to print to.
 * @param indent The indentation to put before each line. Note that spaces are preferred to tabs.
 * @return The number of bytes written if successful, a negative value otherwise.
 */
int fprintBmsConfigFileHelp (FILE* stream, char* indent);

#endif // BMS_STDIO_H