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

int canDbcLoad (const char* dbcFile);

#endif // CAN_DBC_H