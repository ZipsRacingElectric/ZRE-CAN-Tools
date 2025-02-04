#ifndef CAN_EEPROM_H
#define CAN_EEPROM_H

// EEPROM CAN Interface -------------------------------------------------------------------------------------------------------
//
// Author: Cole Barach
// Date created: 2025.01.09
//
// Description: Set of functions for interfacing with an EEPROM via CAN.

// Includes -------------------------------------------------------------------------------------------------------------------

// Includes
#include "can/can_socket.h"
#include "cjson/cjson_util.h"

// Debugging ------------------------------------------------------------------------------------------------------------------

#define CAN_EEPROM_DEBUG 1

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

int canEepromWrite (canEeprom_t* eeprom, canSocket_t* socket, uint16_t address, uint16_t count, void* buffer);

/**
 * @brief Reads a block of data from the EEPROM into a block of memory.
 * @param eeprom The EEPROM to read from.
 * @param socket The CAN socket to negotiate with.
 * @param address The memory address to begin the read at.
 * @param count The number of bytes to read.
 * @param buffer The buffer to write the read data into.
 * @return 0 if successful, the error code otherwise.
 */
int canEepromRead (canEeprom_t* eeprom, canSocket_t* socket, uint16_t address, uint16_t count, void* buffer);

int canEepromWriteVariable (canEeprom_t* eeprom, canSocket_t* socket, canEepromVariable_t* variable, void* buffer);

/**
 * @brief Reads a variable's value from the EEPROM.
 * @param eeprom The EEPROM to read from.
 * @param socket The CAN socket to negotiate with.
 * @param variable The variable to read.
 * @param buffer The buffer to write the read data into.
 * @return 0 if successful, the error code otherwise.
 */
int canEepromReadVariable (canEeprom_t* eeprom, canSocket_t* socket, canEepromVariable_t* variable, void* buffer);

int canEepromWriteJson (canEeprom_t* eeprom, canSocket_t* socket, cJSON* dataJson);

int canEepromReadJson (canEeprom_t* eeprom, canSocket_t* socket, FILE* stream);

int canEepromValidate (canEeprom_t* eeprom, canSocket_t* socket, bool isValid);

int canEepromIsValid (canEeprom_t* eeprom, canSocket_t* socket, bool* isValid);

// Standard I/O ---------------------------------------------------------------------------------------------------------------

/**
 * @brief Prompts the user to select a variable from the EEPROM.
 * @param eeprom The EEPROM to select from.
 * @param steamIn The stream to read input from.
 * @param streamOut The stream to print the prompt to. Use @c NULL for no prompt.
 * @return The selected variable.
 */
canEepromVariable_t* canEepromPromptVariable (canEeprom_t* eeprom, FILE* streamIn, FILE* streamOut);

void canEepromPromptValue (canEepromVariable_t* variable, void* buffer, FILE* streamIn, FILE* streamOut);

/**
 * @brief Prints the value of a variable to the specified stream.
 * @param variable The variable to print the value of.
 * @param buffer The value of the variable.
 * @param stream The stream to print to.
 */
void canEepromPrintVariable (canEepromVariable_t* variable, void* buffer, FILE* stream);

int canEepromPrintMap (canEeprom_t* eeprom, canSocket_t* socket, FILE* stream);

#endif // CAN_EEPROM_H