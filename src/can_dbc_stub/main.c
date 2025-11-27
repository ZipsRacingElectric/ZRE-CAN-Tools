#include "can_database/can_dbc_v2.h"
#include "debug.h"

int main (int argc, char** argv)
{
	if (argc < 2)
		return -1;

	debugSetStream (stderr);

	if (canDbcLoad (argv [1]) != 0)
		return errorPrintf ("Failed to load CAN DBC file");
}