// CAN Device CLI -------------------------------------------------------------------------------------------------------------
//
// Author: Cole Barach, Owen DiBacco
//
// Description: Command-line interface for controlling a CAN adapter.

// Includes -------------------------------------------------------------------------------------------------------------------

// Includes
#include "can_device/can_device.h"
#include "can_device/can_device_stdio.h"
#include "error_codes.h"
#include "time_port.h"

// C Standard Library
#include <errno.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>

// Constants ------------------------------------------------------------------------------------------------------------------

/// @brief The maximum number of CAN IDs that can be filtered during receiving.
const size_t MAX_CAN_ID_COUNT = 2;

// Standard I/O ---------------------------------------------------------------------------------------------------------------

/**
 * @brief Prompts the user for the fields of a CAN frame.
 * @param frame Buffer to write the frame into.
 */
void promptFrame (canFrame_t* frame)
{
	char buffer [512];

	// CAN ID
	while (true)
	{
		printf ("Enter the CAN ID of the frame: ");
		fgets (buffer, sizeof (buffer), stdin);
		buffer [strcspn (buffer, "\r\n")] = '\0';
		if (strToCanId (&frame->id, &frame->ide, buffer) == 0)
			break;

		fprintf (stderr, "Invalid CAN ID.\n");
	}

	// DLC
	while (true)
	{
		printf ("Enter the DLC of the frame: ");
		fgets (buffer, sizeof (buffer), stdin);
		buffer [strcspn (buffer, "\r\n")] = '\0';
		frame->dlc = strtol (buffer, NULL, 0);
		if (frame->dlc <= 8)
			break;
	}

	// Payload
	for (uint8_t index = 0; index < frame->dlc; ++index)
	{
		printf ("Enter byte %i of the payload: ", index);
		fgets (buffer, sizeof (buffer), stdin);
		buffer [strcspn (buffer, "\r\n")] = '\0';
		frame->data [index] = strtol (buffer, NULL, 0);
	}
}

/**
 * @brief Prompts the user to enter a timeout.
 * @return The entered timeout, in milliseconds.
 */
unsigned long int promptTimeout ()
{
	char buffer [512];

	printf ("Timeout (ms): ");
	fgets (buffer, sizeof (buffer), stdin);
	buffer [strcspn (buffer, "\r\n")] = '\0';
	return strtol (buffer, NULL, 0);
}

// User Help ------------------------------------------------------------------------------------------------------------------

/// @brief Prints the program usage.
void fprintUsage (FILE* stream)
{
	fprintf (stream, "Usage: can-dev-cli <options> <device name>\n");
}

/// @brief Prints examples for how to use the program.
void fprintExamples (FILE* stream)
{
	fprintf (stream,
		"Examples:\n"
		"\n"
		"    Dump all received CAN messages:\n"
		"        can-dev-cli -d <Device Name>\n"
		"\n"
		"    Periodically transmit a CAN message (50 times at 10 Hz):\n"
		"        can-dev-cli -t=0x123[0xAB,0xCD]@50,10\n"
		"\n"
		"    Dump all received CAN messages from a list:\n"
		"        can-dev-cli -d=[0x005,0x006,0x007,0x008]\n"
		"\n");
}

/// @brief Prints detailed help about the program.
void fprintHelp (FILE* stream)
{
	fprintUsage (stream);

	fprintf (stream,
		"\nOptions:\n"
		"    Transmiting Frames:\n"
		"        -t=<ID>[<Byte 1>,<Byte 2>,...<Byte n>]\n"
		"            Transmits a single CAN frame.\n"
		"\n"
		"        -t=<ID>[<Byte 1>,<Byte 2>,...<Byte n>]@<Count>,<Freq>\n"
		"            Transmits a CAN frame at a specified frequency.\n"
		"\n"
		"        Parameters:\n"
		"            ID     - The CAN ID of the frame to transmit.\n"
		"            Byte n - The n'th byte of the payload, in little-endian.\n"
		"            Freq   - The frequency to transmit at, in Hertz.\n"
		"            Count  - The number of times to transmit the message.\n"
		"\n"
		"    Receiving Frames:\n"
		"        -r - Receives the first available CAN message.\n"
		"\n"
		"        -r=<ID>\n"
		"            Receives the first available CAN message matching said ID.\n"
		"\n"
		"        -r=<ID>@<Count>\n"
		"            Receives the set of available CAN message matching said ID.\n"
		"\n"
		"        -r=[<ID 0>,<ID 1>,...<ID n>]\n"
		"            Receives the first available CAN message from a list of IDs.\n"
		"\n"
		"        -r=[<ID 0>,<ID 1>,...<ID n>]@<Count>\n"
		"            Receives the first set of available CAN messages from a list of IDs.\n"
		"\n"
		"        -r=[]@<Count>\n"
		"            Receives the first set of available CAN messages.\n"
		"\n"
		"        -d - Dumps all received CAN messages.\n"
		"\n"
		"        -d=<ID>\n"
		"            Dumps all received CAN messages matching said ID.\n"
		"\n"
		"        -d=[<ID 0>,<ID 1>,...<ID n>]\n"
		"            Dumps all received CAN messages from a list of IDs.\n"
		"\n"
		"        Parameters:\n"
		"            ID n  - The n'th CAN ID to filter for.\n"
		"            Count - Specifies the number CAN frames to receive\n"
		"\n");

	fprintExamples (stream);
}

// Command Handling -----------------------------------------------------------------------------------------------------------

/**
 * @brief Handles a transmit command.
 * @param device The CAN device to use.
 * @param command The transmit command string. See @c fprintHelp for the format.
 */
void transmitFrame (canDevice_t* device, char* command)
{
	canFrame_t frame;

	struct timeval period =
	{
		.tv_sec		= 1,
		.tv_usec	= 0
	};
	size_t iterationCount = 1;

	// Parse out the iteration count and frequency
	strtok (command + 2, "@");
	char* iterationStr = strtok (NULL, ",");
	if (iterationStr != NULL)
	{
		iterationCount = (uint32_t) strtoul (iterationStr, NULL, 0);

		char* frequencyStr = strtok (NULL, ",");
		if (frequencyStr != NULL)
		{
			float frequency = strtof (frequencyStr, NULL);
			long periodUs = (1e6 / frequency);
			period = (struct timeval)
			{
				.tv_sec		= periodUs / 1000000,
				.tv_usec	= periodUs % 1000000
			};
		}
	}

	if (command [1] == '=')
	{
		if (strToCanFrame (&frame, command + 2) != 0)
		{
			fprintf (stderr, "Error: Invalid CAN frame format.\n");
			return;
		}
	}
	else
	{
		promptFrame (&frame);
	}

	struct timeval timeCurrent;
	struct timeval timeDeadline;

	for (size_t index = 0; index < iterationCount; ++index)
	{
		// Start timer
		gettimeofday (&timeCurrent, NULL);
		timeradd (&timeCurrent, &period, &timeDeadline);

		if (canTransmit (device, &frame) != 0)
			errorPrintf ("Failed to transmit CAN frame");
		else
		{
			fprintCanFrame (stdout, &frame);
			printf ("\n");
		}

		if (index == iterationCount - 1)
			break;

		// Wait until deadline is reached
		while (timercmp (&timeCurrent, &timeDeadline, <))
			gettimeofday (&timeCurrent, NULL);
	}
}

/**
 * @brief Handles a receive / dump command.
 * @param device The CAN device to use.
 * @param command The receive / dump command string. See @c fprintHelp for the format.
 * @param infiniteIterations True to use to infinite iterations, false for user-specified iteration count.
 */
void receiveFrame (canDevice_t* device, char* command, bool infiniteIterations)
{
	// If not using infinite iterations, parse out the iteration count.
	size_t iterationCount = 1;
	if (!infiniteIterations)
	{
		strtok (command, "@");
		char* iterationStr = strtok (NULL, "@");

		if (iterationStr != NULL)
			iterationCount = strtoul (iterationStr, NULL, 0);
	}

	// Parse the CAN IDs to filter by
	size_t idCount = 0;
	uint32_t ids [MAX_CAN_ID_COUNT];
	bool ides [MAX_CAN_ID_COUNT];
	if (command [1] == '=')
	{
		char* strtokArg = command + 2;
		while (true)
		{
			char* id = strtok (strtokArg, ",]");
			strtokArg = NULL;
			if (id == NULL)
				break;

			if (idCount == MAX_CAN_ID_COUNT)
			{
				fprintf (stderr, "Error: Maximum CAN ID count exceeded.\n");
				return;
			}

			if (strToCanId (&ids [idCount], &ides [idCount], id) != 0)
			{
				fprintf (stderr, "Warning: Ignoring invalid CAN ID '%s'...\n", id);
				continue;
			}

			++idCount;
		}
	}

	// Receive loop
	canFrame_t frame;
	while (infiniteIterations || iterationCount > 0)
	{
		// Receive a frame
		if (canReceive (device, &frame) != 0)
		{
			int code = errno;

			if (canCheckBusError (code))
			{
				// If the error was a bus error, print the error frame and bus error name.
				fprintCanFrame (stdout, &frame);
				printf (" - [%s]\n", canGetBusErrorName (code));
				errno = 0;
			}
			else
				errorPrintf ("Failed to receive CAN frame");
		}

		// Check whether the frame should be filtered or not
		bool frameValid = idCount == 0;
		for (size_t index = 0; index < idCount; ++index)
		{
			if (ids [index] == frame.id && ides [index] == frame.ide)
			{
				frameValid = true;
				break;
			}
		}
		if (!frameValid)
			continue;

		// Print the frame's contents
		fprintCanFrame (stdout, &frame);
		printf ("\n");
		--iterationCount;
	}
}

/**
 * @brief Handles a user-specified command from either the standard arguments or standard input.
 * @param device The CAN device to use.
 * @param command The command to handle.
 * @return 0 for handled command, -1 to quit the program.
 */
int processCommand (canDevice_t* device, char* command)
{
	long unsigned int timeoutMs;

	// Get and check the command type
	char commandType = command[0];
	switch (commandType)
	{
	case 't':
		transmitFrame (device, command);
		break;

	case 'r':
		receiveFrame (device, command, false);
		break;

	case 'd':
		receiveFrame (device, command, true);
		break;

	case 'f':
		if (canFlushRx (device) != 0)
			errorPrintf ("Failed to flush receive buffer");
		break;

	case 'm':
		timeoutMs = promptTimeout ();
		if (canSetTimeout (device, timeoutMs) != 0)
			errorPrintf ("Failed to set timeout");
		break;

	case 'h':
		fprintHelp (stdout);
		break;

	case 'q':
		return -1;

	}

	return 0;
}

// Entrypoint -----------------------------------------------------------------------------------------------------------------

int main (int argc, char** argv)
{
	// Check for query mode / help
	bool queryMode = false;
	for (int index = 1; index < argc; ++index)
	{
		if (strcmp (argv [index], "-q") == 0)
			queryMode = true;

		if (strcmp (argv [index], "-h") == 0 || strcmp (argv [index], "--help") == 0 || strcmp (argv [index], "help") == 0)
		{
			fprintHelp (stdout);
			return 0;
		}
	}

	// Validate the usage
	if (argc < 2)
	{
		fprintUsage (stderr);
		return -1;
	}

	// Create the CAN device
	char* deviceName = argv [argc - 1];
	canDevice_t* device = canInit (deviceName);
	if (device == NULL)
		return errorPrintf ("Failed to create CAN device");

	// If this is query mode, return successful
	if (queryMode)
		return 0;

	if (argc == 3)
	{
		// Run from standard arguments

		char* command = argv [1];
		if (command[0] == '-')
			processCommand (device, command + 1);
		else
			printf ("Options must start with '-'\n");

		return 0;
	}

	while (true)
	{
		char command [512];

		printf ("Enter an option:\n");
		printf (" t - Transmit a CAN message.\n");
		printf (" r - Receive a CAN message.\n");
		printf (" d - Dump received CAN messages.\n");
		printf (" h - Display the help page.\n");
		printf (" f - Flush the receive buffer.\n");
		printf (" m - Set the device's timeout.\n");
		printf (" q - Quit the program.\n");

		// Get and handle the command
		fgets (command, sizeof (command), stdin);
		command [strcspn (command, "\r\n")] = '\0';
		if (processCommand (device, command) == -1)
			break;
	}

	return 0;
}