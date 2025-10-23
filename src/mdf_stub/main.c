// Includes -------------------------------------------------------------------------------------------------------------------

// Includes
#include "mdf/mdf_can_bus_logging.h"
#include "debug.h"
#include "error_codes.h"
#include "can_device/can_device.h"

// C Standard Library
#include <stdbool.h>
#include <stdio.h>
#include <time.h>

// Entrypoint -----------------------------------------------------------------------------------------------------------------

int main (int argc, char** argv)
{
	debugInit ();

	if (argc < 2)
	{
		fprintf (stderr, "Invalid arguments, usage: mdf-stub <MDF file name>");
		return -1;
	}

	// Open the MDF file for writing
	char* filePath = argv [argc - 1];
	FILE* mdf = fopen (filePath, "w");
	if (mdf == NULL)
		return errorPrintf ("Failed to create MDF file '%s'", filePath);

	// Start the data log
	if (mdfCanBusLogInit (mdf, "ZREDART", time (NULL)) != 0)
		return errorPrintf ("Failed to initialize CAN bus MDF log");

	canFrame_t frame =
	{
		.id = 0x123,
		.data =
		{
			0x01,
			0x23,
			0x45,
			0x67,
			0x89,
			0xAB,
			0xCD,
			0xEF
		},
		.dlc = 8
	};

	mdfCanBusLogWriteDataFrame (mdf, &frame, 0, 1);
	frame.id = 0x001;
	mdfCanBusLogWriteDataFrame (mdf, &frame, 1000, 1);
	frame.id = 0x010;
	mdfCanBusLogWriteDataFrame (mdf, &frame, 2000, 1);
	frame.id = 0x100;
	mdfCanBusLogWriteDataFrame (mdf, &frame, 3000, 1);

	return 0;
}