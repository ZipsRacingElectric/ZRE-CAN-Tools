#ifndef CAN_SIGNALS_H
#define CAN_SIGNALS_H

// CAN Signals ----------------------------------------------------------------------------------------------------------------
//
// Author: Cole Barach
// Date Created: 2023.07.08
//
// Description: Datatypes and functions related CAN signals and messages.

// Includes -------------------------------------------------------------------------------------------------------------------

// C Standard Library
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>

// Datatypes ------------------------------------------------------------------------------------------------------------------

// CAN signal forward declaration
typedef struct canSignal canSignal_t;

/**
 * @brief Structure representing the encoding of a message broadcast on a CAN bus.
 * @note This does not have a payload associated with it, just the encoding.
 */
typedef struct
{
	/// @brief The array of signals present in this message.
	canSignal_t* signals;

	/// @brief The number of elements in @c signals .
	size_t signalCount;

	/// @brief The name of this message.
	char* name;

	/// @brief The CAN ID of this message.
	uint32_t id;

	/// @brief The IDE bit of the CAN ID. Indicates whether the ID is a standard identifier or an extended identifier.
	bool ide;

	/// @brief The DLC of this message.
	uint8_t dlc;
} canMessage_t;

/**
 * @brief Structure representing the encoding of a signal present in a CAN message.
 * @note This does not have a value associated with it, just the encoding.
 */
struct canSignal
{
	/// @brief The message this signal belongs to.
	canMessage_t* message;

	/// @brief The name of this signal.
	char* name;

	/// @brief The position of the starting bit of this signal.
	uint8_t bitPosition;

	/// @brief The length of this signal, in bits.
	uint8_t bitLength;

	/// @brief The scale factor to apply to this signal.
	float scaleFactor;

	/// @brief The offset to apply to this signal.
	float offset;

	/// @brief The signedness of this signal. True => signed, false => unsigned.
	bool signedness;

	/// @brief The endianness of this signal. True => Intel format, false => Motorola format.
	bool endianness;

	/// @brief Bitmask for isolating this signal.
	uint64_t bitmask;
};

// Functions ------------------------------------------------------------------------------------------------------------------

/**
 * @brief Encodes the specified signal's data into a payload.
 * @param signal The signal to encode.
 * @param value The value of the signal.
 * @return The encoded payload.
 */
uint64_t signalEncode (canSignal_t* signal, float value);

/**
 * @brief Decodes the specified signal from a payload.
 * @param signal The signal to decoded.
 * @param payload The payload to decode from.
 * @return The decoded value.
 */
float signalDecode (canSignal_t* signal, uint64_t payload);

#endif // CAN_SIGNALS_H