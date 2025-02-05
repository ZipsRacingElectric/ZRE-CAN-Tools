#ifndef CAN_EEPROM_H
#define CAN_EEPROM_H

// EEPROM CAN Interface -------------------------------------------------------------------------------------------------------
//
// Author: Cole Barach
// Date created: 2025.01.09
//
// Description: Set of objects for interacting with a device's EEPROM through a CAN bus. In order to use this, a device must
//   implement the handshake described below.
//
// TODO(Barach):
// - The scope of this module has grown way too much, consider splitting into low-level (protocol & handshake) and high-level
//   (variable mapping, matrices, stdio, etc.)
//
// Command Message:               Response Message:
//   ---------------------------    ---------------------------
//   | Byte | Meaning          |    | Byte | Meaning          |
//   |------|------------------|    |------|------------------|
//   |  7   |                  |    |  7   |                  |
//   |  6   | Data Word        |    |  6   | Data Word        |
//   |  5   | (32-bit)         |    |  5   | (32-bit)         |
//   |  4   |                  |    |  4   |                  |
//   |------|------------------|    |------|------------------|
//   |  3   | Data Address     |    |  3   | Data Address     |
//   |  2   | (16-bit)         |    |  2   | (16-bit)         |
//   |------|------------------|    |------|------------------|
//   |  1   | Reserved         |    |  1   | Reserved         |
//   |------|------------------|    |------|------------------|
//   |  0   | Instruction Byte |    |  0   | Instruction Byte |
//   ---------------------------    ---------------------------
//   ID: eeprom.canId               ID: eeprom.canId + 1
//
//   Data Word (data write command only): The data to write to the EEPROM.
//   Data Word (data read response only): The data read from the EEPROM.
//   Data Address (data command only): The address to read from / write to.
//   Data Address (data response only): The address that was read from.
//   Instruction Byte: Indicates the type of command/reponse, see below.
//
//   Responses should only be given to read commands.
//
// Instruction Byte (Data Message):
//       -------------------------------------------------
//   Bit |  7  |  6  |  5  |  4  |  3  |  2  |  1  |  0  |
//       |-----------------------------------|-----|-----|
//   Var |           Reserved    |     DC    | D/V | R/W |
//       -------------------------------------------------
//
// Instruction Byte (Validation Message):
//       -------------------------------------------------
//   Bit |  7  |  6  |  5  |  4  |  3  |  2  |  1  |  0  |
//       |-----------------------------|-----|-----|-----|
//   Var |           Reserved          |  IV | D/V | R/W |
//       -------------------------------------------------
//
// Instruction Byte Variables:
//   R/W: Read / Not Write
//     0 => Write Command
//     1 => Read Command
//   D/V: Data / Not Validation
//     0 => Interpret as Validation Message
//     1 => Interpret as Data Message
//   DC: Data Count (data message only). Indicates the number of bytes to read/write.
//     0x0 => 1 Byte
//     0x1 => 2 Bytes
//     0x2 => 3 Bytes
//     0x3 => 4 Bytes
//   IV: Is Valid (validation message only). In a write command, indicates the validity to be written. In a read response,
//     indicates the validity that was read.
//     0 => Invalid
//     1 => Valid

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
	uint16_t				canId;
	canEepromVariable_t*	variables;
	uint16_t				variableCount;
	void*					buffer;
} canEeprom_t;

// Functions ------------------------------------------------------------------------------------------------------------------

int canEepromInit (canEeprom_t* eeprom, cJSON* json);

/**
 * @brief Writes a block of data to the EEPROM from a block of memory.
 * @param eeprom The EEPROM to write to.
 * @param socket The CAN socket to negotiate with.
 * @param address The memory address to begin the write at.
 * @param count The number of bytes to write.
 * @param buffer The buffer to write the data from.
 * @return 0 if successful, the error code otherwise.
 */
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

/**
 * @brief Writes a variable's value into the EEPROM.
 * @param eeprom The EEPROM to write to.
 * @param socket The CAN socket to negotiate with.
 * @param variable The variable to write.
 * @param buffer The buffer to read the data from.
 * @return 0 if successful, the error code otherwise.
 */
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

/**
 * @brief Writes the contents of a data JSON to the EEPROM.
 * @param eeprom The EEPROM to write to.
 * @param socket The CAN socket to negotiate with.
 * @param dataJson The data JSON file to write from.
 * @return 0 if successful, the error code otherwise.
 */
int canEepromWriteJson (canEeprom_t* eeprom, canSocket_t* socket, cJSON* dataJson);

/**
 * @brief Reads the contents of the EEPROM into a data JSON.
 * TODO(Barach): This function should write into a @c cJSON struct, not to an I/O stream. This is difficult and not really
 * worth it right now, but it'd be a lot more graceful than the current method.
 * @param eeprom The EEPROM to read from.
 * @param socket The CAN socket to negotiate with.
 * @param stream The stream to print the JSON to.
 * @return 0 if successful, the error code otherwise.
 */
int canEepromReadJson (canEeprom_t* eeprom, canSocket_t* socket, FILE* stream);

/**
 * @brief Writes a validation command to the EEPROM.
 * @param eeprom The EEPROM to validate.
 * @param socket The CAN socket to negotiate with.
 * @param isValid The validity to write. True => valid, false => invalid.
 * @return 0 if successful, the error code otherwise.
 */
int canEepromValidate (canEeprom_t* eeprom, canSocket_t* socket, bool isValid);

/**
 * @brief Reads the validity of the EEPROM.
 * @param eeprom The EEPROM to read from.
 * @param socket The CAN socket to negotiate with.
 * @param isValid Written to contain the validity of the EEPROM.
 * @return 0 if successful, the error code otherwise.
 */
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

/**
 * @brief Prompts the user for the value of an EEPROM variable.
 * @param variable The variable to prompt for.
 * @param buffer The buffer to write the value into.
 * @param streamIn The stream to read input from.
 * @param streamOut The stream to print the prompt to. Use @c NULL for no prompt.
 */
void canEepromPromptValue (canEepromVariable_t* variable, void* buffer, FILE* streamIn, FILE* streamOut);

/**
 * @brief Prints the value of a variable to the specified stream.
 * @param variable The variable to print the value of.
 * @param buffer The value of the variable.
 * @param stream The stream to print to.
 */
void canEepromPrintVariable (canEepromVariable_t* variable, void* buffer, FILE* stream);

/**
 * @brief Reads and prints all the contents of an EEPROM's memory to a table.
 * @param eeprom The EEPROM to read from.
 * @param socket The CAN socket to negotiate with.
 * @param stream The stream to print to.
 * @return 0 if successful, the error code otherwise.
 */
int canEepromPrintMap (canEeprom_t* eeprom, canSocket_t* socket, FILE* stream);

/**
 * @brief Prints a list of an EEPROM's variables, excluding values.
 * @param eeprom The EEPROM to print the map of.
 * @param stream The stream to print to.
 */
void canEepromPrintEmptyMap (canEeprom_t* eeprom, FILE* stream);

#endif // CAN_EEPROM_H