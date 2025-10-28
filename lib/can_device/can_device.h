#ifndef CAN_DEVICE_H
#define CAN_DEVICE_H

// CAN Device Interface -------------------------------------------------------------------------------------------------------
//
// Author: Cole Barach
// Date Created: 2025.07.04
//
// Description: Objects and functions for interacting with CAN devices (also called CAN adapters, CAN interfaces, etc.).

// TODO(Barach): Could consider replacing SLCAN with Candlelight API?

// Includes -------------------------------------------------------------------------------------------------------------------

// Includes
#include "error_codes.h"

// C Standard Library
#include <stdint.h>
#include <stdbool.h>

// Datatypes ------------------------------------------------------------------------------------------------------------------

/// @brief Structure representing a CAN message, also called a CAN frame.
typedef struct
{
	/// @brief The ID of the CAN message, may be either a SID (standard identifier) or EID (extended identifier).
	uint32_t id;

	// TODO(Barach): can_database needs to correctly parse the IDE bit for this to work.
	/// @brief The IDE (extended ID) bit of the message. Indicates the ID is an extended CAN ID rather than a standard CAN ID.
	// bool ide;

	/// @brief The payload of the message. Note that only @c dlc elements are used.
	uint8_t data [8];

	/// @brief The DLC (data length code) of the message.
	uint8_t dlc;

	// TODO(Barach): This is going to break a lot of things.
	/// @brief The RTR (remote transmission request) bit of the message. Indicates a request for data, rather than a frame
	/// containing data.
	// bool rtr;
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

// TODO(Barach): This needs to store baudrate
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
 * @return 0 if successful, the error code otherwise. Note @c errno is set on failure.
 */
static inline int canTransmit (canDevice_t* device, canFrame_t* frame)
{
	return device->vmt.transmit (device, frame);
}

/**
 * @brief Function for receiving a CAN frame.
 * @param device The device to receive from.
 * @param frame The frame to receive.
 * @return 0 if successful, the error code otherwise. Note @c errno is set on failure. TODO(Barach): Docs
 */
static inline int canReceive (canDevice_t* device, canFrame_t* frame)
{
	return device->vmt.receive (device, frame);
}

/**
 * @brief Function for 'flushing' the receive buffer of a CAN device. This is used to indicate all previously received messages
 * should be disposed rather than returned in the next call to @c canReceive .
 * @param device The device to flush.
 * @return 0 if successful, the error code otherwise. Note @c errno is set on failure.
 */
static inline int canFlushRx (canDevice_t* device)
{
	return device->vmt.flushRx (device);
}

/**
 * @brief Function for setting the timeout period of a CAN device. This timeout determines the maximum amount of time a call to
 * @c canReceive will block for.
 * @param device The device to set the timeout of.
 * @param timeoutMs The timeout interval, in milliseconds. Use 0 to indicate no timeout should be used (all calls are
 * blocking).
 * @return 0 if successful, the error code otherwise. Note @c errno is set on failure.
 */
static inline int canSetTimeout (canDevice_t* device, unsigned long timeoutMs)
{
	return device->vmt.setTimeout (device, timeoutMs);
}

/**
 * @brief Identifies and initializes a CAN device based on its name handle. This function will attempt to identify the type of
 * adapter based on context in the provided name.
 * @param name The name (handler) of the CAN device. Note this should either be a SocketCAN name or an SLCAN name.
 * @return The initialized CAN device if successful, @c NULL otherwise. Note @c errno is set on failure.
 */
canDevice_t* canInit (char* name);

static inline bool canIsErrorFrame (int code)
{
	return
		code == ERRNO_CAN_DEVICE_BIT_ERROR ||
		code == ERRNO_CAN_DEVICE_BIT_STUFF_ERROR ||
		code == ERRNO_CAN_DEVICE_FORM_ERROR ||
		code == ERRNO_CAN_DEVICE_ACK_ERROR ||
		code == ERRNO_CAN_DEVICE_CRC_ERROR ||
		code == ERRNO_CAN_DEVICE_BUS_OFF ||
		code == ERRNO_CAN_DEVICE_UNSPEC_ERROR;
}

static inline char* canErrorFrameShorthand (int code)
{
	if (code == ERRNO_CAN_DEVICE_BIT_ERROR)
		return "BIT ERROR";

	if (code == ERRNO_CAN_DEVICE_BIT_STUFF_ERROR)
		return "BIT STUFF ERROR";

	if (code == ERRNO_CAN_DEVICE_FORM_ERROR)
		return "FORM ERROR";

	if (code == ERRNO_CAN_DEVICE_ACK_ERROR)
		return "ACK ERROR";

	if (code == ERRNO_CAN_DEVICE_CRC_ERROR)
		return "CRC ERROR";

	if (code == ERRNO_CAN_DEVICE_BUS_OFF)
		return "BUS-OFF ERROR";

	return "UNSPECIFIED ERROR";
}

#endif // CAN_DEVICE_H