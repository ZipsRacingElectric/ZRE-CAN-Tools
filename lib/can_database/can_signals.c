// Header
#include "can_signals.h"

// C Standard Library
#include <math.h>

// Functions ------------------------------------------------------------------------------------------------------------------

uint64_t signalEncode (canSignal_t* signal, float value)
{
	// TODO(Barach): This doesn't respect endianness
	uint64_t buffer = roundf (value - signal->offset) / signal->scaleFactor;
	buffer &= signal->bitmask;
	buffer <<= signal->bitPosition;
	return buffer;
}

float signalDecode (canSignal_t* signal, uint64_t payload)
{
	payload >>= signal->bitPosition;
	payload &= signal->bitmask;

	// Reverse signal bytes if motorola formatting
	if (!signal->endianness && signal->bitLength > 8)
	{
		uint64_t reversed = 0;
		for (int index = 0; index < signal->bitLength / 8; ++index)
			reversed |= ((payload >> (index * 8)) & 0xFF) << (signal->bitLength - 8 - index * 8);
		payload = reversed;
	}

	// Perform sign extension
	bool negative = payload >> (signal->bitLength - 1) && signal->signedness;
	if (negative)
		payload |= ((uint64_t) -1) - signal->bitmask;

	float value = (signal->signedness ? (float) ((int64_t) payload) : (float) payload);
	return value * signal->scaleFactor + signal->offset;
}