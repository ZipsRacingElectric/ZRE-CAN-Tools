#ifndef SOCKET_CAN_H
#define SOCKET_CAN_H

// SocketCAN ------------------------------------------------------------------------------------------------------------------
//
// Author: Cole Barach
// Date Created: 2023.07.08
//
// Description: An interface for CAN devices based on the Linux SocketCAN implementation.
//
// References:
// - https://www.kernel.org/doc/html/latest/networking/can.html
// - https://en.wikipedia.org/wiki/SocketCAN
// - https://www.man7.org/linux/man-pages/man2/socket.2.html
// - https://www.man7.org/linux/man-pages/man7/netdevice.7.html
// - https://www.gnu.org/software/libc/manual/html_node/Interface-Naming.html
// - https://www.can-cia.org/fileadmin/resources/documents/proceedings/2012_hartkopp.pdf
// - https://github.com/linux-can/can-utils/tree/master

// Includes -------------------------------------------------------------------------------------------------------------------

// Includes
#include "can_device.h"

// C Standard Library
#include <stdbool.h>

// Functions ------------------------------------------------------------------------------------------------------------------

/**
 * @brief Checks if a device name belongs to a SocketCAN device.
 * @param name The name to check.
 * @return True if the name begins with 'can' or 'vcan', false otherwise.
 */
bool socketCanNameDomain (const char* name);

/**
 * @brief Initializes a SocketCAN device.
 * @param name The name (handler) of the device.
 * @return The initialized device if successful, @c NULL otherwise.
 */
canDevice_t* socketCanInit (const char* name);

/**
 * @brief De-allocates the memory owned by SocketCAN device.
 * @param device The device to de-allocate.
 * @return 0 if successful, the error code otherwise.
 */
int socketCanDealloc (void* device);

/// @brief SocketCAN implementation of the @c canTransmit function.
int socketCanTransmit (void* device, canFrame_t* frame);

/// @brief SocketCAN implementation of the @c canReceive function.
int socketCanReceive (void* device, canFrame_t* frame);

/// @brief SocketCAN implementation of the @c canFlushRx function.
int socketCanFlushRx (void* device);

/// @brief SocketCAN implementation of the @c canSetTimeout function.
int socketCanSetTimeout (void* device, unsigned long timeoutMs);

#endif // SOCKET_CAN_H