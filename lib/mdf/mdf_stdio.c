// Header
#include "mdf_stdio.h"

int fprintMdfFileHelp (FILE* stream, char* indent)
{
	return fprintf (stream, ""
		"%s<MDF File>            - The MDF file to use. Should be have the '.mf4' file\n"
		"%s                        extension.\n\n",
		indent, indent);
}