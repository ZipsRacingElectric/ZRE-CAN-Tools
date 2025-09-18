// CAN Device CLI -------------------------------------------------------------------------------------------------------------
//
// Author: Cole Barach
// Date Created: 2025.07.04
//
// Description: Command-line interface for controlling a CAN adapter.

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

void displayHelp() 
{
	// TODO(DiBacco): create a manual page for the can-dev-cli command
}

void printFrame (canFrame_t* frame, int* canIds, int canIdsSize)
{
	// Base-10 representations: 512, 513, 514, 515, 1536, 1537
	// r=[512][513][514][515][1536][1537]
	for (int i = 0; i < canIdsSize; i++) {
		if (canIds[i] == frame->id) {
			printf ("0x%3X : ", frame->id);
			for (uint8_t index = 0; index < frame->dlc; ++index) 
			{
				printf ("[%02X]", frame->data [index]);
			}

			printf ("\n");
			return;
		}
	}
}

unsigned long int promptTimeout ()
{
	char buffer [512];

	printf ("Timeout (ms): ");
	fgets (buffer, sizeof (buffer), stdin);
	buffer [strcspn (buffer, "\r\n")] = '\0';
	return strtol (buffer, NULL, 0);
}

// Entrypoint -----------------------------------------------------------------------------------------------------------------

int main (int argc, char** argv)
{
	if (argc < 2)
	{
		fprintf (stderr, "Format: can-dev-cli <device name>\n");
		return -1;
	}

	char* deviceName = argv [argc - 1];

	// Check for query mode
	// TODO(Barach): This is pretty messy.
	bool queryMode = false;
	for (int index = 1; index < argc - 1; ++ index)
	{
		if (strcmp (argv [index], "-q") == 0)
			queryMode = true;
	}

	canDevice_t* device = canInit (deviceName);
	if (device == NULL)
	{
		int code = errno;
		fprintf (stderr, "Failed to create CAN device: %s.\n", errorMessage (code));
		return code;
	}

	// If this is query mode, return successful
	if (queryMode)
		return 0;

	while (true)
	{
		canFrame_t frame;
		char* command = malloc (100); // TODO (DiBacco): change malloc to an appropriate size 
		long unsigned int timeoutMs;
		
		printf ("Enter an option:\n");
		printf (" t - Transmit a CAN message.\n");
		printf (" r - Receive a CAN message.\n");
		printf (" f - Flush the receive buffer.\n");
		printf (" m - Set the device's timeout.\n");
		printf (" q - Quit the program.\n");

		fscanf (stdin, "%s%*1[\n]", command);

		int equalSignIndex = strcspn (command, "=");
		if ( ! (equalSignIndex == strlen (command) || equalSignIndex == 1)) 
		{
			printf ("Error: Invalid Command Format \n");
			continue;
		}
		
		// TODO(DiBacco): add filters to detect invalid input
		char method = command[0];
		command += 2;
			
		switch (method)
		{
		case 't': {
			promptFrame (&frame);
			if (canTransmit (device, &frame) == 0)
				printf ("Success.\n");
			else
			 	printf ("Error: %s.\n", errorMessage (errno));
			break;
		}
		case 'r': {
			// TODO(DiBacco): add base-10 / base-16 functionality
			// TODO(DiBacco): implement a looping command
			int canIdIndex = 0;
			int canIdLength = 0;
			const char* openBracketCharacters = "([{<";
			const char* closeBracketCharacters = ")]}>";

			for (int i = 0; i < strlen(command); i++) {
				if (strchr (openBracketCharacters, command[i]) != NULL) {
					canIdLength++;
				}
			}

			int canIds[canIdLength];
			
			while (true) {
				int x = strcspn (command, openBracketCharacters);
				int y = strcspn (command, closeBracketCharacters);

				int counter = 0;
				int delta = y - x;
				char* id = malloc (delta * sizeof(char));

				for (int i = x + 1; i < y; i++) {
					id[counter] = command[i];
					counter++;
				}

				command += delta + 1;
				canIds[canIdIndex] = atoi(id);
				canIdIndex++;

				if (command[0] == '\0' || command[0] == 'x') {
					break;
				}
			}

			if (canReceive (device, &frame) == 0)
				printFrame (&frame, canIds, (sizeof(canIds) / sizeof(canIds[0])));
			else
				printf ("Error: %s.\n", errorMessage (errno));
			break;
		}
		case 'f': {
			if (canFlushRx (device) != 0)
				printf ("Error: %s.\n", errorMessage (errno));
			break;
		}
		case 'm': {
			timeoutMs = promptTimeout ();
			if (canSetTimeout (device, timeoutMs) != 0)
				printf ("Error: %s.\n", errorMessage (errno));
			break;
		}
		case 'q': {
			return 0;
		}}
	};
}