// Header
#include "can_eeprom_stdio.h"

int fprintCanEepromConfigHelp (FILE* stream, char* indent)
{
	return fprintf (stream, ""
		"%s<EEPROM Config File>  - The configuration file defining the EEPROM. Must be\n"
		"%s                        a JSON file. This defines the mapping of parameters\n"
		"%s                        in an EEPROM.\n\n",
		indent, indent, indent);
}

int fprintCanEepromDataHelp (FILE* stream, char* indent)
{
	return fprintf (stream, ""
		"%s<EEPROM Data File>    - The data file to use. Must be a JSON file. This\n"
		"%s                        contains the values of each parameter in the EEPROM.\n\n",
		indent, indent);
}