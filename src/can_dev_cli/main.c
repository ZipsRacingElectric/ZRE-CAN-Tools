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
#include "time_port.h"

// C Standard Library
#include <errno.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>

const int CAN_ID_LENGTH = 32;
const int TRANSMIT_DELAY = 500; // in microseconds

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

/*
	Manual Page for the Transmit & Receive Methods
*/
void displayHelp() 
{
	printf ("\n\
		Transmit / Receive Can-Frames. \n\\n\
		Methods: \n\
		Transmit Frames: t=<id>[<byte1>, <byte2>, ...]@<iterations>\n\
			- id: specifies the id of the frame\n\
			- byte: specifies the content of each byte in the payload\n\
			- iterations: specifies the number of iterations to transmit the frame\n\\n\
		Receive Frames: r=[<id1>, <id2>, ...]@<iterations>\n\
			- id: specifies the id of the Can-Frame to retrieve\n\
			- iterations: specifies the number of iterations to receive frames\n\
		");
}

/*
	- Nested function for the Receive Method
	- Print the Frame if the ID matches one of the provided IDs
*/
void printFrame (canFrame_t* frame)
{
	printf ("0x%3X: [", frame->id);
	for (uint8_t index = 0; index < frame->dlc; ++index) 
	{
		printf ("%02X,", frame->data [index]);
	}

	printf ("\b]\n");
	return;
}

/*
	- Creates a Timeout When Transmitting Frames on a Loop
*/
void transmitTimeout (time_t seconds, long microseconds)
{
	struct timeval timeout = 
	{
		.tv_sec = seconds,
		.tv_usec = microseconds
	};
	
	struct timeval deadline;
	struct timeval currentTime;
	gettimeofday (&currentTime, NULL);
	timeradd (&currentTime, &timeout, &deadline);
	
	while (timercmp (&currentTime, &deadline, <))
	{
		gettimeofday (&currentTime, NULL);
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

// Method Functions ------------------------------------------------------------------------------------------------------------

/*
	- Syntax ex). t=1[1,2,3,4,5,6,7,8]@12
*/
int transmitFrame (canDevice_t* device, char* command) {
	canFrame_t frame;
	int byteCount = 0;
	char originalCommand [500];
	strncpy(originalCommand, command, sizeof(originalCommand) - 1);

	// Set Iterations
	strtok(command, "@");
	int transmitIterations = (uint32_t) strtoul (strtok (NULL, "@"), NULL, 0);
	if (transmitIterations == 0) transmitIterations = 1;
	
	// Assign Frame ID
	if (command[0] == '[') {
		printf ("Please, enter an ID\n\n");
		return -1;
	}
	frame.id = (uint32_t) strtoul (strtok (command, "["), NULL, 0);

	// Assign Frame Data
	while (true) {
		char* byte = strtok (NULL, ",");
		if (byte == NULL) {
			break;
		}
		frame.data[byteCount] = (uint8_t) strtol (byte, NULL, 0);
		byteCount++;
	}

	// Assign Frame DLC
	frame.dlc = (uint8_t) byteCount -1;

	// Transmit Frame
	printf ("\n");
	for (int i = 0; i < transmitIterations; i++) {
		if (canTransmit (device, &frame) == 0) {
			transmitTimeout (0, TRANSMIT_DELAY); // 100 ms timeout between transmissions
			printf ("%2d). %s => Success\n", i + 1, originalCommand);
		} else {
			printf ("Error: %s.\n", errorMessage (errno));
			return errno;
		}
	}
	printf ("\n");
	return 0;
}

/*
	- Syntax ex). r=[512,513,514,515,1536,1]@12	
*/
int receiveFrame (canDevice_t* device, char* command) {
	canFrame_t frame;
	uint32_t canIds [CAN_ID_LENGTH];
	
	// Get Iterations from Input
	strtok(command, "@");
	int receiveIterations = (uint32_t) strtoul (strtok (NULL, "@"), NULL, 0);
	if (receiveIterations == 0) receiveIterations = 1;
	
	// Parse CAN IDs
	int canIdIndex = 0;
	if (command[0] == '[') command++;

	while (true) {
		char* id = (canIdIndex == 0) ? strtok (command, ",") : strtok (NULL, ",");
		if (id == NULL) {
			break;
		}
		canIds[canIdIndex] = (uint32_t) strtoul (id, NULL, 0);
		canIdIndex++;
	}

	// Receive Frame
	// TODO (DiBacco): Error: Receiver Empty - the CAN seems to run dry if it loops infinitly
	while (receiveIterations > 0) { 
		if (canReceive (device, &frame) == 0) {
			for (size_t i = 0; i < canIdIndex; i++) {
				if (canIds[i] == frame.id) {
					printFrame (&frame);
					receiveIterations--;
					break;
				}
			}
		} else {
			printf ("Error: %s.\n", errorMessage (errno));
		}
	}
	printf ("\n");
	return 0;
}

/*
	- Takes Entire Command and Processes it	
*/
int processCommand (canDevice_t* device, char* command) {	
	// Get Method & Shift Command
	long unsigned int timeoutMs;
	char method = command[0];
	command += 2;
			
	switch (method) {
	case 't': 
		transmitFrame (device, command);
		break;

	case 'r': 
		receiveFrame(device, command);
		break;

	case 'h':
		displayHelp();
		break;

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
		return -1;
	}}
	return 0;
}

// Entrypoint -----------------------------------------------------------------------------------------------------------------

int main (int argc, char** argv)
{	
	if (argc < 2 || argc > 3)
	{
		fprintf (stderr, "Format: can-dev-cli -method (optional) <device name>\n");
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

	// Directly From Command Line
	if (argc == 3) {
		char* command = argv [1];
		processCommand (device, command);
		return 0;

	} else {
		// Interactive Mode
		// If this is query mode, return successful
		if (queryMode)
			return 0;

		while (true)
		{
			char command [512];  
		
			printf ("Enter an option:\n");
			printf (" t - Transmit a CAN message.\n");
			printf (" r - Receive a CAN message.\n");
			printf (" h - Display man page.\n");
			printf (" f - Flush the receive buffer.\n");
			printf (" m - Set the device's timeout.\n");
			printf (" q - Quit the program.\n");

			// fscanf (stdin, "%s*1[\n]", command);
			fgets(command, sizeof(command), stdin);

			if (strchr (command, '=') != NULL) {
				if (processCommand (device, command) == -1) return 0;
				continue;

			} else {
				canFrame_t frame;
				char method = command[0];
				long unsigned int timeoutMs;
				switch (method) {
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

					case 'h':
						displayHelp();
						break;

					case 'f':
						if (canFlushRx (device) != 0)
							printf ("Error: %s.\n", errorMessage (errno));
						break;
						
					case 'm':
						timeoutMs = promptTimeout ();
						if (canSetTimeout (device, timeoutMs) != 0)
							printf ("Error: %s.\n", errorMessage (errno));
						break;

					case 'q':
						return 0;
				}
			}
		
		};
	}
	return 0;
}