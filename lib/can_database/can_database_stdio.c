// Header
#include "can_database_stdio.h"

int fprintCanDbcFileHelp (FILE* stream, char* indent)
{
	return fprintf (stream, ""
		"%s<DBC File>            - The CAN DBC file to use for the given CAN bus.\n\n",
		indent);
}