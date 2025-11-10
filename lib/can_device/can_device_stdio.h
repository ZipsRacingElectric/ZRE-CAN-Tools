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
 * @param rtr Buffer to write the RTR bit into.
 * @param str The string to parse.
 * @return 0 if successful, the error code otherwise.
 */
int strToCanId (uint32_t* id, bool* ide, bool* rtr, const char* str);

/**
 * @brief Converts a string to a CAN frame. String should take the following format:
 * "<CAN ID>[<Byte 0>,<Byte 1>,...<Byte N>]"
 * @note The @c str parameter is modified by this function. A copy of the string must be made if the original need be
 * preserved.
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
 * @param rtr The RTR bit.
 * @return The number of bytes written if successful, a negative value otherwise.
 */
int fprintCanId (FILE* stream, uint32_t id, bool ide, bool rtr);

/**
 * @brief Prints a CAN frame to an I/O stream. Printed as:
 * "<CAN ID>[<Byte 0>,<Byte 1>,...<Byte N>]"
 * @param stream The stream to write to.
 * @param frame The CAN frame to write.
 * @return The number of bytes written if successful, a negative value otherwise.
 */
int fprintCanFrame (FILE* stream, canFrame_t* frame);

/**
 * @brief Prints CAN ID into an existing string. Printed as either:
 * "<Standard CAN ID>" or "<Extended CAN ID>x"
 * @param str The string to print into.
 * @param n The size of the buffer to write into, including termination. At most @c n-1 characters are written, plus
 * terminator.
 * @param id The CAN ID.
 * @param ide The IDE bit.
 * @param rtr The RTR bit.
 * @return The number of characters that would be written to @c str , regardless of buffer size, or a negative value if
 * unsuccessful. Only when this returned value is non-negative and less than @c n has the string has been completely written.
 */
int snprintCanId (char* str, size_t n, uint32_t id, bool ide, bool rtr);

/**
 * @brief Prints the help page for the CAN device name handle.
 * @param stream The I/O stream to print to.
 * @param indent The indentation to put before each line. Note that spaces are preferred to tabs.
 * @return The number of bytes written if successful, a negative value otherwise.
 */
int fprintCanDeviceNameHelp (FILE* stream, const char* indent);

/**
 * @brief Prints the help page for the CAN ID parameter (input string to @c strToCanId ).
 * @param stream The I/O stream to print to.
 * @param indent The indentation to put before each line. Note that spaces are preferred to tabs.
 * @return The number of bytes written if successful, a negative value otherwise.
 */
int fprintCanIdHelp (FILE* stream, const char* indent);

/**
 * @brief Prints the help page for the CAN frame parameter (input string to @c strToCanFrame ).
 * @param stream The I/O stream to print to.
 * @param indent The indentation to put before each line. Note that spaces are preferred to tabs.
 * @return The number of bytes written if successful, a negative value otherwise.
 */
int fprintCanFrameHelp (FILE* stream, const char* indent);

#endif // CAN_DEVICE_STDIO_H