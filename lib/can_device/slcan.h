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
//
// Unsupported features:
// - SerialCAN library does not implement CAN bus error detection or error frames. If a bus error occurs while trying to
//   receive a CAN frame, the canReceive function will simply ignore it and continue waiting.

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
 * @brief Checks whether a SLCAN device name indicates enumeration should be performed.
 * @param name The name to check.
 * @return True if enumeration should be performed, false otherwise.
 */
bool slcanWildcard (const char* name);

/**
 * @brief Initializes an SLCAN device.
 * @param name The name (handler) of the device. Note the serial port handler is OS-dependent.
 * @param baudrate The baudrate of the device.
 * @return The initialized SLCAN device if successful, @c NULL otherwise.
 */
canDevice_t* slcanInit (char* name, canBaudrate_t baudrate);

/** 
 * @brief Enumerates and initializes all possible SLCAN devices. This returns an array of initialized SLCAN devices. The unused
 * devices should be deallocated via @c slcanDealloc .
 * @param baudrate The baudrate of the device.
 * @param deviceCount Buffer to write the number of detected devices into.
 * @return A dynamically allocated array of SLCAN devices of size @c deviceCount if successful, @c NULL otherwise.
 */
canDevice_t** slcanEnumerate (canBaudrate_t baudate, size_t* deviceCount);

/**
 * @brief Selects the index of an SLCAN device from a list of SLCAN devices via a command prompt. 
 * @param devices A list of SLCAN devices.
 * @param deviceCount The number of SLCAN devices.
 * @return The selected SLCAN device's index within the list.
 */
size_t slcanSelectDevice (canDevice_t** devices, size_t deviceCount); 

/**
 * @brief De-allocates the memory owned by an SLCAN device.
 * @param device The device to de-allocate.
 */
void slcanDealloc (void* device);

/// @brief SLCAN implementation of the @c canTransmit function.
int slcanTransmit (void* device, canFrame_t* frame);

/// @brief SLCAN implementation of the @c canReceive function.
int slcanReceive (void* device, canFrame_t* frame);

/// @brief SLCAN implementation of the @c canFlushRx function.
int slcanFlushRx (void* device);

/// @brief SLCAN implementation of the @c canSetTimeout function.
int slcanSetTimeout (void* device, unsigned long timeoutMs);

/// @brief SLCAN implementation of the @c canGetBaudrate function.
canBaudrate_t slcanGetBaudrate (void* device);

/// @brief SLCAN implementation of the @c canGetDeviceType function.
const char* slcanGetDeviceName (void* device);

/// @brief SLCAN implementation of the @c canGetDeviceType function.
const char* slcanGetDeviceType (void);

#endif // SERIAL_CAN_H