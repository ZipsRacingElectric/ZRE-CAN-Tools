// Header
#include "abs_signal.h"

// Includes
#include "uinput_helper.h"
#include "cjson/cjson_util.h"
#include "debug.h"

// Linux
#include <linux/uinput.h>

// POSIX
#include <sys/ioctl.h>

// C Standard Library
#include <stdlib.h>
#include <math.h>

int absSignalInit (absSignal_t* abs, absSignalConfig_t* config, int fd, canDatabase_t* database)
{
	// Store the configuration
	abs->config		= *config;
	abs->fd			= fd;
	abs->database	= database;

	// Find the positive signal
	abs->positiveSignal = canDatabaseFindSignal (database, config->positiveSignalName);
	if (abs->positiveSignal < 0)
		return errno;

	// Find the negative signal, if specified.
	if (config->negativeSignalName != NULL)
	{
		abs->negativeSignal = canDatabaseFindSignal (database, config->negativeSignalName);
		if (abs->negativeSignal < 0)
			return errno;
	}
	else
		abs->negativeSignal = -1;

	// Enable the absolute axis.
	if (ioctl (fd, UI_SET_ABSBIT, config->code) < 0)
	{
		debugPrintf ("ioctl UI_SET_ABSBIT failed.\n");
		return errno;
	}

	// Setup the absolute axis. Use unidirection / bidirectional based on whether a negative signal was specified.
	struct uinput_abs_setup setup =
	{
		.code = config->code,
		.absinfo =
		{
			.maximum	= 255,
			.minimum	= abs->negativeSignal < 0 ? 0 : -255,
			.flat		= 0,
			.fuzz		= 0,
			.resolution	= 1,
			.value		= 0
		}
	};
	if (ioctl (fd, UI_ABS_SETUP, &setup) < 0)
	{
		debugPrintf ("ioctl UI_ABS_SETUP failed.\n");
		return errno;
	}

	// Success
	return 0;
}

absSignal_t* absSignalsLoad (cJSON* config, int fd, canDatabase_t* database, size_t* count)
{
	cJSON* arrayJson;
	if (jsonGetObject (config, "absoluteAxes", &arrayJson) != 0)
		return NULL;

	// Allocate the array
	*count = cJSON_GetArraySize (arrayJson);
	absSignal_t* abs = malloc (sizeof (absSignal_t) * *count);
	if (abs == NULL)
		return NULL;

	// Initialize each axis in the configuration array
	for (size_t index = 0; index < *count; ++index)
	{
		absSignalConfig_t config = {};

		// Get this axis's config
		cJSON* configJson = cJSON_GetArrayItem (arrayJson, index);
		if (configJson == NULL)
		{
			errno = ERRNO_CJSON_PARSE_FAIL;
			return NULL;
		}

		char* positiveSignalName;
		if (jsonGetString (configJson, "positiveSignalName", &positiveSignalName) != 0)
			return NULL;
		config.positiveSignalName = positiveSignalName;

		if (jsonGetFloat (configJson, "positiveZero", &config.positiveZero) != 0)
			return NULL;

		if (jsonGetFloat (configJson, "positiveMax", &config.positiveMax) != 0)
			return NULL;

		char* negativeSignalName;
		if (jsonGetString (configJson, "negativeSignalName", &negativeSignalName) == 0)
		{
			config.negativeSignalName = negativeSignalName;

			if (jsonGetFloat (configJson, "negativeZero", &config.negativeZero) != 0)
				return NULL;

			if (jsonGetFloat (configJson, "negativeMax", &config.negativeMax) != 0)
				return NULL;
		}
		else
			config.negativeSignalName = NULL;

		char* codeAlias;
		if (jsonGetString (configJson, "code", &codeAlias) != 0)
			return NULL;

		config.code = uinputAbsAlias (codeAlias);
		if (config.code < 0)
			return NULL;

		debugPrintf (
			"Loaded absolute axis signal:\n"
			"    positiveSignalName = '%s'\n"
			"    negativeSignalName = '%s'\n"
			"    code = %i\n"
			"    positiveZero = %f\n"
			"    positiveMax = %f\n"
			"    negativeZero = %f\n"
			"    negativeMax = %f\n",
			config.positiveSignalName, config.negativeSignalName == NULL ? "null" : config.negativeSignalName, config.code,
			config.positiveZero, config.positiveMax, config.negativeZero, config.negativeMax);

		if (absSignalInit (&abs [index], &config, fd, database) != 0)
			return NULL;
	}

	return abs;
}

int absSignalUpdate (absSignal_t* abs)
{
	int positiveInt = 0;
	float value;
	if (canDatabaseGetFloat (abs->database, abs->positiveSignal, &value) == CAN_DATABASE_VALID)
	{
		positiveInt = (int) roundf (255 / (abs->config.positiveMax - abs->config.positiveZero) * (value - abs->config.positiveZero));
		if (positiveInt > 255)
			positiveInt = 255;
		if (positiveInt < 0)
			positiveInt = 0;
	}

	int negativeInt = 0;
	if (abs->negativeSignal >= 0 && canDatabaseGetFloat (abs->database, abs->negativeSignal, &value) == CAN_DATABASE_VALID)
	{
		negativeInt = (int) roundf (255 / (abs->config.negativeZero - abs->config.negativeMax) * (value - abs->config.negativeMax) - 255);
		if (negativeInt > 0)
			negativeInt = 0;
		if (negativeInt < -255)
			negativeInt = -255;
	}

	return uinputEmit (abs->fd, EV_ABS, abs->config.code, positiveInt + negativeInt);
}