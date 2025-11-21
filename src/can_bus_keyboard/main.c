#include <fcntl.h>
#include <sys/ioctl.h>
#include <linux/uinput.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>

#include "debug.h"
#include "can_device/can_device.h"
#include "can_database/can_database.h"
#include "cjson/cjson_util.h"

typedef struct
{
	canDatabase_t* database;
	const char* databaseKey;
	int keyboardKey;
	float threshold;
	bool inverted;
	int descriptor;
} keySignalConfig_t;

typedef struct
{
	keySignalConfig_t config;
	ssize_t index;
	bool pressed;
} keySignal_t;

#define keyCase(str, key)			\
	if (strcmp (str, #key) == 0)	\
		return key

int keyStrToInt (const char* str)
{
	keyCase (str, KEY_ESC);
	keyCase (str, KEY_1);
	keyCase (str, KEY_2);
	keyCase (str, KEY_3);
	keyCase (str, KEY_4);
	keyCase (str, KEY_5);
	keyCase (str, KEY_6);
	keyCase (str, KEY_7);
	keyCase (str, KEY_8);
	keyCase (str, KEY_9);
	keyCase (str, KEY_0);
	keyCase (str, KEY_MINUS);
	keyCase (str, KEY_EQUAL);
	keyCase (str, KEY_BACKSPACE);
	keyCase (str, KEY_TAB);
	keyCase (str, KEY_Q);
	keyCase (str, KEY_W);
	keyCase (str, KEY_E);
	keyCase (str, KEY_R);
	keyCase (str, KEY_T);
	keyCase (str, KEY_Y);
	keyCase (str, KEY_U);
	keyCase (str, KEY_I);
	keyCase (str, KEY_O);
	keyCase (str, KEY_P);
	keyCase (str, KEY_LEFTBRACE);
	keyCase (str, KEY_RIGHTBRACE);
	keyCase (str, KEY_ENTER);
	keyCase (str, KEY_LEFTCTRL);
	keyCase (str, KEY_A);
	keyCase (str, KEY_S);
	keyCase (str, KEY_D);
	keyCase (str, KEY_F);
	keyCase (str, KEY_G);
	keyCase (str, KEY_H);
	keyCase (str, KEY_J);
	keyCase (str, KEY_K);
	keyCase (str, KEY_L);
	keyCase (str, KEY_SEMICOLON);
	keyCase (str, KEY_APOSTROPHE);
	keyCase (str, KEY_GRAVE);
	keyCase (str, KEY_LEFTSHIFT);
	keyCase (str, KEY_BACKSLASH);
	keyCase (str, KEY_Z);
	keyCase (str, KEY_X);
	keyCase (str, KEY_C);
	keyCase (str, KEY_V);
	keyCase (str, KEY_B);
	keyCase (str, KEY_N);
	keyCase (str, KEY_M);
	keyCase (str, KEY_COMMA);
	keyCase (str, KEY_DOT);
	keyCase (str, KEY_SLASH);
	keyCase (str, KEY_RIGHTSHIFT);
	keyCase (str, KEY_KPASTERISK);
	keyCase (str, KEY_LEFTALT);
	keyCase (str, KEY_SPACE);
	keyCase (str, KEY_CAPSLOCK);
	keyCase (str, KEY_F1);
	keyCase (str, KEY_F2);
	keyCase (str, KEY_F3);
	keyCase (str, KEY_F4);
	keyCase (str, KEY_F5);
	keyCase (str, KEY_F6);
	keyCase (str, KEY_F7);
	keyCase (str, KEY_F8);
	keyCase (str, KEY_F9);
	keyCase (str, KEY_F10);
	keyCase (str, KEY_NUMLOCK);
	keyCase (str, KEY_SCROLLLOCK);
	keyCase (str, KEY_KP7);
	keyCase (str, KEY_KP8);
	keyCase (str, KEY_KP9);
	keyCase (str, KEY_KPMINUS);
	keyCase (str, KEY_KP4);
	keyCase (str, KEY_KP5);
	keyCase (str, KEY_KP6);
	keyCase (str, KEY_KPPLUS);
	keyCase (str, KEY_KP1);
	keyCase (str, KEY_KP2);
	keyCase (str, KEY_KP3);
	keyCase (str, KEY_KP0);
	keyCase (str, KEY_KPDOT);

	keyCase (str, KEY_F11);
	keyCase (str, KEY_F12);

	keyCase (str, KEY_RIGHTCTRL);
	keyCase (str, KEY_RIGHTALT);
	keyCase (str, KEY_LINEFEED);
	keyCase (str, KEY_HOME);
	keyCase (str, KEY_UP);
	keyCase (str, KEY_PAGEUP);
	keyCase (str, KEY_LEFT);
	keyCase (str, KEY_RIGHT);
	keyCase (str, KEY_END);
	keyCase (str, KEY_DOWN);
	keyCase (str, KEY_PAGEDOWN);
	keyCase (str, KEY_INSERT);
	keyCase (str, KEY_DELETE);
	keyCase (str, KEY_MACRO);
	keyCase (str, KEY_MUTE);
	keyCase (str, KEY_VOLUMEDOWN);
	keyCase (str, KEY_VOLUMEUP);
	keyCase (str, KEY_POWER);
	keyCase (str, KEY_PAUSE);
	keyCase (str, KEY_SCALE);

	keyCase (str, KEY_LEFTMETA);
	keyCase (str, KEY_RIGHTMETA);

	keyCase (str, BTN_MISC);
	keyCase (str, BTN_0);
	keyCase (str, BTN_1);
	keyCase (str, BTN_2);
	keyCase (str, BTN_3);
	keyCase (str, BTN_4);
	keyCase (str, BTN_5);
	keyCase (str, BTN_6);
	keyCase (str, BTN_7);
	keyCase (str, BTN_8);
	keyCase (str, BTN_9);

	keyCase (str, BTN_MOUSE);
	keyCase (str, BTN_LEFT);
	keyCase (str, BTN_RIGHT);
	keyCase (str, BTN_MIDDLE);
	keyCase (str, BTN_SIDE);
	keyCase (str, BTN_EXTRA);
	keyCase (str, BTN_FORWARD);
	keyCase (str, BTN_BACK);
	keyCase (str, BTN_TASK);

	keyCase (str, BTN_JOYSTICK);
	keyCase (str, BTN_TRIGGER);
	keyCase (str, BTN_THUMB);
	keyCase (str, BTN_THUMB2);
	keyCase (str, BTN_TOP);
	keyCase (str, BTN_TOP2);
	keyCase (str, BTN_PINKIE);
	keyCase (str, BTN_BASE);
	keyCase (str, BTN_BASE2);
	keyCase (str, BTN_BASE3);
	keyCase (str, BTN_BASE4);
	keyCase (str, BTN_BASE5);
	keyCase (str, BTN_BASE6);
	keyCase (str, BTN_DEAD);

	keyCase (str, BTN_GAMEPAD);
	keyCase (str, BTN_SOUTH);
	keyCase (str, BTN_A);
	keyCase (str, BTN_EAST);
	keyCase (str, BTN_B);
	keyCase (str, BTN_C);
	keyCase (str, BTN_NORTH);
	keyCase (str, BTN_X);
	keyCase (str, BTN_WEST);
	keyCase (str, BTN_Y);
	keyCase (str, BTN_Z);
	keyCase (str, BTN_TL);
	keyCase (str, BTN_TR);
	keyCase (str, BTN_TL2);
	keyCase (str, BTN_TR2);
	keyCase (str, BTN_SELECT);
	keyCase (str, BTN_START);
	keyCase (str, BTN_MODE);
	keyCase (str, BTN_THUMBL);
	keyCase (str, BTN_THUMBR);

	keyCase (str, BTN_DIGI);
	keyCase (str, BTN_TOOL_PEN);
	keyCase (str, BTN_TOOL_RUBBER);
	keyCase (str, BTN_TOOL_BRUSH);
	keyCase (str, BTN_TOOL_PENCIL);
	keyCase (str, BTN_TOOL_AIRBRUSH);
	keyCase (str, BTN_TOOL_FINGER);
	keyCase (str, BTN_TOOL_MOUSE);
	keyCase (str, BTN_TOOL_LENS);
	keyCase (str, BTN_TOOL_QUINTTAP);
	keyCase (str, BTN_STYLUS3);
	keyCase (str, BTN_TOUCH);
	keyCase (str, BTN_STYLUS);
	keyCase (str, BTN_STYLUS2);
	keyCase (str, BTN_TOOL_DOUBLETAP);
	keyCase (str, BTN_TOOL_TRIPLETAP);
	keyCase (str, BTN_TOOL_QUADTAP);

	keyCase (str, BTN_WHEEL);
	keyCase (str, BTN_GEAR_DOWN);
	keyCase (str, BTN_GEAR_UP);

	keyCase (str, KEY_OK);
	keyCase (str, KEY_SELECT);
	keyCase (str, KEY_GOTO);
	keyCase (str, KEY_CLEAR);
	keyCase (str, KEY_POWER2);
	keyCase (str, KEY_OPTION);
	keyCase (str, KEY_INFO);
	keyCase (str, KEY_TIME);
	keyCase (str, KEY_VENDOR);
	keyCase (str, KEY_ARCHIVE);
	keyCase (str, KEY_PROGRAM);
	keyCase (str, KEY_CHANNEL);
	keyCase (str, KEY_FAVORITES);
	keyCase (str, KEY_EPG);
	keyCase (str, KEY_PVR);
	keyCase (str, KEY_MHP);
	keyCase (str, KEY_LANGUAGE);
	keyCase (str, KEY_TITLE);
	keyCase (str, KEY_SUBTITLE);
	keyCase (str, KEY_ANGLE);
	keyCase (str, KEY_FULL_SCREEN);
	keyCase (str, KEY_ZOOM);
	keyCase (str, KEY_MODE);
	keyCase (str, KEY_KEYBOARD);
	keyCase (str, KEY_ASPECT_RATIO);
	keyCase (str, KEY_SCREEN);
	keyCase (str, KEY_PC);
	keyCase (str, KEY_TV);
	keyCase (str, KEY_TV2);
	keyCase (str, KEY_VCR);
	keyCase (str, KEY_VCR2);
	keyCase (str, KEY_SAT);
	keyCase (str, KEY_SAT2);
	keyCase (str, KEY_CD);
	keyCase (str, KEY_TAPE);
	keyCase (str, KEY_RADIO);
	keyCase (str, KEY_TUNER);
	keyCase (str, KEY_PLAYER);
	keyCase (str, KEY_TEXT);
	keyCase (str, KEY_DVD);
	keyCase (str, KEY_AUX);
	keyCase (str, KEY_MP3);
	keyCase (str, KEY_AUDIO);
	keyCase (str, KEY_VIDEO);
	keyCase (str, KEY_DIRECTORY);
	keyCase (str, KEY_LIST);
	keyCase (str, KEY_MEMO);
	keyCase (str, KEY_CALENDAR);
	keyCase (str, KEY_RED);
	keyCase (str, KEY_GREEN);
	keyCase (str, KEY_YELLOW);
	keyCase (str, KEY_BLUE);
	keyCase (str, KEY_CHANNELUP);
	keyCase (str, KEY_CHANNELDOWN);
	keyCase (str, KEY_FIRST);
	keyCase (str, KEY_LAST);
	keyCase (str, KEY_AB);
	keyCase (str, KEY_NEXT);
	keyCase (str, KEY_RESTART);
	keyCase (str, KEY_SLOW);
	keyCase (str, KEY_SHUFFLE);
	keyCase (str, KEY_BREAK);
	keyCase (str, KEY_PREVIOUS);
	keyCase (str, KEY_DIGITS);
	keyCase (str, KEY_TEEN);
	keyCase (str, KEY_TWEN);
	keyCase (str, KEY_VIDEOPHONE);
	keyCase (str, KEY_GAMES);
	keyCase (str, KEY_ZOOMIN);
	keyCase (str, KEY_ZOOMOUT);
	keyCase (str, KEY_ZOOMRESET);
	keyCase (str, KEY_WORDPROCESSOR);
	keyCase (str, KEY_EDITOR);
	keyCase (str, KEY_SPREADSHEET);
	keyCase (str, KEY_GRAPHICSEDITOR);
	keyCase (str, KEY_PRESENTATION);
	keyCase (str, KEY_DATABASE);
	keyCase (str, KEY_NEWS);
	keyCase (str, KEY_VOICEMAIL);
	keyCase (str, KEY_ADDRESSBOOK);
	keyCase (str, KEY_MESSENGER);
	keyCase (str, KEY_DISPLAYTOGGLE);
	keyCase (str, KEY_BRIGHTNESS_TOGGLE);
	keyCase (str, KEY_SPELLCHECK);
	keyCase (str, KEY_LOGOFF);

	keyCase (str, KEY_FN);
	keyCase (str, KEY_FN_ESC);
	keyCase (str, KEY_FN_F1);
	keyCase (str, KEY_FN_F2);
	keyCase (str, KEY_FN_F3);
	keyCase (str, KEY_FN_F4);
	keyCase (str, KEY_FN_F5);
	keyCase (str, KEY_FN_F6);
	keyCase (str, KEY_FN_F7);
	keyCase (str, KEY_FN_F8);
	keyCase (str, KEY_FN_F9);
	keyCase (str, KEY_FN_F10);
	keyCase (str, KEY_FN_F11);
	keyCase (str, KEY_FN_F12);
	keyCase (str, KEY_FN_1);
	keyCase (str, KEY_FN_2);
	keyCase (str, KEY_FN_D);
	keyCase (str, KEY_FN_E);
	keyCase (str, KEY_FN_F);
	keyCase (str, KEY_FN_S);
	keyCase (str, KEY_FN_B);
	keyCase (str, KEY_FN_RIGHT_SHIFT);

	keyCase (str, KEY_RIGHT_UP);
	keyCase (str, KEY_RIGHT_DOWN);
	keyCase (str, KEY_LEFT_UP);
	keyCase (str, KEY_LEFT_DOWN);

	debugPrintf ("Unknown keyboard key string '%s'.\n", str);
	return -1;
}

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

	keySignal->index = canDatabaseFindSignal (config->database, config->databaseKey);
	if (keySignal->index < 0)
		return errno;

	ioctl (config->descriptor, UI_SET_KEYBIT, config->keyboardKey);

	keySignal->pressed = false;
	return 0;
}

keySignal_t* keySignalsLoad (const char* path, int descriptor, canDatabase_t* database, size_t* count)
{
	cJSON* config = jsonLoad (path);
	if (config == NULL)
		return NULL;

	cJSON* signalKeys;
	if (jsonGetObject (config, "signalKeys", &signalKeys) != 0)
		return NULL;

	*count = cJSON_GetArraySize (signalKeys);
	keySignal_t* signals = malloc (sizeof (keySignal_t) * *count);
	if (signals == NULL)
		return NULL;

	for (size_t index = 0; index < *count; ++index)
	{
		keySignalConfig_t config =
		{
			.database = database,
			.descriptor = descriptor
		};

		cJSON* json = cJSON_GetArrayItem (signalKeys, index);
		if (json == NULL)
		{
			errno = ERRNO_CJSON_PARSE_FAIL;
			return NULL;
		}

		char* databaseKey;
		if (jsonGetString (json, "databaseKey", &databaseKey) != 0)
			return NULL;
		config.databaseKey = databaseKey;

		char* keyboardKey;
		if (jsonGetString (json, "keyboardKey", &keyboardKey) != 0)
			return NULL;

		config.keyboardKey = keyStrToInt (keyboardKey);
		if (config.keyboardKey < 0)
		{
			errno = EINVAL;
			return NULL;
		}

		if (jsonGetBool (json, "inverted", &config.inverted) != 0)
			config.inverted = false;

		if (jsonGetFloat (json, "threshold", &config.threshold) != 0)
			config.threshold = 0.5f;

		debugPrintf (
			"Loaded key:\n"
			"    databaseKey = '%s'\n"
			"    keyboardKey = %i\n"
			"    inverted = %i\n"
			"    threshold = %f\n",
			config.databaseKey, config.keyboardKey, config.inverted, config.threshold);

		if (keySignalInit (&signals [index], &config) != 0)
			return NULL;
	}

	return signals;
}

int keySignalUpdate (keySignal_t* keySignal)
{
	float value;

	if (canDatabaseGetFloat (keySignal->config.database, keySignal->index, &value) != CAN_DATABASE_VALID)
		value = 0;

	if ((value >= keySignal->config.threshold) != keySignal->config.inverted && !keySignal->pressed)
	{
		keySignal->pressed = true;
		emit (keySignal->config.descriptor, EV_KEY, keySignal->config.keyboardKey, 1);
		emit (keySignal->config.descriptor, EV_SYN, SYN_REPORT, 0);
	}

	if ((value < keySignal->config.threshold) != keySignal->config.inverted && keySignal->pressed)
	{
		keySignal->pressed = false;
		emit (keySignal->config.descriptor, EV_KEY, keySignal->config.keyboardKey, 0);
		emit (keySignal->config.descriptor, EV_SYN, SYN_REPORT, 0);
	}

	return 0;
}

int main (int argc, char** argv)
{
	debugInit ();

	debugSetStream (stderr);

	if (argc < 4)
	{
		// TODO(Barach): Usage
		fprintf (stderr, "Invalid usage.\n");
		return -1;
	}

	struct uinput_setup usetup;

	// TODO(Barach): Docs
	int descriptor = open("/dev/uinput", O_WRONLY | O_NONBLOCK);
	if (descriptor < 0)
		return errorPrintf ("Failed to open /dev/uinput");

	// TODO(Barach): Docs?
	if (ioctl (descriptor, UI_SET_EVBIT, EV_KEY) == -1)
		return errorPrintf ("ioctl");

	char* deviceName = argv [argc - 3];
	canDevice_t* device = canInit (deviceName);
	if (device == NULL)
		return errorPrintf ("Failed to initialize CAN device '%s'", deviceName);

	char* dbcPath = argv [argc - 2];
	canDatabase_t database;
	if (canDatabaseInit (&database, device, dbcPath) != 0)
		return errorPrintf ("Failed to initialize CAN database");

	size_t signalCount;
	keySignal_t* signals = keySignalsLoad (argv [argc - 1], descriptor, &database, &signalCount);
	if (signals == NULL)
		return errorPrintf ("Failed to load key signals");

	memset(&usetup, 0, sizeof(usetup));
	usetup.id.bustype = BUS_USB;
	usetup.id.vendor = 0x1234; /* sample vendor */
	usetup.id.product = 0x5678; /* sample product */
	strcpy(usetup.name, "Example device");

	// TODO(Barach): Docs
	if (ioctl (descriptor, UI_DEV_SETUP, &usetup) == -1)
		return errorPrintf ("ioctl");

	// TODO(Barach): Docs
	if (ioctl (descriptor, UI_DEV_CREATE) == -1)
		return errorPrintf ("ioctl");

	while (1)
	{
		for (size_t index = 0; index < signalCount; ++index)
			keySignalUpdate (&signals [index]);

		struct timespec period = { .tv_sec = 0, .tv_nsec = 10000000 };
		nanosleep (&period, NULL);
	}

	ioctl (descriptor, UI_DEV_DESTROY);
	close (descriptor);

	return 0;
}