#ifndef CAN_EEPROM_H
#define CAN_EEPROM_H

// EEPROM CAN Interface -------------------------------------------------------------------------------------------------------
//
// Author: Cole Barach
// Date created: 2025.01.09
//
// Description: Set of functions for interfacing with an EEPROM via CAN.
//
// TODO(Barach):
// - Index should be size_t or uint16_t.

// Includes -------------------------------------------------------------------------------------------------------------------

// Includes
#include "can_socket.h"

// cJSON
#include "cjson.h"

// Datatypes ------------------------------------------------------------------------------------------------------------------

typedef enum
{
	CAN_EEPROM_TYPE_UINT8_T		= 0,
	CAN_EEPROM_TYPE_UINT16_T	= 1,
	CAN_EEPROM_TYPE_UINT32_T	= 2,
	CAN_EEPROM_TYPE_FLOAT		= 3,
} canEepromType_t;

typedef struct
{
	char*			name;
	uint16_t		address;
	canEepromType_t	type;
} canEepromVariable_t;

typedef struct
{
	char*					name;
	uint16_t				canAddress;
	canEepromVariable_t*	variables;
	uint16_t				variableCount;
} canEeprom_t;

// Functions ------------------------------------------------------------------------------------------------------------------

int canEepromInit (canEeprom_t* eeprom, cJSON* json);

int canEepromWrite (canEeprom_t* eeprom, canSocket_t* socket, uint16_t variableIndex, void* data);

int canEepromRead (canEeprom_t* eeprom, canSocket_t* socket, uint16_t variableIndex, void* data);

int canEepromValidate (canEeprom_t* eeprom, canSocket_t* socket, bool isValid);

int canEepromIsValid (canEeprom_t* eeprom, canSocket_t* socket, bool* isValid);

// Standard I/O ---------------------------------------------------------------------------------------------------------------

uint16_t canEepromVariablePrompt (canEeprom_t* eeprom);

void canEepromValuePrompt (canEeprom_t* eeprom, uint16_t variableIndex, void* data);

void canEepromPrintVariable (canEeprom_t* eeprom, uint16_t variableIndex, void* data);

void canEepromPrintMap (canEeprom_t* eeprom, canSocket_t* socket);

void canEepromPrintEmptyMap (canEeprom_t* eeprom);

#endif // CAN_EEPROM_H