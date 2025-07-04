#ifndef CAN_SIGNALS_H
#define CAN_SIGNALS_H

// CAN Signals ----------------------------------------------------------------------------------------------------------------
//
// Author: Cole Barach
// Date Created: 2023.07.08
//
// Description: Datatypes and functions related CAN signals and messages.

// Includes -------------------------------------------------------------------------------------------------------------------

// Includes
#include "can_device/can_device.h"

// C Standard Library
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>

// Datatypes ------------------------------------------------------------------------------------------------------------------

// CAN signal forward declaration
typedef struct canSignal canSignal_t;

/**
 * @brief Structure representing a message broadcast on a CAN bus.
 */
typedef struct
{
	canSignal_t*	signals;
	size_t			signalCount;
	char*			name;
	uint32_t		id;
	uint8_t			dlc;
} canMessage_t;

/**
 * @brief Structure representing a signal present in a CAN message.
 */
struct canSignal
{
	canMessage_t*	message;
	char*			name;
	uint8_t			bitPosition;
	uint8_t			bitLength;
	float			scaleFactor;
	float			offset;
	bool			signedness;
	bool			endianness;
	uint64_t		bitmask;
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

// Standard I/O ---------------------------------------------------------------------------------------------------------------

// TODO(Barach): Docs
void framePrint (FILE* stream, canFrame_t* frame);

// TODO(Barach): Docs
void messagePrint (FILE* stream, canMessage_t* message, uint8_t* data);

// TODO(Barach): Docs
canFrame_t messagePrompt (canMessage_t* message);

/**
 * @brief Prints the decoded signal's data.
 * @param stream The stream to print to.
 * @param signal The signal to print.
 * @param payload The payload to decode from.
 */
void signalPrint (FILE* stream, canSignal_t* signal, uint64_t payload);

/**
 * @brief Prompts the user to input a value for the given signal.
 * @param signal The signal to prompt for.
 * @return The encoded payload.
 */
uint64_t signalPrompt (canSignal_t* signal);

#endif // CAN_SIGNALS_H