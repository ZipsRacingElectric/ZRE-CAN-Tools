#ifndef CAN_EEPROM_OPERATIONS_H
#define CAN_EEPROM_OPERATIONS_H

// CAN EEPROM Low-Level Operations --------------------------------------------------------------------------------------------
//
// Author: Cole Barach
// Date created: 2025.02.05
//
// Description: Functions for operating on a device's EEPROM through a CAN bus. These functions are not intended for common
//   use, higher level interactions should be done using the 'can_eeprom' module. In order to use this, a device must
//   implement the below interface.
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

// Functions ------------------------------------------------------------------------------------------------------------------

/**
 * @brief Writes a block of data to the EEPROM from a block of memory.
 * @param canId The EEPROM's base CAN ID.
 * @param socket The CAN socket to negotiate with.
 * @param address The memory address to begin the write at.
 * @param count The number of bytes to write.
 * @param buffer The buffer containing the data to write.
 * @return 0 if successful, the error code otherwise.
 */
int canEepromWrite (uint16_t canId, canSocket_t* socket, uint16_t address, uint16_t count, void* buffer);

/**
 * @brief Reads a block of data from the EEPROM into a block of memory.
 * @param canId The EEPROM's base CAN ID.
 * @param socket The CAN socket to negotiate with.
 * @param address The memory address to begin the read at.
 * @param count The number of bytes to read.
 * @param buffer The buffer to write the read data into.
 * @return 0 if successful, the error code otherwise.
 */
int canEepromRead (uint16_t canId, canSocket_t* socket, uint16_t address, uint16_t count, void* buffer);

/**
 * @brief Writes a validation command to the EEPROM.
 * @param canId The EEPROM's base CAN ID.
 * @param socket The CAN socket to negotiate with.
 * @param isValid The validity to write. True => valid, false => invalid.
 * @return 0 if successful, the error code otherwise.
 */
int canEepromValidate (uint16_t canId, canSocket_t* socket, bool isValid);

/**
 * @brief Reads the validity of the EEPROM.
 * @param canId The EEPROM's base CAN ID.
 * @param socket The CAN socket to negotiate with.
 * @param isValid Written to contain the validity of the EEPROM.
 * @return 0 if successful, the error code otherwise.
 */
int canEepromIsValid (uint16_t canId, canSocket_t* socket, bool* isValid);

#endif // CAN_EEPROM_OPERATIONS_H