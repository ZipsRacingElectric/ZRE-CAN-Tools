// CAN Bus Load Calculator ----------------------------------------------------------------------------------------------------
//
// Author: Cole Barach
// Date Created: 2025.11.04
//
// Description: See help page. See can_device/can_bus_load.h for more details on what CAN bus load is and how it is calculated.

// Includes -------------------------------------------------------------------------------------------------------------------

// Includes
#include "can_device/can_bus_load.h"
#include "can_device/can_device.h"
#include "can_device/can_device_stdio.h"
#include "debug.h"
#include "options.h"
#include "time_port.h"

// C Standard Library
#include <stdio.h>

// Functions ------------------------------------------------------------------------------------------------------------------

void fprintUsage (FILE* stream)
{
	fprintf (stream, "Usage: can-bus-load <Options> <Device Name>\n");
}

void fprintHelp (FILE* stream)
{
	fprintf (stream, ""
		"can-bus-load - Application for estimating the load of a CAN bus. CAN bus load is\n"
		"               defined as the percentage of time the CAN bus is in use. This\n"
		"               calculator estimates both the minimum and maximum bounds of this\n"
		"               load.\n\n");

	fprintUsage (stream);

	fprintf (stream, "\nOutput format: [<Min>, <Max>]\n\n");

	fprintf (stream, "Parameters:\n\n");
	fprintCanDeviceNameHelp (stream, "    ");

	fprintf (stream, "Options:\n\n");
	fprintOptionHelp (stream, "    ");
}

// Entrypoint -----------------------------------------------------------------------------------------------------------------

int main (int argc, char** argv)
{
	// Debug initialization
	debugInit ();

	// Check standard arguments
	for (int index = 1; index < argc; ++index)
	{
		switch (handleOption (argv [index], NULL, fprintHelp))
		{
		case OPTION_CHAR:
		case OPTION_STRING:
			fprintf (stderr, "Unknown argument '%s'.\n", argv [index]);
			return -1;

		case OPTION_QUIT:
			return 0;

		default:
			break;
		}
	}

	// Validate usage
	if (argc < 2)
	{
		fprintUsage (stderr);
		return -1;
	}

	// Initialize the CAN device
	char* deviceName = argv [argc - 1];
	canDevice_t* device = canInit (deviceName);
	if (device == NULL)
		return errorPrintf ("Failed to initialize CAN device '%s'", deviceName);

	// Calculate the bit time from the bus baudrate.
	canBaudrate_t baudrate = canGetBaudrate (device);
	if (baudrate == CAN_BAUDRATE_UNKNOWN)
	{
		fprintf (stderr, "CAN device baudrate is required.\n");
		return -1;
	}
	float bitTime = canCalculateBitTime (baudrate);

	// Set a receive timeout so we can measure busses with no load.
	canSetTimeout (device, 100);

	while (true)
	{
		// Load measurements
		size_t frameCount = 0;
		size_t minBitCount = 0;
		size_t maxBitCount = 0;

		// Track the measurement period
		struct timespec startTime;
		struct timespec endTime;
		struct timespec currentTime;
		clock_gettime (CLOCK_MONOTONIC, &startTime);
		endTime = timespecAdd (&startTime, &(struct timespec) { .tv_sec = 1 });

		// Measurement loop
		do
		{
			// Receive a frame and record its measurements
			canFrame_t frame;
			if (canReceive (device, &frame) == 0)
			{
				++frameCount;
				minBitCount += canGetMinBitCount (&frame);
				maxBitCount += canGetMaxBitCount (&frame);
			}

			// Check measurement timeout
			clock_gettime (CLOCK_MONOTONIC, &currentTime);
		} while (timespecCompare (&currentTime, &endTime, <));

		// Calculate the actual measurement period
		struct timespec period = timespecSub (&currentTime, &startTime);

		// Calculate the min and max loads
		float maxLoad = canCalculateBusLoad (maxBitCount, bitTime, period);
		float minLoad = canCalculateBusLoad (minBitCount, bitTime, period);

		// Print stats
		printf ("Bus Load: [%6.2f%%, %6.2f%%],   Frames Received: %5lu,   Bits Received: [%7lu, %7lu]\n",
			minLoad * 100.0f, maxLoad * 100.0f, (unsigned long) frameCount, (unsigned long) minBitCount, (unsigned long) maxBitCount);
	}
}