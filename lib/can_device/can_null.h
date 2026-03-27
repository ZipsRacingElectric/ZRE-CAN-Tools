#ifndef NULL_H
#define NULL_H

// Null CAN Device ------------------------------------------------------------------------------------------------------------
//
// Author: Cole Barach
// Date Created: 2026.03.27
//
// Description: Dummy CAN device that does not connect to any real hardware. This can be used when a CAN device is required but
//   hardware isn't available.

// Includes -------------------------------------------------------------------------------------------------------------------

// Includes
#include "can_device.h"

// C Standard Library
#include <stdbool.h>
#include <stddef.h>

// Functions ------------------------------------------------------------------------------------------------------------------

/**
 * @brief Checks if a device name belongs to a null CAN device.
 * @param name The name to check.
 * @return True if the name is 'null', false otherwise.
 */
bool canNullNameDomain (const char* name);

/**
 * @brief Initializes a null CAN device.
 * @param name The name (handler) of the device. Note the serial port handler is OS-dependent.
 * @param baudrate The baudrate of the device.
 * @return The initialized null CAN device if successful, @c NULL otherwise.
 */
canDevice_t* canNullInit (char* name, canBaudrate_t baudrate);

/**
 * @brief De-allocates the memory owned by a null CAN device.
 * @param device The device to de-allocate.
 */
void canNullDealloc (void* device);

/// @brief Null implementation of the @c canTransmit function.
int canNullTransmit (void* device, canFrame_t* frame);

/// @brief Null implementation of the @c canReceive function.
int canNullReceive (void* device, canFrame_t* frame);

/// @brief Null implementation of the @c canFlushRx function.
int canNullFlushRx (void* device);

/// @brief Null implementation of the @c canSetTimeout function.
int canNullSetTimeout (void* device, unsigned long timeoutMs);

/// @brief Null implementation of the @c canGetBaudrate function.
canBaudrate_t canNullGetBaudrate (void* device);

/// @brief Null implementation of the @c canGetDeviceType function.
const char* canNullGetDeviceName (void* device);

/// @brief Null implementation of the @c canGetDeviceType function.
const char* canNullGetDeviceType (void);

#endif // NULL_H