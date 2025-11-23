// Header
#include "key_signal.h"

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

int keySignalInit (keySignal_t* key, keySignalConfig_t* config, int fd, canDatabase_t* database)
{
	// Store the configuration
	key->config		= *config;
	key->fd			= fd;
	key->database	= database;

	// Find the CAN signal
	key->index = canDatabaseFindSignal (database, config->signalName);
	if (key->index < 0)
		return errno;

	// Enable the key
	if (ioctl (fd, UI_SET_KEYBIT, config->code) < 0)
	{
		debugPrintf ("ioctl UI_SET_KEYBIT failed.\n");
		return errno;
	}

	// Success
	key->pressed = false;
	return 0;
}

keySignal_t* keySignalsLoad (cJSON* json, int fd, canDatabase_t* database, size_t* count)
{
	cJSON* keyArrayJson;
	if (jsonGetObject (json, "keys", &keyArrayJson) != 0)
		return NULL;

	// Allocate the array
	*count = cJSON_GetArraySize (keyArrayJson);
	keySignal_t* signals = malloc (sizeof (keySignal_t) * *count);
	if (signals == NULL)
		return NULL;

	for (size_t index = 0; index < *count; ++index)
	{
		keySignalConfig_t config = {};

		// Get the key's config
		cJSON* configJson = cJSON_GetArrayItem (keyArrayJson, index);
		if (configJson == NULL)
		{
			errno = ERRNO_CJSON_PARSE_FAIL;
			return NULL;
		}

		char* signalName;
		if (jsonGetString (configJson, "signalName", &signalName) != 0)
			return NULL;
		config.signalName = signalName;

		char* codeAlias;
		if (jsonGetString (configJson, "code", &codeAlias) != 0)
			return NULL;

		config.code = uinputKeyAlias (codeAlias);
		if (config.code < 0)
			return NULL;

		if (jsonGetBool (configJson, "inverted", &config.inverted) != 0)
			config.inverted = false;

		if (jsonGetFloat (configJson, "threshold", &config.threshold) != 0)
			config.threshold = 0.5f;

		debugPrintf (
			"Loaded signal key:\n"
			"    signalName = '%s'\n"
			"    code = %i\n"
			"    inverted = %i\n"
			"    threshold = %f\n",
			config.signalName, config.code, config.inverted, config.threshold);

		if (keySignalInit (&signals [index], &config, fd, database) != 0)
			return NULL;
	}

	return signals;
}

int keySignalUpdate (keySignal_t* key)
{
	float value;

	if (canDatabaseGetFloat (key->database, key->index, &value) == CAN_DATABASE_VALID)
	{
		if ((value >= key->config.threshold) != key->config.inverted && !key->pressed)
		{
			key->pressed = true;
			return uinputEmit (key->fd, EV_KEY, key->config.code, 1);
		}

		if ((value < key->config.threshold) != key->config.inverted && key->pressed)
		{
			key->pressed = false;
			return uinputEmit (key->fd, EV_KEY, key->config.code, 0);
		}
	}
	else if (key->pressed)
	{
		key->pressed = false;
		return uinputEmit (key->fd, EV_KEY, key->config.code, 0);
	}

	return 0;
}