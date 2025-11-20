#include <fcntl.h>
#include <sys/ioctl.h>
#include <linux/uinput.h>
#include <unistd.h>
#include <string.h>

#include "debug.h"
#include "can_device/can_device.h"
#include "can_database/can_database.h"

typedef struct
{
	canDatabase_t* database;
	const char* name;
	float threshold;
	bool inverted;
	int key;
	int descriptor;
} keySignalConfig_t;

typedef struct
{
	keySignalConfig_t config;
	ssize_t index;
	bool pressed;
} keySignal_t;

void emit (int descriptor, int type, int code, int val)
{
	struct input_event ie;

	ie.type = type;
	ie.code = code;
	ie.value = val;
	/* timestamp values below are ignored */
	ie.time.tv_sec = 0;
	ie.time.tv_usec = 0;

	write (descriptor, &ie, sizeof(ie));
}

int keySignalInit (keySignal_t* keySignal, keySignalConfig_t* config)
{
	keySignal->config = *config;

	keySignal->index = canDatabaseFindSignal (config->database, config->name);
	if (keySignal->index < 0)
		return errno;

	ioctl (config->descriptor, UI_SET_KEYBIT, config->key);

	keySignal->pressed = false;
	return 0;
}

int keySignalUpdate (keySignal_t* keySignal)
{
	float value;

	if (canDatabaseGetFloat (keySignal->config.database, keySignal->index, &value) != CAN_DATABASE_VALID)
		value = 0;

	if ((value >= keySignal->config.threshold) != keySignal->config.inverted && !keySignal->pressed)
	{
		keySignal->pressed = true;
		emit (keySignal->config.descriptor, EV_KEY, keySignal->config.key, 1);
		emit (keySignal->config.descriptor, EV_SYN, SYN_REPORT, 0);
	}

	if ((value < keySignal->config.threshold) != keySignal->config.inverted && keySignal->pressed)
	{
		keySignal->pressed = false;
		emit (keySignal->config.descriptor, EV_KEY, keySignal->config.key, 0);
		emit (keySignal->config.descriptor, EV_SYN, SYN_REPORT, 0);
	}

	return 0;
}

int main (int argc, char** argv)
{
	debugInit ();

	debugSetStream (stderr);

	struct uinput_setup usetup;

	int descriptor = open("/dev/uinput", O_WRONLY | O_NONBLOCK);
	if (descriptor < 0)
		return errorPrintf ("Failed to open /dev/uinput");

	ioctl (descriptor, UI_SET_EVBIT, EV_KEY);

	char* deviceName = argv [argc - 2];
	canDevice_t* device = canInit (deviceName);
	if (device == NULL)
		return errorPrintf ("Failed to initialize CAN device '%s'", deviceName);

	char* dbcPath = argv [argc - 1];
	canDatabase_t database;
	if (canDatabaseInit (&database, device, dbcPath) != 0)
		return errorPrintf ("Failed to initialize CAN database");

	keySignal_t keyBml;
	if (keySignalInit (&keyBml, &(keySignalConfig_t)
		{
			.database = &database,
			.key = KEY_C,
			.name = "BUTTON_MID_LEFT",
			.inverted = true,
			.threshold = 0.5,
			.descriptor = descriptor
		}) != 0)
		return errorPrintf ("Failed to initialize key signal");

	keySignal_t keyBbl;
	if (keySignalInit (&keyBbl, &(keySignalConfig_t)
		{
			.database = &database,
			.key = KEY_X,
			.name = "BUTTON_BOTTOM_LEFT",
			.inverted = true,
			.threshold = 0.5,
			.descriptor = descriptor
		}) != 0)
		return errorPrintf ("Failed to initialize key signal");

	keySignal_t keyBtl;
	if (keySignalInit (&keyBtl, &(keySignalConfig_t)
		{
			.database = &database,
			.key = KEY_SPACE,
			.name = "BUTTON_TOP_LEFT",
			.inverted = true,
			.threshold = 0.5,
			.descriptor = descriptor
		}) != 0)
		return errorPrintf ("Failed to initialize key signal");

	keySignal_t keyStrd;
	if (keySignalInit (&keyStrd, &(keySignalConfig_t)
		{
			.database = &database,
			.key = KEY_ENTER,
			.name = "SWITCH_TOP_RIGHT_DOWN",
			.inverted = true,
			.threshold = 0.5,
			.descriptor = descriptor
		}) != 0)
		return errorPrintf ("Failed to initialize key signal");

	keySignal_t keyStru;
	if (keySignalInit (&keyStru, &(keySignalConfig_t)
		{
			.database = &database,
			.key = KEY_ESC,
			.name = "SWITCH_TOP_RIGHT_UP",
			.inverted = true,
			.threshold = 0.5,
			.descriptor = descriptor
		}) != 0)
		return errorPrintf ("Failed to initialize key signal");

	keySignal_t keyBmr;
	if (keySignalInit (&keyBmr, &(keySignalConfig_t)
		{
			.database = &database,
			.key = KEY_UP,
			.name = "BUTTON_MID_RIGHT",
			.inverted = true,
			.threshold = 0.5,
			.descriptor = descriptor
		}) != 0)
		return errorPrintf ("Failed to initialize key signal");

	keySignal_t keyBbr;
	if (keySignalInit (&keyBbr, &(keySignalConfig_t)
		{
			.database = &database,
			.key = KEY_DOWN,
			.name = "BUTTON_BOTTOM_RIGHT",
			.inverted = true,
			.threshold = 0.5,
			.descriptor = descriptor
		}) != 0)
		return errorPrintf ("Failed to initialize key signal");

	keySignal_t keyLPad;
	if (keySignalInit (&keyLPad, &(keySignalConfig_t)
		{
			.database = &database,
			.key = KEY_LEFT,
			.name = "PADDLE_LEFT_PERCENT",
			.inverted = true,
			.threshold = 50,
			.descriptor = descriptor
		}) != 0)
		return errorPrintf ("Failed to initialize key signal");

	keySignal_t keyRPad;
	if (keySignalInit (&keyRPad, &(keySignalConfig_t)
		{
			.database = &database,
			.key = KEY_RIGHT,
			.name = "PADDLE_RIGHT_PERCENT",
			.inverted = true,
			.threshold = 50,
			.descriptor = descriptor
		}) != 0)
		return errorPrintf ("Failed to initialize key signal");

	memset(&usetup, 0, sizeof(usetup));
	usetup.id.bustype = BUS_USB;
	usetup.id.vendor = 0x1234; /* sample vendor */
	usetup.id.product = 0x5678; /* sample product */
	strcpy(usetup.name, "Example device");

	ioctl (descriptor, UI_DEV_SETUP, &usetup);
	ioctl (descriptor, UI_DEV_CREATE);

	while (1)
	{
		keySignalUpdate (&keyBtl);
		keySignalUpdate (&keyBml);
		keySignalUpdate (&keyBbl);
		keySignalUpdate (&keyStru);
		keySignalUpdate (&keyStrd);
		keySignalUpdate (&keyBmr);
		keySignalUpdate (&keyBbr);
		keySignalUpdate (&keyLPad);
		keySignalUpdate (&keyRPad);

		struct timespec period = { .tv_sec = 0, .tv_nsec = 10000000 };
		nanosleep (&period, NULL);
	}

	ioctl (descriptor, UI_DEV_DESTROY);
	close (descriptor);

	return 0;
}