#ifndef CAN_DEVICE_STDIO_H
#define CAN_DEVICE_STDIO_H

// CAN Device Standard Input/Output -------------------------------------------------------------------------------------------
//
// Author: Cole Barach
// Date Created: 2025.10.30
//
// Description: Helper functions for using standard input & output with the can_device library.

// Includes -------------------------------------------------------------------------------------------------------------------

// Includes
#include "can_device.h"

// C Standard Library
#include <stdio.h>

// Functions ------------------------------------------------------------------------------------------------------------------

/**
 * @brief Converts a string to a CAN ID. String should take one of the following formats:
 * "<Standard CAN ID>" or "<Extended CAN ID>x"
 * @param id Buffer to write the CAN ID into.
 * @param ide Buffer to write the IDE bit into.
 * @param str The string to parse.
 * @return 0 if successful, the error code otherwise.
 */
int strToCanId (uint32_t* id, bool* ide, const char* str);

/**
 * @brief Converts a string to a CAN frame. String should take the following format:
 * "<CAN ID>[<Byte 0>,<Byte 1>,...<Byte N>]"
 * @param frame Buffer to write the CAN frame into.
 * @param str The string to parse.
 * @return 0 if successful, the error code otherwise.
 */
int strToCanFrame (canFrame_t* frame, char* str);

/**
 * @brief Prints a CAN ID to an I/O stream. Printed as either:
 * "<Standard CAN ID>" or "<Extended CAN ID>x"
 * @param stream The stream to write to.
 * @param id The CAN ID.
 * @param ide The IDE bit.
 * @return The number of bytes written if successful, a negative value otherwise.
 */
int fprintCanId (FILE* stream, uint32_t id, bool ide);

/**
 * @brief Prints a CAN frame to an I/O stream. Printed as:
 * "<CAN ID>[<Byte 0>,<Byte 1>,...<Byte N>]"
 * @param stream The stream to write to.
 * @param frame The CAN frame to write.
 * @return The number of bytes written if succcessful, a negative value otherwise.
 */
int fprintCanFrame (FILE* stream, canFrame_t* frame);

#endif // CAN_DEVICE_STDIO_H