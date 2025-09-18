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

void printFrame (canFrame_t* frame, uint32_t* canIds, size_t canIdLength, int numberOfIterations)
{
	// r=[512][513][514][515][1536][1537]
	// r=[0x200][0x201][0x202][0x203][0x600][0x601]
	for (size_t i = 0; i < canIdLength; i++) {
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
		char* command = malloc (512);  
		long unsigned int timeoutMs;
		
		printf ("Enter an option:\n");
		printf (" t - Transmit a CAN message.\n");
		printf (" r - Receive a CAN message.\n");
		printf (" f - Flush the receive buffer.\n");
		printf (" m - Set the device's timeout.\n");
		printf (" q - Quit the program.\n");

		fscanf (stdin, "%s%*1[\n]", command);

		size_t equalSignIndex = strcspn (command, "=");
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
			size_t canIdLength = 0;
			const char* bracketCharacters = "([{<>}])";

			// Retreive the number of iterations from the user input
			int numberOfIterations = 1;
			size_t x = strcspn (command, "x");
			if (command[x] == 'x') numberOfIterations = strtol(command + x + 1, NULL, 10);

			// Get the number of CAN Ids to allocate memory for the array
			for (size_t i = 0; i < strlen(command); i++) {
				if (strchr (bracketCharacters, command[i]) != NULL) { 
					canIdLength++;
				}
			}

			canIdLength /= 2; 
			uint32_t* canIds = malloc (canIdLength * sizeof(uint32_t));
			char* id = strtok (command, bracketCharacters);
			canIds[canIdIndex] = (uint32_t) strtoul (id, NULL, 10);
			canIdIndex++;

			// Parse the CAN Ids from the user input
			for (size_t i = 0; i < canIdLength -1; i++) {
				int base = 10;
				id = strtok (NULL, bracketCharacters);
				if (id[0] == '0' && (id[1] == 'x' || id[1] == 'X')) base = 16;
				canIds[canIdIndex] = (uint32_t) strtoul (id, NULL, base);
				printf ("CAN ID: %u\n", canIds[canIdIndex]);
				canIdIndex++;
			}

			if (canReceive (device, &frame) == 0)
				printFrame (&frame, canIds, canIdLength, numberOfIterations);
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