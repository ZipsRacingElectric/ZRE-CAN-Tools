#ifndef CAN_EEPROM_H
#define CAN_EEPROM_H

// EEPROM CAN Interface -------------------------------------------------------------------------------------------------------
//
// Author: Cole Barach
// Date created: 2025.01.09
//
// Description: Object and functions for interacting with a device's EEPROM through a CAN bus. See the 'can_eeprom_operations'
//   module for a low-level description of the CAN interface a device must implement.
//
// TODO(Barach):
// - PrintMap operations intermittently become misaligned, printing invalid data. LLD ought to prevent such a situation.

// Includes -------------------------------------------------------------------------------------------------------------------

// Includes
#include "can_device/can_device.h"
#include "cjson/cjson.h"

// C Standard Library
#include <stdio.h>

// Datatypes ------------------------------------------------------------------------------------------------------------------

typedef enum
{
	CAN_EEPROM_TYPE_UINT8_T		= 0,
	CAN_EEPROM_TYPE_UINT16_T	= 1,
	CAN_EEPROM_TYPE_UINT32_T	= 2,
	CAN_EEPROM_TYPE_FLOAT		= 3,
} canEepromType_t;

typedef enum
{
	CAN_EEPROM_MODE_READ_WRITE	= 0,
	CAN_EEPROM_MODE_READ_ONLY	= 1,
	CAN_EEPROM_MODE_WRITE_ONLY	= 2
} canEepromMode_t;

typedef struct
{
	char*			name;
	uint16_t		address;
	canEepromType_t	type;
	canEepromMode_t	mode;
	uint16_t		width;
	uint16_t		height;
	char*			comment;
} canEepromVariable_t;

typedef struct
{
	/// @brief The user-friendly name of the device.
	char* name;
	/// @brief The base CAN ID of the device.
	uint16_t canId;
	/// @brief The array of EEPROM variables.
	canEepromVariable_t* variables;
	/// @brief The size of @c variables .
	uint16_t variableCount;
	/// @brief A buffer large enough to store the data of the largest variable.
	void* buffer;
} canEeprom_t;

// Functions ------------------------------------------------------------------------------------------------------------------

/**
 * @brief Initializes the EEPROM by importing the structure's data from a configuration JSON.
 * @param eeprom The EEPROM to initialize.
 * @param config The configuration JSON to load the data from.
 * @return 0 if successful, the error code otherwise.
 */
int canEepromInit (canEeprom_t* eeprom, cJSON* config);

/**
 * @brief Writes a variable's value into an EEPROM.
 * @param eeprom The EEPROM to write to.
 * @param device The CAN device to negotiate with.
 * @param variable The variable to write.
 * @param buffer The buffer to read the data from.
 * @return 0 if successful, the error code otherwise.
 */
int canEepromWriteVariable (canEeprom_t* eeprom, canDevice_t* device, canEepromVariable_t* variable, void* buffer);

/**
 * @brief Reads a variable's value from an EEPROM.
 * @param eeprom The EEPROM to read from.
 * @param device The CAN device to negotiate with.
 * @param variable The variable to read.
 * @param buffer The buffer to write the read data into.
 * @return 0 if successful, the error code otherwise.
 */
int canEepromReadVariable (canEeprom_t* eeprom, canDevice_t* device, canEepromVariable_t* variable, void* buffer);

/**
 * @brief Writes the contents of a data JSON to the EEPROM.
 * @param eeprom The EEPROM to write to.
 * @param device The CAN device to negotiate with.
 * @param dataJson The data JSON file to write from.
 * @return 0 if successful, the error code otherwise.
 */
int canEepromWriteJson (canEeprom_t* eeprom, canDevice_t* device, cJSON* dataJson);

/**
 * @brief Reads the contents of the EEPROM into a data JSON.
 * TODO(Barach): This function should write into a @c cJSON struct, not to an I/O stream. This is difficult and not really
 * worth it right now, but it'd be a lot more graceful than the current method.
 * @param eeprom The EEPROM to read from.
 * @param device The CAN device to negotiate with.
 * @param stream The stream to print the JSON to.
 * @return 0 if successful, the error code otherwise.
 */
int canEepromReadJson (canEeprom_t* eeprom, canDevice_t* device, FILE* stream);

/**
 * @brief Allocates a buffer large enough to store the data of the largest variable in an EEPROM.
 * @param eeprom The EEPROM to get the buffer for.
 * @return The dynamically allocated memory if successful, @c NULL otherwise.
 */
void* canEepromAllocateBuffer (canEeprom_t* eeprom);

// Standard I/O ---------------------------------------------------------------------------------------------------------------

/**
 * @brief Prompts the user to select a variable from the EEPROM.
 * @param eeprom The EEPROM to select from.
 * @param steamIn The stream to read input from.
 * @param streamOut The stream to print the prompt to. Use @c NULL for no prompt.
 * @return The selected variable.
 */
canEepromVariable_t* canEepromPromptVariable (canEeprom_t* eeprom, FILE* streamIn, FILE* streamOut);

/**
 * @brief Prompts the user for the value of an EEPROM variable.
 * @param variable The variable to prompt for.
 * @param buffer The buffer to write the value into.
 * @param streamIn The stream to read input from.
 * @param streamOut The stream to print the prompt to. Use @c NULL for no prompt.
 */
void canEepromPromptValue (canEepromVariable_t* variable, void* buffer, FILE* streamIn, FILE* streamOut);

/**
 * @brief Prints the value of a variable to the specified stream. The value is followed by a newline.
 * @param variable The variable to print the value of.
 * @param buffer The value of the variable.
 * @param indent The indentation to use at the beginning of each new line (for matrices).
 * @param stream The stream to print to.
 */
void canEepromPrintVariableValue (canEepromVariable_t* variable, void* buffer, const char* indent, FILE* stream);

/**
 * @brief Reads and prints all the contents of an EEPROM's memory to a table.
 * @param eeprom The EEPROM to read from.
 * @param device The CAN device to negotiate with.
 * @param stream The stream to print to.
 * @return 0 if successful, the error code otherwise.
 */
int canEepromPrintMap (canEeprom_t* eeprom, canDevice_t* device, FILE* stream);

/**
 * @brief Prints a list of an EEPROM's variables, excluding values.
 * @param eeprom The EEPROM to print the map of.
 * @param stream The stream to print to.
 */
void canEepromPrintEmptyMap (canEeprom_t* eeprom, FILE* stream);

#endif // CAN_EEPROM_H