#ifndef SERIAL_CAN_H
#define SERIAL_CAN_H

// SLCAN ----------------------------------------------------------------------------------------------------------------------
//
// Author: Cole Barach
// Date Created: 2025.07.04
//
// Description: An interface for CAN devices based on the SLCAN standard. Note that this is merely a wrapper for the SerialCAN
//   library
//
// References:
// - https://github.com/mac-can/SerialCAN

// Includes -------------------------------------------------------------------------------------------------------------------

// Includes
#include "can_device.h"

// C Standard Library
#include <stdbool.h>
#include <stddef.h>

// Functions ------------------------------------------------------------------------------------------------------------------

/**
 * @brief Checks if a device name belongs to a SLCAN device.
 * @param name The name to check.
 * @return True if the name begins with '/dev/tty' or 'COM', false otherwise.
 */
bool slcanNameDomain (const char* name);

/**
 * @brief Initializes an SLCAN device.
 * @param name The name (handler) of the device. This handle must also include the baudrate to initialize to, in the following
 * form: <Serial Port>@<Baudrate>. Note the serial port handler is OS-dependent.
 * @return The initialized SLCAN device if successful, @c NULL otherwise.
 */
canDevice_t* slcanInit (char* name);

/**
 * @brief De-allocates the memory owned by an SLCAN device.
 * @param device The device to de-allocate.
 * @return 0 if successful, the error code otherwise.
 */
int slcanDealloc (void* device);

/// @brief SLCAN implementation of the @c canTransmit function.
int slcanTransmit (void* device, canFrame_t* frame);

/// @brief SLCAN implementation of the @c canReceive function.
int slcanReceive (void* device, canFrame_t* frame);

/// @brief SLCAN implementation of the @c canFlushRx function.
int slcanFlushRx (void* device);

/// @brief SLCAN implementation of the @c canSetTimeout function.
int slcanSetTimeout (void* device, unsigned long timeoutMs);

/** 
 * @brief Enumerates connected devices and assigns the first one to the deviceName argument.
 * @param deviceName The argument to be assigned the device's name.
 * @param baudRate Specifies the baud rate for the device.
 */ 
int slcanEnumerateDevice (char* deviceName, char* baudRate);

#endif // SERIAL_CAN_H
