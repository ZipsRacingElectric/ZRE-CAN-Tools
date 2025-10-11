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
		fprintf (stderr, "Invalid arguments, usage: data-logger <options> <CAN device>.\n");
		return -1;
	}

	FILE* file = stdout;
	for (int index = 1; index < argc - 1; ++index)
	{
		if (argv [index][0] == '-' && argv [index][1] == 'f' && argv [index][2] == '=')
		{
			file = fopen (argv [index] + 3, "w");
			if (file == NULL)
			{
				int code = errno;
				fprintf (stderr, "Failed to open MDF file: %s.\n", errorMessage (code));
				return code;
			}
			continue;
		}

		fprintf (stderr, "Warning: Unrecognized option '%s'.\n", argv [index]);
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

		fprintf (file, "0x%03X", frame.id);
		for (size_t index = 0; index < frame.dlc; ++index)
			fprintf (file, "[0x%02X]", frame.data [index]);
		fprintf (file, "\n");
	}

	return 0;
}