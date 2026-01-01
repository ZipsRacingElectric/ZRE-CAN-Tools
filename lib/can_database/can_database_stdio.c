// Header
#include "can_database_stdio.h"

int fprintCanDbcFileHelp (FILE* stream, char* indent)
{
	return fprintf (stream, ""
		"%s<DBC File>            - The CAN DBC file to use for the given CAN bus.\n\n",
		indent);
}

int fprintCanDatabaseFloatStatic (FILE* stream, const char* formatValue, const char* formatInvalid, float value,
	canDatabaseSignalState_t state, const char* unit)
{
	switch (state)
	{
	case CAN_DATABASE_MISSING:
		// Print '--' as a string using the user-provided format string. The unit is not known, so also use '-'.
		return fprintf (stream, formatInvalid, "--", "-");

	case CAN_DATABASE_TIMEOUT:
		// Print '--' as a string using the user-provided format string. The unit is ignored if not '%s' specifier is added.
		return fprintf (stream, formatInvalid, "--", unit);

	case CAN_DATABASE_VALID:
		// Print the value as a float using the user-provided format string. The unit is ignored if not '%s' specifier is added.
		return fprintf (stream, formatValue, value, unit);
	}

	errno = EINVAL;
	return -1;
}

int snprintCanDatabaseFloatStatic (char* str, size_t n, const char* formatValue, const char* formatInvalid, float value,
	canDatabaseSignalState_t state, const char* unit)
{
	switch (state)
	{
	case CAN_DATABASE_MISSING:
		// Print '--' as a string using the user-provided format string. The unit is not known, so also use '-'.
		return snprintf (str, n, formatInvalid, "--", "-");

	case CAN_DATABASE_TIMEOUT:
		// Print '--' as a string using the user-provided format string. The unit is ignored if not '%s' specifier is added.
		return snprintf (str, n, formatInvalid, "--", unit);

	case CAN_DATABASE_VALID:
		// Print the value as a float using the user-provided format string. The unit is ignored if not '%s' specifier is added.
		return snprintf (str, n, formatValue, value, unit);
	}

	errno = EINVAL;
	return -1;
}

int fprintCanDatabaseFloat (FILE* stream, const char* formatValue, const char* formatInvalid,
	canDatabase_t* database, ssize_t index)
{
	char* unit = NULL;
	canSignal_t* signal = canDatabaseGetSignal (database, index);
	if (signal != NULL)
		unit = signal->unit;

	float value;
	canDatabaseSignalState_t state = canDatabaseGetFloat (database, index, &value);

	return fprintCanDatabaseFloatStatic (stream, formatValue, formatInvalid, value, state, unit);
}

int snprintCanDatabaseFloat (char* str, size_t n, const char* formatValue, const char* formatInvalid,
	canDatabase_t* database, ssize_t index)
{
	char* unit = NULL;
	canSignal_t* signal = canDatabaseGetSignal (database, index);
	if (signal != NULL)
		unit = signal->unit;

	float value;
	canDatabaseSignalState_t state = canDatabaseGetFloat (database, index, &value);

	return snprintCanDatabaseFloatStatic (str, n, formatValue, formatInvalid, value, state, unit);
}