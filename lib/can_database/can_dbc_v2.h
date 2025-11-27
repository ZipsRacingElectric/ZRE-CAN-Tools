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
#include "can_signals.h"

// C Standard Library
#include <stdlib.h>

// Functions ------------------------------------------------------------------------------------------------------------------

/**
 * @brief Loads the contents of a CAN DBC file into a set of parallel arrays.
 * @param dbcFile The path of the DBC file to load.
 * @param messages Buffer to write the address of the message array into.
 * @param messageCount Buffer to write the size of the message array into.
 * @param signals Buffer to write the address of the signal array into.
 * @param signalCount Buffer to write the size of the signal array into.
 * @return 0 if successful, the error code otherwise.
 */
int canDbcLoad (char* dbcFile, canMessage_t** messages, size_t* messageCount,
	canSignal_t** signals, size_t* signalCount);

/**
 * @brief Loads an array of CAN DBC files into a set of parallel arrays.
 * @param dbcFiles The array of pathes of each DBC file to load.
 * @param dbcCount The number of elements in @c dbcFiles .
 * @param messages Buffer to write the address of the message array into.
 * @param messageCount Buffer to write the size of the message array into.
 * @param signals Buffer to write the address of the signal array into.
 * @param signalCount Buffer to write the size of the signal array into.
 * @return 0 if successful, the error code otherwise.
 */
int canDbcsLoad (char** dbcFiles, size_t dbcCount, canMessage_t** messages, size_t* messageCount,
	canSignal_t** signals, size_t* signalCount);

/**
 * @brief Deallocates the arrays created by either @c canDbcLoad or @c canDbcsLoad .
 * @param messages The array of messages.
 * @param messageCount The size of the message array.
 * @param signals The array of signals.
 */
void canDbcsDealloc (canMessage_t* messages, size_t messageCount, canSignal_t* signals);

#endif // CAN_DBC_H