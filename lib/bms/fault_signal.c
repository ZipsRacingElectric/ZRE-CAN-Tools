// Header
#include "fault_signal.h"

// Includes
#include "cjson/cjson_util.h"

int faultSignalLoad (faultSignal_t* signal, cJSON* config, canDatabase_t* database)
{
	// Get the fault name, fail if not provided.
	if (jsonGetString (config, "name", &signal->name) != 0)
		return errno;

	// Get the signal name, fail if not provided.
	char* signalName;
	if (jsonGetString (config, "signal", &signalName) != 0)
		return errno;

	// Lookup the signal index, fail if it is missing.
	signal->index = canDatabaseFindSignal (database, signalName);
	if (signal->index < 0)
		return errno;

	// Get the threshold value, use default if not provided.
	if (jsonGetFloat (config, "threshold", &signal->threshold) != 0)
		signal->threshold = 0.5f;

	// Get the threshold value, use default if not provided.
	if (jsonGetBool (config, "inverted", &signal->inverted) != 0)
		signal->inverted = false;

	return 0;
}

faultSignal_t* faultSignalsLoad (cJSON* configArray, size_t* count, canDatabase_t* database)
{
	// Get the size of the array and allocate
	*count = cJSON_GetArraySize (configArray);
	faultSignal_t* faults = malloc (*count * sizeof (faultSignal_t));
	if (faults == NULL)
		return NULL;

	// Init each element of the array
	for (size_t index = 0; index < *count; ++index)
	{
		cJSON* config = cJSON_GetArrayItem (configArray, index);
		if (faultSignalLoad (&faults [index], config, database) != 0)
		{
			// Deallocate and exit early
			free (faults);
			return NULL;
		}
	}

	return faults;
}

faultSignalState_t faultSignalCheck (canDatabase_t* database, faultSignal_t* signal)
{
	// Get the signal value
	float value;
	switch (canDatabaseGetFloat (database, signal->index, &value))
	{
	case CAN_DATABASE_MISSING:
		return FAULT_SIGNAL_MISSING;

	case CAN_DATABASE_TIMEOUT:
		return FAULT_SIGNAL_TIMEOUT;

	default:
		break;
	}

	// Check fault range
	if ((value >= signal->threshold) != signal->inverted)
		return FAULT_SIGNAL_FAULTED;

	// No fault.
	return FAULT_SIGNAL_OKAY;
}

char* faultSignalsCheck (canDatabase_t* database, faultSignal_t* signals, size_t signalCount)
{
	// Check each signal. If a fault is present, return the appropriate code.
	for (size_t index = 0; index < signalCount; ++index)
	{
		switch (faultSignalCheck (database, &signals [index]))
		{
		case FAULT_SIGNAL_MISSING:
			return "MISSING";
		case FAULT_SIGNAL_TIMEOUT:
			return "TIMEOUT";
		case FAULT_SIGNAL_FAULTED:
			return signals [index].name;
		default:
			break;
		}
	}

	// No faults.
	return "OKAY";
}