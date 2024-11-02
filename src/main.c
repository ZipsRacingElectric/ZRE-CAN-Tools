// CAN Command-Line Interface -------------------------------------------------------------------------------------------------
//
// Author: Cole Barach
// Date Created: 2024.10.27
//
// Description: 

// Includes -------------------------------------------------------------------------------------------------------------------

// Includes
#include "can_dbc.h"
#include "can_socket.h"
#include "vcu.h"

// C Standard Library
#include <stdio.h>
#include <string.h>

// Prompts --------------------------------------------------------------------------------------------------------------------

canMessage_t* messageNamePrompt (canMessage_t* messages, size_t messageCount)
{
	char buffer [512];

	while (true)
	{
		printf ("Enter the name of the message: ");
		fgets (buffer, sizeof (buffer), stdin);
		buffer [strcspn (buffer, "\r\n")] = '\0';

		for (uint8_t index = 0; index < messageCount; ++index)
			if (strcmp (buffer, messages [index].name) == 0)
				return messages + index;

		printf ("Invalid name.\n");
	}
}

void messageTransmitPrompt (canSocket_t* canSocket, canMessage_t* messages, size_t messageCount)
{
	canMessage_t* message = messageNamePrompt (messages, messageCount);
	struct can_frame frame = messagePrompt (message);
	
	if (canSocketTransmit (canSocket, &frame))
		printf ("Success.\n");
	else
		printf ("Failed.\n");
}

void messageReceivePrompt (canSocket_t* canSocket, canMessage_t* messages, size_t messageCount)
{
	canMessage_t* message = messageNamePrompt (messages, messageCount);

	while (true)
	{
		struct can_frame frame;
		if (canSocketReceive(canSocket, &frame) && frame.can_id == message->id)
		{
			messagePrint (message, &frame);
			return;
		}
	}
}

void messagesPrint (canMessage_t* messages, size_t messageCount)
{
	for (size_t index = 0; index < messageCount; ++index)
		printf ("%s\n", messages [index].name);
}

// Entrypoint -----------------------------------------------------------------------------------------------------------------

int main (int argc, char** argv)
{
	if (argc != 3)
	{
		printf ("Format: can-cli <device name> <DBC file path>\n");
		return -1;
	}

	const char* deviceName = argv [1];
	const char* dbcPath = argv [2];

	canMessage_t messages [128];
	canSignal_t signals [512];

	size_t messageCount	= sizeof (messages)	/ sizeof (canMessage_t);
	size_t signalCount	= sizeof (signals)	/ sizeof (canSignal_t);
	dbcFileParse (dbcPath, messages, &messageCount, signals, &signalCount);

	canSocket_t can0;
	canSocketInit (&can0, deviceName);

	while (true)
	{
		char selection;
		printf ("Enter an option:\n");
		printf (" t - Transmit a CAN message.\n");
		printf (" r - Receive a CAN message.\n");
		printf (" p - Print a list of all messages.\n");
		printf (" v - Program the VCU's EEPROM.\n");
		printf (" q - Quit the program.\n");

		struct can_frame frame;

		fscanf (stdin, "%c%*1[\n]", &selection);
		switch (selection)
		{
		case 't':
			messageTransmitPrompt (&can0, messages, messageCount);
			break;
		case 'r':
			messageReceivePrompt (&can0, messages, messageCount);
			break;
		case 'p':
			messagesPrint (messages, messageCount);
			break;
		case 'v':
			frame = vcuEepromMessagePrompt ();
			canSocketTransmit (&can0, &frame);
			break;
		case 'q':
			return 0;
		}
	};
}