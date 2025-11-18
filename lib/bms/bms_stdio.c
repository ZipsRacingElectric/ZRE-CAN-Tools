// Header
#include "bms_stdio.h"

int fprintBmsConfigFileHelp (FILE* stream, char* indent)
{
	return fprintf (stream, ""
		"%s<BMS Config File>     - The configuration file defining the BMS. Must be a\n"
		"%s                        JSON file.\n\n",
		indent, indent);
}