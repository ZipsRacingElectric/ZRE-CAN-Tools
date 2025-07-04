// CAN Device CLI -------------------------------------------------------------------------------------------------------------
//
// Author: Cole Barach
// Date Created: 2025.07.04
//
// Description: TODO(Barach):

// Includes -------------------------------------------------------------------------------------------------------------------

// Includes
#include "can_device/can_device.h"
#include "error_codes.h"

// C Standard Library
#include <errno.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

// Functions ------------------------------------------------------------------------------------------------------------------

void promptFrame (canFrame_t* frame)
{
	char buffer [512];

	printf ("Enter the ID of the message: ");
	fgets (buffer, sizeof (buffer), stdin);
	buffer [strcspn (buffer, "\r\n")] = '\0';
	frame->id = strtol (buffer, NULL, 0);

	printf ("Enter the DLC of the message: ");
	fgets (buffer, sizeof (buffer), stdin);
	buffer [strcspn (buffer, "\r\n")] = '\0';
	frame->dlc = strtol (buffer, NULL, 0);

	for (uint8_t index = 0; index < frame->dlc; ++index)
	{
		printf ("Enter byte %i of the payload: ", index);
		fgets (buffer, sizeof (buffer), stdin);
		buffer [strcspn (buffer, "\r\n")] = '\0';
		frame->data [index] = strtol (buffer, NULL, 0);
	}
}

void printFrame (canFrame_t* frame)
{
	printf ("0x%3X : ", frame->id);

	for (uint8_t index = 0; index < frame->dlc; ++index)
		printf ("[%02X]", frame->data [index]);

	printf ("\n");
}

// Entrypoint -----------------------------------------------------------------------------------------------------------------

int main (int argc, char** argv)
{
	if (argc != 2)
	{
		fprintf (stderr, "Format: can-dev-cli <device name>\n");
		return -1;
	}

	const char* deviceName = argv [1];

	canDevice_t* device = canInit (deviceName);
	if (device == NULL)
	{
		int code = errno;
		fprintf (stderr, "Failed to create CAN device: %s.\n", errorMessage (code));
		return code;
	}

	while (true)
	{
		canFrame_t frame;
		char selection;
		printf ("Enter an option:\n");
		printf (" t - Transmit a CAN message.\n");
		printf (" r - Receive a CAN message.\n");
		printf (" f - Flush the receive buffer.\n");
		printf (" q - Quit the program.\n");

		fscanf (stdin, "%c%*1[\n]", &selection);
		switch (selection)
		{
		case 't':
			promptFrame (&frame);
			if (canTransmit (device, &frame) == 0)
				printf ("Success.\n");
			else
			 	printf ("Error: %s.\n", errorMessage (errno));
			break;
		case 'r':
			if (canReceive (device, &frame) == 0)
				printFrame (&frame);
			else
				printf ("Error: %s.\n", errorMessage (errno));
			break;
		case 'f':
			if (canFlushRx (device) != 0)
				printf ("Error: %s.\n", errorMessage (errno));
			break;
		case 'q':
			return 0;
		}
	};
}