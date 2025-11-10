#ifndef CAN_DEVICE_H
#define CAN_DEVICE_H

// CAN Device Interface -------------------------------------------------------------------------------------------------------
//
// Author: Cole Barach
// Date Created: 2025.07.04
//
// Description: Objects and functions for interacting with CAN devices (also called CAN adapters, CAN interfaces, etc.).

// Includes -------------------------------------------------------------------------------------------------------------------

// C Standard Library
#include <stdint.h>

// Datatypes ------------------------------------------------------------------------------------------------------------------

/// @brief Structure representing a CAN message, also called a CAN frame.
typedef struct
{
	/// @brief The ID of the CAN message may be either SID or EID.
	uint32_t id;

	/// @brief The payload of the message. Note that only @c dlc elements are used.
	uint8_t data [8];

	/// @brief The DLC (data length code) of the message.
	uint8_t dlc;
} canFrame_t;

/// @brief Function signature for the @c canTransmit function.
typedef int canTransmit_t (void* device, canFrame_t* frame);

/// @brief Function signature for the @c canReceive function.
typedef int canReceive_t (void* device, canFrame_t* frame);

/// @brief Function signature for the @c canFlushRx function.
typedef int canFlushRx_t (void* device);

/// @brief Function signature for the @c canSetTimeout function.
typedef int canSetTimeout_t (void* device, unsigned long timeoutMs);

/**
 * @brief Virtual method table for the @c canDevice_t structure.
 */
typedef struct
{
	/// @brief A device's specific implementation of the @c canTransmit function.
	canTransmit_t* transmit;

	/// @brief A device's specific implementation of the @c canReceive function.
	canReceive_t* receive;

	/// @brief A device's specific implementation of the @c canFlushRx function.
	canFlushRx_t* flushRx;

	/// @brief A device's specific implementation of the @c canSetTimeout function.
	canSetTimeout_t* setTimeout;
} canDeviceVmt_t;

/**
 * @brief Polymorphic object representing a CAN device. This structure defines an abstract object, that is, it is not
 * instantiable. Rather than being instanced, this structure defines an interface for implementations of a CAN device.
 */
typedef struct
{
	canDeviceVmt_t vmt;
} canDevice_t;

// Functions ------------------------------------------------------------------------------------------------------------------

/**
 * @brief Function for transmitting a CAN frame.
 * @param device The device to transmit with.
 * @param frame The frame to transmit.
 * @return 0 if successful, the error code otherwise.
 */
#define canTransmit(device, frame)						\
	(device)->vmt.transmit (device, frame)

/**
 * @brief Function for receiving a CAN frame.
 * @param device The device to receive from.
 * @param frame The frame to receive.
 * @return 0 if successful, the error code otherwise.
 */
#define canReceive(device, frame)						\
	(device)->vmt.receive (device, frame)

/**
 * @brief Function for 'flushing' the receive buffer of a CAN device. This is used to indicate all previously received messages
 * should be disposed rather than returned in the next call to @c canReceive .
 * @param device The device to flush.
 * @return 0 if successful, the error code otherwise.
 */
#define canFlushRx(device)								\
	(device)->vmt.flushRx (device)

/**
 * @brief Function for setting the timeout period of a CAN device. This timeout determines the maximum amount of time a call to
 * @c canReceive will block for.
 * @param device The device to set the timeout of.
 * @param timeoutMs The timeout interval, in milliseconds. Use 0 to indicate no timeout should be used (all calls are
 * blocking).
 * @return 0 if successful, the error code otherwise.
 */
#define canSetTimeout(device, timeoutMs)				\
	(device)->vmt.setTimeout (device, timeoutMs)

/**
 * @brief Identifies and initializes a CAN device based on its name handle. This function will attempt to identify the type of
 * adapter based on context in the provided name.
 * @param name The name (handler) of the CAN device. Note this should either be a SocketCAN name or an SLCAN name.
 * @return The initialized CAN device if successful, @c NULL otherwise. Note @c errno is set on failure.
 */
canDevice_t* canInit (char* name);

/**
 * @brief Initialize a CAN device by iterating through a list of device names. 
 * @param names A list of device names.
 * @param count The number of elements in the list of device names.  
 * @return The first initialized CAN device if successful, @c NULL otherwise. Note @c errno is set on failure.
 */
canDevice_t* findCanDevice (char** names, size_t count);

#endif // CAN_DEVICE_H