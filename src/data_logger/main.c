// Includes
#include "can_device/can_device.h"
#include "debug.h"
#include "error_codes.h"

// C Standard Library
#include <errno.h>

int main (int argc, char** argv)
{
	debugInit ();

	if (argc < 2)
	{
		fprintf (stderr, "Invalid arguments, usage: data-logger <CAN device>.\n");
		return -1;
	}

	char* deviceName = argv [argc - 1];
	canDevice_t* device = canInit (deviceName);
	if (device == NULL)
	{
		int code = errno;
		fprintf (stderr, "Failed to initialize CAN device: %s.\n", errorMessage (code));
		return code;
	}

	while (1)
	{
		canFrame_t frame;
		if (canReceive (device, &frame) != 0)
		{
			fprintf (stderr, "Warning, failed to recieve CAN frame: %s.\n", errorMessage (errno));
			continue;
		}

		printf ("0x%03X", frame.id);
		for (size_t index = 0; index < frame.dlc; ++index)
			printf ("[0x%02X]", frame.data [index]);
		printf ("\n");
	}

	return 0;
}