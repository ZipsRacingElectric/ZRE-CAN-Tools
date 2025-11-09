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
#include "can_device/slcan.h"

// C Standard Library
#include <errno.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>

// Functions ------------------------------------------------------------------------------------------------------------------

// DiBacco: function no longer used
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

// TODO(DiBacco): create a manual page for the can-dev-cli command
void displayHelp() 
{
	// TODO(DiBacco): create a manual page for the can-dev-cli command
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

void receiveTimeout (time_t seconds, long microseconds)
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
	printf ("Done\n");
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
	// TODO(DiBacco): figure out why transmit function is not working correctly from the command line
	canFrame_t frame;
	char* originalCommand;

	// Validate Input
    // TODO (DiBacco): may need to add ' ' functionality between bytes in the payload
    // TODO (DiBacco): may need to validate that each id is within a certain range during parsing
    bool hasPayload = false;
    bool hasIterationInput = false; 
    int x = strcspn (command, "["); 
    int y = strcspn (command, "]");
    if (command[0] == '[') {
        printf ("Invalid Input: Missing ID Field\n");
        return 1;
    }
    if (! (command[x] == '[' && command[y] == ']')) {
        printf("Invalid Input: Missing Brackets\n");
        return 1;
    }
    for (int i = 0; i < x; i++) {
        if (! ((command[i] >= '0' && command[i] <= '9'))) {
            printf("Invalid Input: Invalid Character in Input Field\n");
            return 1;
        }
    }
    for (int i = x + 1; i < y; i++) {
        if (! ((command[i] >= '0' && command[i] <= '9') || command[i] == ',')) {
            if (i > 1 && command[i] == ' ' && (command[i-1] == ',' || command[i-1] == ' '))  continue; 
            printf("Invalid Input: Invalid Character in Brackets\n");
            return 1;
        } else {
            hasPayload = true;
        }
    }
    if (! hasPayload) {
        printf("Invalid Input: Missing Payload\n");
        return 1;
    }
    if (! (command[y + 1] == '\0')) {
        if (! (command[y + 1] == '@')) {
            printf("Invalid Input: Invalid Symbol After Brackets (@)\n");
            return 1;
        } 
        for (int i = y + 2; i < strlen(command); i++) {
            if (! ((command[i] >= '0' && command[i] <= '9'))) {
                printf("Invalid Input: Invalid Iteration Input\n");
                return 1;
            } else {
                hasIterationInput = true;
            }
        }
        if (! hasIterationInput) {
            printf("Invalid Input: Missing Iteration Input\n");
            return 1;
        }
    }

	// Set Iterations
	int transmitIterations = 1;
	size_t delimiterPosition = strcspn (command, "@");
	if (command[delimiterPosition] == '@') {
		transmitIterations = strtol(command + delimiterPosition + 1, NULL, 10);
		command[transmitIterations] = '\0';
		strcpy (originalCommand, command);
	} else {
		strcpy (originalCommand, command);
	}
	command[strlen(command) -1] = '\0';

	// Get Number of DLC Bytes
	int dlcBytes = 1;
	for (size_t i = 0; i < strlen(command); i++) {
		if (strchr (",", command[i]) != NULL) { 
			dlcBytes++;
		}
	}
	
	// Assign Frame ID
	int id = atoi (strtok (command, "["));
	frame.id = (uint32_t) id;

	// Assign Frame DLC & Data
	frame.dlc = (uint8_t) dlcBytes;
	for (int i = 0; i < dlcBytes; i++) {
		char* byte = strtok (NULL, ",");
		frame.data[i] = (uint8_t) strtol (byte, NULL, 0);
	}

	// Transmit Frame
	printf ("\n");
	for (int i = 0; i < transmitIterations; i++) {
		if (canTransmit (device, &frame) == 0) {
			printf ("%2d). %s => Success\n", i + 1, originalCommand);
		} else {
			printf ("Error: %s.\n", errorMessage (errno));
			return errno;
		}
	}
	return 0;
}

/*
	- Syntax ex). r=[512,513,514,515,1536,1]@12	
*/
int receiveFrame (canDevice_t* device, char* command) {
	canFrame_t frame;
	
	int canIdIndex = 0;
	size_t canIdLength = 1;

	// Validate Input
    // TODO (DiBacco): may need to add ' ' functionality between ids
    // TODO (DiBacco): may need to validate that each id is within a certain range during parsing
	int x = strcspn (command, "]");
    bool hasIds = false;
    bool hasIterationInput = false;
    if (! (command[0] == '[' && command[x] == ']')) {
        printf("Invalid Input: Missing Brackets\n");
        return 1;
    } 
    for (int i = 1; i < x; i++) {
        if (! ((command[i] >= '0' && command[i] <= '9') || command[i] == ',')) {
            if (i > 1 && command[i] == ' ' && (command[i-1] == ',' || command[i-1] == ' '))  continue; 
            printf("Invalid Input: Invalid Character in Brackets\n");
            return 1;
        } else {
            hasIds = true;
        }
    }
    if (! hasIds) {
        printf("Invalid Input: Missing IDs in Brackets\n");
        return 1;
    }
    if (! (command[x + 1] == '\0')) {
        if (! (command[x + 1] == '@')) {
            printf("Invalid Input: Invalid Symbol After Brackets (@)\n");
            return 1;
        } 
        for (int i = x + 2; i < strlen(command); i++) {
            if (! ((command[i] >= '0' && command[i] <= '9'))) {
                printf("Invalid Input: Invalid Iteration Input\n");
                return 1;
            } else {
                hasIterationInput = true;
            }
        }
        if (! hasIterationInput) {
            printf("Invalid Input: Missing Iteration Input\n");
            return 1;
        }
    }

	// Set Iterations
	int receiveIterations = 1;
	size_t deliminatorPosition = strcspn (command, "@");
	if (command[deliminatorPosition] == '@') {
		receiveIterations = strtol(command + deliminatorPosition + 1, NULL, 10);
		command[deliminatorPosition -1] = '\0';
	} 
	
	// Get Number of CAN IDs 
	for (size_t i = 0; i < strlen(command); i++) {
		if (strchr (",", command[i]) != NULL) { 
			canIdLength++;
		}
	}

	// Parse CAN IDs
	if (command[0] == '[') command++;
	if (command[strlen(command) - 1] == ']') command[strlen(command) - 1] = '\0'; 
	uint32_t* canIds = malloc (canIdLength * sizeof(uint32_t));
	for (size_t i = 0; i < canIdLength; i++) {
		char* id = (i == 0) ? strtok (command, ",") : strtok (NULL, ",");
		canIds[canIdIndex] = (uint32_t) strtoul (id, NULL, 0);
		canIdIndex++;
	}

	// Receive Frame
	// TODO(DiBacco): display a message for iterations w. no frames received
	for (int m = 0; m < receiveIterations; m++) {
		printf ("\n");
		printf ("Iteration: %d\n", m +1);
		for (size_t i = 0; i < canIdLength; i++) {
			int* visited = malloc (15 * sizeof(char*));
			size_t currentIndex = 0;
			bool received = false;
			while (! received) { 
				if (canReceive(device, &frame) == 0) {
					if (canIds[i] == frame.id) {
						printFrame (&frame);
						received = true;
					} else {
						for (size_t j = 0; j < currentIndex; j++) {
							if (visited[j] == frame.id) {
								received = true;
								break;
							}
						}
						visited[currentIndex] = frame.id;
						currentIndex++;
					}
				} else {
					printf ("Error: %s.\n", errorMessage (errno));
				}
			}
			free (visited);
		}
	}
	printf ("\n");
}

/*
	- Takes Entire Command and Processes it	
	- DiBacco: better function name
*/
int processCommand (canDevice_t* device, char* command) {
	// Validate Input
	size_t equalSignIndex = strcspn (command, "=");
	if ( ! (equalSignIndex == strlen (command) || equalSignIndex == 1)) 
	{
		printf ("Error: Invalid Command Format \n");
		return -1;
	}
		
	// TODO(DiBacco): add filters to detect invalid input
	// Get Method & Shift Command
	char method = command[0];
	command += 2;
			
	switch (method) {
	case 't': 
		transmitFrame (device, command);
		break;
	case 'r': 
		receiveFrame(device, command);
		break;
	case 'f': {
		if (canFlushRx (device) != 0)
			printf ("Error: %s.\n", errorMessage (errno));
		break;
	}
	case 'm': { 
		long unsigned int timeoutMs = promptTimeout (); // DiBacco
		if (canSetTimeout (device, timeoutMs) != 0)
			printf ("Error: %s.\n", errorMessage (errno));
		break;
	}
	case 'q': {
		return 0;
	}}
	return 0;
}

// Entrypoint -----------------------------------------------------------------------------------------------------------------

int main (int argc, char** argv)
{	
	// TODO(DiBacco): temporary removal before a more permanent implimentation
	/*
	if (argc < 2 || argc > 3)
	{
		fprintf (stderr, "Format: can-dev-cli -method (optional) <device name>\n");
		return -1;
	}
	*/

	// Enumerates communication devices if one is not provided
	char* deviceName;
	if (argc == 2) 
	{
		deviceName = argv [argc - 1];
	}
	else 
	{
		size_t deviceCount = 0;
		char** deviceNames = slcanEnumerateDevices (&deviceCount);
		// TODO(DiBacco): remove device name listing
		for (int i = 0; i < deviceCount; ++i)
		{
			printf ("Device Name: %s\n", deviceNames[i]);
		}
		deviceName = slcanGetDevice (deviceNames, deviceCount, "1000000");
		printf ("Full Device Name: %s\n", deviceName);
	}

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
			char* command = malloc (512);  
		
			printf ("Enter an option:\n");
			printf (" t - Transmit a CAN message.\n");
			printf (" r - Receive a CAN message.\n");
			printf (" f - Flush the receive buffer.\n");
			printf (" m - Set the device's timeout.\n");
			printf (" q - Quit the program.\n");

			fscanf (stdin, "%s%*1[\n]", command);
			processCommand (device, command);

			free (command);
		};
	}
	return 0;
}

