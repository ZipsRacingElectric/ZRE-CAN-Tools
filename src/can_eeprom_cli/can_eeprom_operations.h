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
// Command Message:                                   Response Message:
//   ----------------------------------------------     ----------------------------------------------
//   | Bit/Byte |  7  | 6 | 5 | 4 | 3 | 2 | 1 | 0 |     | Bit/Byte |  7  | 6 | 5 | 4 | 3 | 2 | 1 | 0 |
//   |----------|---------------------------------|     |----------|---------------------------------|
//   |    5     |           Data Byte 3           |     |    5     |           Data Byte 3           |
//   |----------|---------------------------------|     |----------|---------------------------------|
//   |    4     |           Data Byte 2           |     |    4     |           Data Byte 2           |
//   |----------|---------------------------------|     |----------|---------------------------------|
//   |    3     |           Data Byte 1           |     |    3     |           Data Byte 1           |
//   |----------|---------------------------------|     |----------|---------------------------------|
//   |    2     |           Data Byte 0           |     |    2     |           Data Byte 0           |
//   |----------|---------------------------------|     |----------|---------------------------------|
//   |    1     | R/W |   Data Address HI         |     |    1     | R/W |   Data Address HI         |
//   |----------|---------------------------------|     |----------|---------------------------------|
//   |    0     |         Data Address LO         |     |    0     |         Data Address LO         |
//   ----------------------------------------------     ----------------------------------------------
//   ID: eeprom.canId                                   ID: eeprom.canId + 1
//
// The DLC of the command and response frames indicates the size of the data payload.
//   DLC = 6 => 4 data bytes, DLC = 3 => 1 data bytes
//
// Data Address: The 15-bit byte address to read/write to.
//
// R/W: Indicates whether the operation is a read operation or write operation.
//   1 => Read, 0 => Write
//
// Write Operation:
//   To write to a memory address, a command message with the address, data, and R/W = 0 should be sent. For writes less than 4
//   bytes, the DLC of the command should be shortened to end the frame with the last data byte.
//
//   In response, the EEPROM will send a response message with the same address, data, R/W, and DLC. If any field of the
//   response does not match that of the command, or no response is received at all, the operation should be assumed to have
//   failed.
//
// Read Operation:
//   To read from a memory address, a command message with the address and R/W = 1 should be sent. The DLC of the command
//   should indicate the amount of data to be read. The in the data field of the command is irrelevant.
//
//   In response, the EEPROM will send a response message with the same address, R/W, and DLC. The data field will be populated
//   with the data that was read.

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

#endif // CAN_EEPROM_OPERATIONS_H