// CAN Device CLI -------------------------------------------------------------------------------------------------------------
//
// Author: Cole Barach, ... REVIEW(Barach): Add your name up here. This tells later software devs who to direct questions to.
// Date Created: 2025.07.04
//
// Description: Command-line interface for controlling a CAN adapter.

// REVIEW(Barach):
// -d doesn't work from standard arguments.

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

// REVIEW(Barach): Documentation on what this is, could be helped with better naming too,
// ex. MAX_CAN_ID_COUNT or something to that extent.
const int CAN_ID_LENGTH = 32;

// Functions ------------------------------------------------------------------------------------------------------------------

/*
	Function used by the interactive frame trasmit implementation to prompt frame information from the user and assign it to the frame accordingly
*/
void promptFrame (canFrame_t* frame)
{
	char buffer [512];

	// id
	printf ("Enter the ID of the message: ");
	fgets (buffer, sizeof (buffer), stdin);
	buffer [strcspn (buffer, "\r\n")] = '\0';
	frame->id = strtol (buffer, NULL, 0);

	// DLC
	printf ("Enter the DLC of the message: ");
	fgets (buffer, sizeof (buffer), stdin);
	buffer [strcspn (buffer, "\r\n")] = '\0';
	frame->dlc = strtol (buffer, NULL, 0);

	// Payload
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
			Methods:\n\
			Transmiting Frames:\n\
    			t=<ID>[<Byte 1>,<Byte 2>,...<Byte n>]\n\
        			Transmits a single CAN frame.\n\
				\n\
    			t=<ID>[<Byte 1>,<Byte 2>,...<Byte n>]@<Freq>,<Count>\n\
        			Transmits a CAN frame at a specified frequency.\n\
				\n\
    			Arguments:\n\
        			ID     - The CAN ID of the frame to transmit.\n\
        			Byte n - The n'th byte of the payload, in little-endian.\n\
        			Freq   - The frequency to transmit at, in Hertz.\n\
        			Count  - The number of times to transmit the message.\n\
			\n\
			Receiving Frames:\n\
   				r - Receives the first available CAN message.\n\
				\n\
    			r=[<ID 0>,<ID 1>,...<ID n>]\n\
        			Receives the first available CAN message from a list of IDs.\n\
				\n\
    			r=[<ID 0>,<ID 1>,...<ID n>]@<Count>\n\
        			Receives the first set of availble CAN messages from a list of IDs.\n\
				\n\
    			r=[]@<Iterations>\n\
        			Receives the first set of available CAN messages.\n\
				\n\
    			d - Dumps all received CAN messages.\n\
				\n\
    			d=[<ID 0>,<ID 1>,...<ID n>]\n\
        			Dumps all received CAN messages from a list of IDs.\n\
				\n\
    			Arguments:\n\
        			ID n  - The n'th CAN ID to filter for.\n\
        			Count - Specifies the number CAN frames to receive\n\
		");
	printf ("\n");
}

// REVIEW(Barach): Outdated documentation (doesn't check IDs anymore).
/*
	- Nested function for the Receive Method
	- Print the Frame if the ID matches one of the provided IDs
	- Output is displayed in 0xid[<byte1>, <byte2>, ...] format
*/
void printFrame (canFrame_t* frame)
{
	// REVIEW(Barach): This should use the same syntax as the transmit function accepts, i.e. no spaces or colon.
	// REVIEW(Barach): The '%3X' specifier prints right justified, so there is an awkward gap between the '0x' and the number
	//   printed. Better to use the '%03X' specifier which inserts leading 0's.
	printf ("0x%3X: [", frame->id);
	for (uint8_t index = 0; index < frame->dlc; ++index)
	{
		printf ("%02X,", frame->data [index]);
	}

	// REVIEW(Barach): Behavior of \b is not standardized across all terminal environments. Better to not print the comma on
	//   the last iteration.
	printf ("\b]\n");
	return;
}

/*
	Assigns duration for the CAN BUS to wait when it is not receiving frames
*/
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
	- Syntax ex). t=1[1,2,3,4,5,6,7,8]@0.5, 12
*/
int transmitFrame (canDevice_t* device, char* command) {
	canFrame_t frame;
	int byteCount = 0;

	// Set Iterations
	strtok(command, "@"); // seperates backet segment of the command from the iteration / frequency part
	
	float frequency;
	int transmitIterations;
	char* x = strtok (NULL, ",");
	if (x == NULL) {
		frequency = 1.0;
		transmitIterations = 1;
	} else {
		frequency = strtof (x, NULL); // in hertz
		x = strtok (NULL, ",");
		if (x == NULL) 
			transmitIterations = 1;
		else
	 		transmitIterations = (uint32_t) strtoul (x, NULL, 0);
	}
	long totalMicroseconds = (1e6 / frequency); // microseconds = 1,000,000 / hertz
	
	long microseconds = totalMicroseconds % 1000000;
	time_t seconds = totalMicroseconds / 1000000;

	struct timeval timeout = 
	{
		.tv_sec = seconds,
		.tv_usec = microseconds
	};
	
	struct timeval deadline;
	struct timeval currentTime;

	// Assign Frame ID
	if (command[0] == '[') { // if the first index in the command is '[' (not id)
		printf ("Please, enter an ID\n\n");
		return -1;
	}
	// REVIEW(Barach): Unchecked strtok return value.
	frame.id = (uint32_t) strtoul (strtok (command, "["), NULL, 0);

	// REVIEW(Barach): This doesn't handle empty frames, '[]', correctly. Interprets as '[0]'. Not an off-by-one error though.
	// Assign Frame Data
	while (true) { // parse out ids from command until there are no more
		char* byte = strtok (NULL, ",");
		if (byte == NULL) {
			break;
		}
		frame.data[byteCount] = (uint8_t) strtol (byte, NULL, 0);
		byteCount++;
	}

	// Assign Frame DLC
	frame.dlc = (uint8_t) byteCount;

	// REVIEW(Barach): Very dense code is hard to read. Put some empty lines between distinct sections and comments to break
	//   it up. Ex. gettimeofday and timeradd are performing one distinct 'action' so they can be grouped together.
	//   Also inline comments (after the line of code) are not preferred for this reason, better to put the comment above the
	//   line and use spacing to seperate it from the adjacent code.
	// Transmit Frame
	printf ("\n");
	for (int i = 0; i < transmitIterations; i++) {
		gettimeofday (&currentTime, NULL);
		timeradd (&currentTime, &timeout, &deadline);
		if (canTransmit (device, &frame) == 0) {
			printFrame(&frame);

			// REVIEW(Barach): No need to delay on the last iteration. This makes single message transmissions very slow.
			while (timercmp (&currentTime, &deadline, <)) // waits until deadline is reached
				gettimeofday (&currentTime, NULL);
			
		} else {
			// REVIEW(Barach): Should attempt to keep transmitting if an error occurs. No need to change anything with
			//   iterations though, if we're only doing 5 messages and all 5 fail, then no need to retry any of them. Also
			//   preferrably the delay is applied to failed messages as well (move while loop outside the if branch).
			printf ("Error: %s.\n", errorMessage (errno));
			return errno;
		}
	}
	printf ("\n");
	return 0;
}

/*
	- Syntax ex). r=[512,513,514,515,1536,1]@12	/ d=[512,513,514,515,1536,1]
*/
int receiveFrame (canDevice_t* device, char* command, bool infiniteIterations) {
	canFrame_t frame;
	bool filterIds = false;
	uint32_t canIds [CAN_ID_LENGTH];
	
	// Get Iterations from Input
	int receiveIterations;
	if (! infiniteIterations) { // if the user is using the receive (r) method
		strtok(command, "@");
		receiveIterations = (uint32_t) strtoul (strtok (NULL, "@"), NULL, 0);
		if (receiveIterations == 0) receiveIterations = 1;
	}
	
	// Parse CAN IDs
	int canIdIndex = 0;
	if (command[0] == '[') command++;

	while (true) {
		char* id = (canIdIndex == 0) ? strtok (command, ",") : strtok (NULL, ",");
		// checks if the parse is invalid
		// REVIEW(Barach): This second clause can be avoided by adding ']' to the delimiters.
		if (id == NULL || strcmp (id, "]") == 0) {
			break;
		}
		canIds[canIdIndex] = (uint32_t) strtoul (id, NULL, 0);
		filterIds = true; // indicates that the user has input at least one id
		canIdIndex++;
		
	}

	// Receive Frame
	while (infiniteIterations || receiveIterations > 0) { 
		if (canReceive (device, &frame) == 0) { // receives a CAN Frame from the bus
			if (! filterIds) { // checks that the user has input ids
				printFrame (&frame);
				receiveIterations--;
				continue;
			}
			else {
				for (size_t i = 0; i < canIdIndex; i++) {
					if (canIds[i] == frame.id) {
						printFrame (&frame);
						receiveIterations--;
						break;
					}
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
	char method = command[0]; // gets method type (t / r / d)
	command += 2;
	
	switch (method) {
	case 't': 
		transmitFrame (device, command);
		break;

	case 'r': 
		receiveFrame(device, command, false);
		break;

	case 'd':
		receiveFrame(device, command, true);
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
	if (argc < 2)
	{
		// REVIEW(Barach): A bit pedantic, but all other applications use the below syntax:
		//   can-dev-cli <options> <device name>
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
		if (command[0] == '-') 
			processCommand (device, ++command);

		else 
			// REVIEW(Barach): Bit pedantic again, but technically this should say options (device name is a command-line
			//  argument, but it doesn't start with '-')
			printf ("Command-Line arguments should start with '-'\n");
		
		return 0;

	} else {
		// REVIEW(Barach): Preferrably query mode is higher precedence than the command-line options (immediately after
		//   the call to canInit). Shouldn't really be an issue, but just in case.
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
			// REVIEW(Barach): Preferrably this refers to it as 'Dump received CAN messages' or something to that effect, just
			// to help with the mnemonic.
			printf (" d - Infinitely receive CAN messages.\n");
			printf (" h - Display man page.\n");
			printf (" f - Flush the receive buffer.\n");
			printf (" m - Set the device's timeout.\n");
			printf (" q - Quit the program.\n");

			fgets(command, sizeof(command), stdin);

			if (strchr (command, '=') != NULL) {
				if (processCommand (device, command) == -1) return 0;
				continue;

			} else {
				// REVIEW(Barach): A lot of redundant code between this and the processCommand function. This is where the bug
				//   with '-d' is coming from. The functionality of this and the processCommand should be merged together for
				//   consistency (processCommand will need to use return code for quit).
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
					
					case 'd':
						while (true) {
							if (canReceive (device, &frame) == 0)
								printFrame (&frame);

							else
								printf ("Error: %s.\n", errorMessage (errno));
						}
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