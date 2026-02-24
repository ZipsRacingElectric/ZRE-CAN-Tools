// Header
#include "fault_signal.h"

// Includes
#include "cjson/cjson_util.h"

static int loadSignal (faultSignal_t* signal, cJSON* config, canDatabase_t* database)
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

static faultSignalState_t signalCheck (faultSignal_t* signal, canDatabase_t* database)
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

int faultSignalsLoad (faultSignals_t* faults, cJSON* config, canDatabase_t* database)
{
	// Load the fault signal array
	cJSON* faultConfig;
	if (jsonGetObject (config, "faults", &faultConfig) != 0)
		return errno;

	// Get the size of the config array and allocate the fault array
	faults->count = cJSON_GetArraySize (faultConfig);
	faults->signals = malloc (faults->count * sizeof (faultSignal_t));
	if (faults == NULL)
		return errno;

	// Load each element of the array
	for (size_t index = 0; index < faults->count; ++index)
	{
		cJSON* config = cJSON_GetArrayItem (faultConfig, index);
		if (loadSignal (&faults->signals [index], config, database) != 0)
		{
			// Deallocate and exit early
			free (faults);
			return errno;
		}
	}

	// Save the CAN database reference
	faults->database = database;

	// Success
	return 0;
}

void faultSignalsDealloc (faultSignals_t* faults)
{
	free (faults->signals);
}

faultSignalState_t faultSignalsGetIndex (faultSignals_t* faults, size_t* index)
{
	// Iterate over each signal.
	for (size_t i = 0; i < faults->count; ++i)
	{
		// Check the signal, if okay, then skip.
		faultSignalState_t state = signalCheck (&faults->signals [i], faults->database);
		if (state == FAULT_SIGNAL_OKAY)
			continue;

		// If not okay, write the index and return the state
		*index = i;
		return state;
	}

	// No faults.
	return FAULT_SIGNAL_OKAY;
}

char* faultSignalsGetString (faultSignals_t* faults)
{
	// Get the state and index, then return the code.
	size_t index;
	switch (faultSignalsGetIndex (faults, &index))
	{
	case FAULT_SIGNAL_MISSING:
		return "MISSING";

	case FAULT_SIGNAL_TIMEOUT:
		return "TIMEOUT";

	case FAULT_SIGNAL_FAULTED:
		return faults->signals [index].name;

	case FAULT_SIGNAL_OKAY:
		return "OKAY";
	}

	return "OKAY";
}