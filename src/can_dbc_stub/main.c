#include "can_device/can_device_stdio.h"
#include "can_database/can_dbc_v2.h"
#include "debug.h"

int main (int argc, char** argv)
{
	debugInit ();

	if (argc < 2)
		return -1;

	debugSetStream (stderr);

	canMessage_t* messages;
	size_t messageCount;

	canSignal_t* signals;
	size_t signalCount;

	size_t dbcCount = argc - 1;
	size_t* dbcMessageIndices = malloc (sizeof (size_t) * dbcCount);
	if (dbcMessageIndices == NULL)
		return errorPrintf ("Failed to allocate DBC message indices");

	if (canDbcsLoad (argv + 1, dbcCount, &messages, &messageCount, &signals, &signalCount, dbcMessageIndices) != 0)
		return errorPrintf ("Failed to load CAN DBC file");

	for (size_t messageIndex = 0; messageIndex < messageCount; ++messageIndex)
	{
		canMessage_t* message = &messages [messageIndex];

		printf ("Message %s: ID: ", message->name);
		fprintCanId (stdout, message->id, message->ide, false);
		printf (", DLC %u\n", message->dlc);

		for (size_t signalIndex = 0; signalIndex < message->signalCount; ++signalIndex)
		{
			canSignal_t* signal = &message->signals [signalIndex];
			printf (""
				"    Signal %s:\n"
				"        Bit length: %u\n"
				"        Bit position: %u\n"
				"        Scale factor: %f\n"
				"        Offset: %f\n"
				, signal->name, signal->bitLength, signal->bitPosition, signal->scaleFactor, signal->offset);
		}
	}

	canDbcsDealloc (messages, messageCount, signals);
}