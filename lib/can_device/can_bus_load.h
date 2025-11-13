#ifndef CAN_BUS_LOAD_H
#define CAN_BUS_LOAD_H

// CAN Bus Load Calculator ----------------------------------------------------------------------------------------------------
//
// Author: Cole Barach
// Date Created: 2025.11.13
//
// Description: Set of functions for calculating the load of a CAN bus. CAN bus load is defined as the percentage of time the
//   CAN bus is in use. In practice, this is estimated by measuring the number of bits received over a specific period of time.
//   This calculator provides two estimates: the maximum bus load and the minimum bus load. In practice, the maximum bus load
//   is a more useful estimate, however the minimum is provided for completeness.

// Includes -------------------------------------------------------------------------------------------------------------------

// Includes
#include "can_device.h"

// POSIX
#include <sys/time.h>

// C Standard Library
#include <stddef.h>

// Functions ------------------------------------------------------------------------------------------------------------------

/**
 * @brief Calculates the maximum number of bits in a CAN frame. This is used to calculate the maximum (or worst-case) load of a
 * CAN bus.
 * @param frame The frame to get the size of.
 * @return The maximum number of bits in the CAN frame. This value is not the exact size of the CAN frame, merely an upper
 * bound.
 */
size_t canGetMaxBitCount (canFrame_t* frame);

/**
 * @brief Calculates the minimum number of bits in a CAN frame. This is used to calculate the minimum (or best-case) load of a
 * CAN bus. Note this should only be used complementarily to the maximum bus load, calculated via the @c canGetMaxBitCount
 * function. The minimum bus load is not useful on its own.
 * @param frame The frame to get the size of.
 * @return The minimum number of bits in the CAN frame. This value is not the exact size of the CAN frame, merely a lower
 * bound.
 */
size_t canGetMinBitCount (canFrame_t* frame);

/**
 * @brief Calculates the bit time of a CAN bus based on its baudrate.
 * @param baudrate The baudrate of the CAN bus.
 * @return The bit time (amount of time to transmit a single bit) of the CAN bus, in seconds.
 */
float canCalculateBitTime (canBaudrate_t baudrate);

/**
 * @brief Calculates the load of a CAN bus based on the number of bits received over a period of time.
 * @param bitCount The number of received bits, may be either the min or max estimate.
 * @param bitTime The bit time of the CAN bus, as returned by @c canCalculateBitTime .
 * @param period The amount of time that has the @c bitCount was measured over.
 * @return The load of the CAN bus, 0 => 0%, 1 => 100%.
 */
float canCalculateBusLoad (size_t bitCount, float bitTime, struct timeval period);

#endif // CAN_BUS_LOAD_H