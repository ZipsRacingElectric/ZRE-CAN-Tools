#ifndef CAN_EEPROM_H
#define CAN_EEPROM_H

// EEPROM CAN Interface -------------------------------------------------------------------------------------------------------
//
// Author: Cole Barach
// Date created: 2025.01.09
//
// Description: Set of functions for interfacing with an EEPROM via CAN.

// Includes -------------------------------------------------------------------------------------------------------------------

#include "can_socket.h"

// Datatypes ------------------------------------------------------------------------------------------------------------------

typedef enum
{
	CAN_EEPROM_DATATYPE_UINT8_T		= 0,
	CAN_EEPROM_DATATYPE_UINT16_T	= 1,
	CAN_EEPROM_DATATYPE_UINT32_T	= 2,
	CAN_EEPROM_DATATYPE_FLOAT		= 3,
} canEepromDatatype_t;

uint8_t CAN_EEPROM_DATATYPE_SIZES [] =
{
	sizeof (uint8_t),
	sizeof (uint16_t),
	sizeof (uint32_t),
	sizeof (float)
};

const char* CAN_EEPROM_DATATYPE_NAMES [] =
{
	"uint8_t",
	"uint16_t",
	"uint32_t",
	"float"
};

typedef struct
{
	uint16_t			address;
	const char*			name;
	canEepromDatatype_t	type;
} canEepromVariable_t;

typedef struct
{
	char*					name;
	uint16_t				canAddress;
	canEepromVariable_t*	variables;
	uint16_t				variableCount;
} canEeprom_t;

// Functions ------------------------------------------------------------------------------------------------------------------

#endif // CAN_EEPROM_H