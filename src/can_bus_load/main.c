// CAN Bus Load Calculator ----------------------------------------------------------------------------------------------------
//
// Author: Cole Barach
// Date Created: 2025.11.04
//
// Description: TODO(Barach)

// TODO(Barach): Engineering min and max bit stuffing?

// Includes -------------------------------------------------------------------------------------------------------------------

// Includes
#include "can_device/can_device.h"
#include "time_port.h"

// Entrypoint -----------------------------------------------------------------------------------------------------------------

int main (int argc, char** argv)
{
	if (argc != 2)
	{
		fprintf (stderr, "Invalid arguments, usage: can-bus-load <Device Name>\n");
		return -1;
	}

	canDevice_t* device = canInit (argv [argc - 1]);
	if (device == NULL)
		return errorPrintf ("Failed to create CAN device");

	canBaudrate_t baudrate = canGetBaudrate (device);
	float bitTime = 1.0f / baudrate;

	while (true)
	{
		// Track the response timeout
		struct timeval startTime;
		struct timeval endTime;
		struct timeval currentTime;
		gettimeofday (&startTime, NULL);
		const struct timeval MIN_PERIOD = { .tv_sec = 1 };
		timeradd (&startTime, &MIN_PERIOD, &endTime);

		size_t minBitCount = 0;
		size_t maxBitCount = 0;

		//for (size_t index = 0; index < 610; ++index)
		do
		{
			canFrame_t frame;
			if (canReceive (device, &frame) == 0)
			{
				size_t bitCount;
				if (frame.ide)
				{
					// TODO(Barach): validate and doc
					bitCount = 67 + frame.dlc * 8;
				}
				else
				{
					// TODO(Barach): validate and doc
					bitCount = 47 + frame.dlc * 8;
				}

				// TODO(Barach): Accurate?
				minBitCount += bitCount;
				maxBitCount += bitCount + (bitCount - 12) / 5;
			}

			gettimeofday (&currentTime, NULL);
		} while (timercmp (&currentTime, &endTime, <));

		struct timeval actualPeriodTv;
		timersub (&currentTime, &startTime, &actualPeriodTv);

		float actualPeriod = actualPeriodTv.tv_sec + actualPeriodTv.tv_usec * 1e-6f;

		float maxLoad = bitTime * maxBitCount / actualPeriod;
		float minLoad = bitTime * minBitCount / actualPeriod;
		printf ("Max bit count = %zu, min bit count = %zu\n", maxBitCount, minBitCount);
		printf ("Max Load = %f %%\n", maxLoad * 100.0f);
		printf ("Min Load = %f %%\n", minLoad * 100.0f);
	}
}