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
#include "list.h"

// C Standard Library
#include <stdlib.h>

// Datatypes ------------------------------------------------------------------------------------------------------------------

listDefine (canMessage_t);

listDefine (canSignal_t);

// Functions ------------------------------------------------------------------------------------------------------------------

int canDbcLoad (const char* dbcFile, list_t (canMessage_t)* messages, list_t (canSignal_t)* signals);

void canDbcLink (canMessage_t* messages, size_t messageCount, canSignal_t* signals);

int canDbcsLoad (char* const* dbcFiles, size_t dbcCount, canMessage_t** messages, size_t* messageCount, canSignal_t** signals,
	size_t* signalCount);

void canDbcsDealloc (canMessage_t* messages, size_t messageCount, canSignal_t* signals);

#endif // CAN_DBC_H