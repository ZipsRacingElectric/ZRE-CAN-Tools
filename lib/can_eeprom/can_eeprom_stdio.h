#ifndef CAN_EEPROM_STDIO_H
#define CAN_EEPROM_STDIO_H

// CAN EEPROM Standard Input / Output -----------------------------------------------------------------------------------------
//
// Author: Cole Barach
// Date Created: 2025.11.18
//
// Description: Helper functions for using standard input & output with the can_eeprom library.

// Includes -------------------------------------------------------------------------------------------------------------------

// C Standard Library
#include <stdio.h>

// Functions ------------------------------------------------------------------------------------------------------------------

/**
 * @brief Prints the help page for the EEPROM config file parameter.
 * @param stream The I/O stream to print to.
 * @param indent The indentation to put before each line. Note that spaces are preferred to tabs.
 * @return The number of bytes written if successful, a negative value otherwise.
 */
int fprintCanEepromConfigHelp (FILE* stream, char* indent);

/**
 * @brief Prints the help page for the EEPROM data file parameter.
 * @param stream The I/O stream to print to.
 * @param indent The indentation to put before each line. Note that spaces are preferred to tabs.
 * @return The number of bytes written if successful, a negative value otherwise.
 */
int fprintCanEepromDataHelp (FILE* stream, char* indent);

#endif // CAN_EEPROM_STDIO_H