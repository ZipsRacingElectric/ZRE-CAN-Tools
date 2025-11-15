// CAN Bus Load Calculator ----------------------------------------------------------------------------------------------------
//
// Author: Cole Barach
// Date Created: 2025.11.04
//
// Description: Application for estimating the load of a CAN bus. See can_device/can_bus_load.h for more details on what CAN
//   bus load is and how it is calculated.

// Includes -------------------------------------------------------------------------------------------------------------------

// Includes
#include "can_device/can_bus_load.h"
#include "can_device/can_device.h"
#include "time_port.h"

// Entrypoint -----------------------------------------------------------------------------------------------------------------

int main (int argc, char** argv)
{
	// Validate arguments
	if (argc != 2)
	{
		fprintf (stderr, "Invalid arguments, usage: can-bus-load <Device Name>\n");
		return -1;
	}

	// Create the CAN device.
	canDevice_t* device = canInit (argv [argc - 1]);
	if (device == NULL)
		return errorPrintf ("Failed to create CAN device");

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
		struct timeval startTime;
		struct timeval endTime;
		struct timeval currentTime;
		gettimeofday (&startTime, NULL);
		timeradd (&startTime, &(struct timeval) { .tv_sec = 1 }, &endTime);

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
			gettimeofday (&currentTime, NULL);
		} while (timercmp (&currentTime, &endTime, <));

		// Calculate the actual measurement period
		struct timeval period;
		timersub (&currentTime, &startTime, &period);

		// Calculate the min and max loads
		float maxLoad = canCalculateBusLoad (maxBitCount, bitTime, period);
		float minLoad = canCalculateBusLoad (minBitCount, bitTime, period);

		// Print stats
		printf ("Bus Load: [%6.2f%%, %6.2f%%],   Frames Received: %5lu,   Bits Received: [%7lu, %7lu]\n",
			minLoad * 100.0f, maxLoad * 100.0f, (unsigned long) frameCount, (unsigned long) minBitCount, (unsigned long) maxBitCount);
	}
}