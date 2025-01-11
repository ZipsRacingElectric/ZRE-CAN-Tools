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
// - If a read fails the program hangs.

// Includes -------------------------------------------------------------------------------------------------------------------

#include "can_socket.h"

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

struct can_frame canEepromWriteMessageEncode (canEeprom_t* eeprom, uint16_t variableIndex, void* data);

struct can_frame canEepromReadMessageEncode (canEeprom_t* eeprom, uint16_t variableIndex);

bool canEepromReadMessageParse (canEeprom_t* eeprom, struct can_frame* frame, uint16_t variableIndex, void* data);

struct can_frame canEepromValidateMessageEncode (canEeprom_t* eeprom, bool isValid);

struct can_frame canEepromIsValidMessageEncode (canEeprom_t* eeprom);

bool canEepromIsValidMessageParse (canEeprom_t* eeprom, struct can_frame* frame, bool* isValid);

bool canEepromWrite (canEeprom_t* eeprom, canSocket_t* socket, uint16_t variableIndex, void* data);

bool canEepromRead (canEeprom_t* eeprom, canSocket_t* socket, uint16_t variableIndex, void* data);

bool canEepromValidate (canEeprom_t* eeprom, canSocket_t* socket, bool isValid);

bool canEepromIsValid (canEeprom_t* eeprom, canSocket_t* socket, bool* isValid);

// Standard I/O ---------------------------------------------------------------------------------------------------------------

uint16_t canEepromVariableIndexPrompt (canEeprom_t* eeprom);

void canEepromVariableValuePrompt (canEeprom_t* eeprom, uint16_t variableIndex, void* data);

void canEepromPrintVariable (canEeprom_t* eeprom, uint16_t variableIndex, void* data);

void canEepromPrintMap (canEeprom_t* eeprom, canSocket_t* socket);

#endif // CAN_EEPROM_H