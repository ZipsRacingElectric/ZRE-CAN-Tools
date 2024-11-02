#ifndef CAN_DBC_H
#define CAN_DBC_H

// CAN DBC --------------------------------------------------------------------------------------------------------------------
//
// Author: Cole Barach
// Date Created: 2023.07.19
//
// Description: A group of functions relating to CAN DBC files.
//
// References:
// - http://mcu.so/Microcontroller/Automotive/dbc-file-format-documentation_compress.pdf
// - https://www.csselectronics.com/pages/can-dbc-file-database-intro
// - https://docs.fileformat.com/database/dbc/

// Includes -------------------------------------------------------------------------------------------------------------------

// Includes
#include "can_socket.h"

// C Standard Library
#include <stdbool.h>
#include <stdlib.h>

// Debugging ------------------------------------------------------------------------------------------------------------------

#define CAN_DBC_DEBUG_INFO		1
#define CAN_DBC_DEBUG_PARSING	0

// Constants ------------------------------------------------------------------------------------------------------------------

/// @brief The maximum length of a line in the file, in bytes.
#define LINE_LENGTH_MAX 4096

// Functions ------------------------------------------------------------------------------------------------------------------

/**
 * @brief Parses the specified CAN database file.
 * @param path The relative/absolute path of the DBC file.
 * @param messages The array to populate with the database's messages.
 * @param messageCount Input, should contain the size of the @c messages array. Output written to contain the number of
 * populated messages.
 * @param signals The array of populate with the database's signals.
 * @param signalCount Input, should contain the size of the @c signals array. Output written to contain the number of populated
 * signals.
 * @return True if successful, false otherwise. 
 */
bool dbcFileParse (const char* path, canMessage_t* messages, size_t* messageCount, canSignal_t* signals, size_t* signalCount);

#endif // CAN_DBC_H