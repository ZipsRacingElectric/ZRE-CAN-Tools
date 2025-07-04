// Header
#include "can_signals.h"

// C Standard Library
#include <math.h>

// Functions ------------------------------------------------------------------------------------------------------------------

uint64_t signalEncode (canSignal_t* signal, float value)
{
	uint64_t buffer = roundf (value - signal->offset) / signal->scaleFactor;
	buffer &= signal->bitmask;
	buffer <<= signal->bitPosition;
	return buffer;
}

float signalDecode (canSignal_t* signal, uint64_t payload)
{
	payload >>= signal->bitPosition;
	payload &= signal->bitmask;

	// Perform sign extension
	bool negative = payload >> (signal->bitLength - 1) && signal->signedness;
	if (negative)
		payload |= ((uint64_t) -1) - signal->bitmask;

	float value = (signal->signedness ? (float) ((int64_t) payload) : (float) payload);
	return value * signal->scaleFactor + signal->offset;
}

// Standard I/O ---------------------------------------------------------------------------------------------------------------

void framePrint (FILE* stream, canFrame_t* frame)
{
	fprintf (stream, "%X\t[%u]\t", frame->id, frame->dlc);
	for (uint8_t index = 0; index < frame->dlc; ++index)
		fprintf (stream, "%2X ", frame->data [index]);
	fprintf (stream, "\n");
}

void messagePrint (FILE* stream, canMessage_t* message, uint8_t* data)
{
	fprintf (stream, "- Message %s (0x%X) -\n", message->name, message->id);

	for (size_t index = 0; index < message->signalCount; ++index)
		signalPrint (stream, &message->signals [index], *(uint64_t*) data);
}

canFrame_t messagePrompt (canMessage_t* message)
{
	canFrame_t frame;
	uint64_t* payload = (uint64_t*) frame.data;

	printf ("- Message %s (0x%X) -\n", message->name, message->id);

	for (size_t index = 0; index < message->signalCount; ++index)
		*payload |= signalPrompt (message->signals + index);

	return frame;
}

void signalPrint (FILE* stream, canSignal_t* signal, uint64_t payload)
{
	float value = signalDecode (signal, payload);
	fprintf (stream, "%s: %f\n", signal->name, value);
}

uint64_t signalPrompt (canSignal_t* signal)
{
	while (true)
	{
		printf ("%s: ", signal->name);

		float value;
		if (fscanf (stdin, "%f%*1[\n]", &value) == 1)
			return signalEncode (signal, value);

		printf ("Invalid value.\n");
	}
}