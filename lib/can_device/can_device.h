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
	/// @brief The ID of the CAN message, may be either a SID (standard identifier) or EID (extended identifier). Test the
	/// @c ide field to determine which type it is.
	uint32_t id;

	/// @brief The IDE (extended ID) bit of the message. Indicates the ID is an extended CAN ID rather than a standard CAN ID.
	bool ide;

	/// @brief The payload of the message. Note that only @c dlc elements are used.
	uint8_t data [8];

	/// @brief The DLC (data length code) of the message.
	uint8_t dlc;

	/// @brief The RTR (remote transmission request) bit of the message. Indicates a request for data, rather than a frame
	/// containing data.
	/// @note Not all can devices support this, for said devices, a request to transmit an RTR frame will return an error, when
	/// an RTR frame is received, it will be ignored.
	bool rtr;
} canFrame_t;

/// @brief Indicates a baudrate is not known.
#define CAN_BAUDRATE_UNKNOWN 0

/// @brief Datatype representing a CAN bus's baudrate.
typedef unsigned int canBaudrate_t;

/// @brief Function signature for the @c canTransmit function.
typedef int canTransmit_t (void* device, canFrame_t* frame);

/// @brief Function signature for the @c canReceive function.
typedef int canReceive_t (void* device, canFrame_t* frame);

/// @brief Function signature for the @c canFlushRx function.
typedef int canFlushRx_t (void* device);

/// @brief Function signature for the @c canSetTimeout function.
typedef int canSetTimeout_t (void* device, unsigned long timeoutMs);

/// @brief Function signature for the @c canGetBaudrate function.
typedef canBaudrate_t canGetBaudrate_t (void* device);

/// @brief Function signature for the @c canGetDeviceName function.
typedef const char* canGetDeviceName_t (void* device);

/// @brief Function signature for the @c canGetDeviceType function.
typedef const char* canGetDeviceType_t (void);

/// @brief Function signature for the @c canDealloc function.
typedef void canDealloc_t (void* device);

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

	/// @brief A device's specific implementation of the @c canGetBaudrate function.
	canGetBaudrate_t* getBaudrate;

	/// @brief A device's specific implementation of the @c canGetDeviceName function.
	canGetDeviceName_t* getDeviceName;

	/// @brief A device's specific implementation of the @c canGetDeviceType function.
	canGetDeviceType_t* getDeviceType;

	/// @brief A device's specific implementation of the @c canDealloc function.
	canDealloc_t* dealloc;
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
 * @brief Identifies and initializes a CAN device based on its name handle. This function will attempt to identify the type of
 * adapter based on context in the provided name.
 * @param deviceName The name of the CAN device. Note this should either be a SocketCAN name or an SLCAN name.
 * @return The initialized CAN device if successful, @c NULL otherwise. Note @c errno is set on failure.
 */
canDevice_t* canInit (char* deviceName);

/**
 * @brief Closes and deallocates and CAN device. The @c device pointer is no longer usable after a call to this function.
 * @param device The device to deallocate.
 */
static inline void canDealloc (canDevice_t* device)
{
	device->vmt.dealloc (device);
}

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
 * @param frame The buffer to receive the frame into.
 * @return 0 if successful, the error code otherwise. If the error code, as tested by @c canCheckBusError indicates an error
 * frame was generated, said frame is written to the @c frame buffer. Note @c errno is also set on failure.
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
 * @brief Gets the baudrate of a CAN device.
 * @param device The device to get from.
 * @return The baudrate of the CAN bus, if known. @c CAN_BAUDRATE_UNKNOWN otherwise.
 */
static inline canBaudrate_t canGetBaudrate (canDevice_t* device)
{
	return device->vmt.getBaudrate (device);
}

/**
 * @brief Gets the name of a CAN device.
 * @param device The device to get the name of.
 * @return The name of the device.
 */
static inline const char* canGetDeviceName (canDevice_t* device)
{
	return device->vmt.getDeviceName (device);
}

/**
 * @brief Gets a string identifying the type of a CAN device. Note this only be used for debugging and record keeping. No
 * specific programming behavior should be based on this return value.
 * @param device The device to get the type of.
 * @return A user friendly string identifying the type of the device.
 */
static inline const char* canGetDeviceType (canDevice_t* device)
{
	return device->vmt.getDeviceType ();
}

/**
 * @brief Checks whether a given return code corresponds to a CAN bus error.
 * @param code The error code, as returned from the function or read from @c errno .
 * @return True if the error is specific to the CAN bus, false otherwise. If true, this indicates an error frame was generated.
 */
bool canCheckBusError (int code);

/**
 * @brief Gets a string describing the type of CAN bus error that occurred.
 * @param code The error code, as returned from the function or read from @c errno . Note this error code should first be
 * tested by @c canCheckBusError to determine if it is a CAN bus related error.
 * @return The shorthand string naming the error.
 */
char* canGetBusErrorName (int code);

#endif // CAN_DEVICE_H