#include <fcntl.h>
#include <sys/ioctl.h>
#include <linux/uinput.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <signal.h>
#include <math.h>

#include "debug.h"
#include "can_device/can_device.h"
#include "can_database/can_database.h"
#include "cjson/cjson_util.h"

#define codeAlias(macro)		\
{								\
	.alias	= #macro,			\
	.code	= macro				\
}

typedef struct
{
	const char* alias;
	int code;
} codeAlias_t;
const codeAlias_t KEY_ALIASES [] =
{
	codeAlias (KEY_ESC),
	codeAlias (KEY_1),
	codeAlias (KEY_2),
	codeAlias (KEY_3),
	codeAlias (KEY_4),
	codeAlias (KEY_5),
	codeAlias (KEY_6),
	codeAlias (KEY_7),
	codeAlias (KEY_8),
	codeAlias (KEY_9),
	codeAlias (KEY_0),
	codeAlias (KEY_MINUS),
	codeAlias (KEY_EQUAL),
	codeAlias (KEY_BACKSPACE),
	codeAlias (KEY_TAB),
	codeAlias (KEY_Q),
	codeAlias (KEY_W),
	codeAlias (KEY_E),
	codeAlias (KEY_R),
	codeAlias (KEY_T),
	codeAlias (KEY_Y),
	codeAlias (KEY_U),
	codeAlias (KEY_I),
	codeAlias (KEY_O),
	codeAlias (KEY_P),
	codeAlias (KEY_LEFTBRACE),
	codeAlias (KEY_RIGHTBRACE),
	codeAlias (KEY_ENTER),
	codeAlias (KEY_LEFTCTRL),
	codeAlias (KEY_A),
	codeAlias (KEY_S),
	codeAlias (KEY_D),
	codeAlias (KEY_F),
	codeAlias (KEY_G),
	codeAlias (KEY_H),
	codeAlias (KEY_J),
	codeAlias (KEY_K),
	codeAlias (KEY_L),
	codeAlias (KEY_SEMICOLON),
	codeAlias (KEY_APOSTROPHE),
	codeAlias (KEY_GRAVE),
	codeAlias (KEY_LEFTSHIFT),
	codeAlias (KEY_BACKSLASH),
	codeAlias (KEY_Z),
	codeAlias (KEY_X),
	codeAlias (KEY_C),
	codeAlias (KEY_V),
	codeAlias (KEY_B),
	codeAlias (KEY_N),
	codeAlias (KEY_M),
	codeAlias (KEY_COMMA),
	codeAlias (KEY_DOT),
	codeAlias (KEY_SLASH),
	codeAlias (KEY_RIGHTSHIFT),
	codeAlias (KEY_KPASTERISK),
	codeAlias (KEY_LEFTALT),
	codeAlias (KEY_SPACE),
	codeAlias (KEY_CAPSLOCK),
	codeAlias (KEY_F1),
	codeAlias (KEY_F2),
	codeAlias (KEY_F3),
	codeAlias (KEY_F4),
	codeAlias (KEY_F5),
	codeAlias (KEY_F6),
	codeAlias (KEY_F7),
	codeAlias (KEY_F8),
	codeAlias (KEY_F9),
	codeAlias (KEY_F10),
	codeAlias (KEY_NUMLOCK),
	codeAlias (KEY_SCROLLLOCK),
	codeAlias (KEY_KP7),
	codeAlias (KEY_KP8),
	codeAlias (KEY_KP9),
	codeAlias (KEY_KPMINUS),
	codeAlias (KEY_KP4),
	codeAlias (KEY_KP5),
	codeAlias (KEY_KP6),
	codeAlias (KEY_KPPLUS),
	codeAlias (KEY_KP1),
	codeAlias (KEY_KP2),
	codeAlias (KEY_KP3),
	codeAlias (KEY_KP0),
	codeAlias (KEY_KPDOT),

	codeAlias (KEY_F11),
	codeAlias (KEY_F12),

	codeAlias (KEY_RIGHTCTRL),
	codeAlias (KEY_RIGHTALT),
	codeAlias (KEY_LINEFEED),
	codeAlias (KEY_HOME),
	codeAlias (KEY_UP),
	codeAlias (KEY_PAGEUP),
	codeAlias (KEY_LEFT),
	codeAlias (KEY_RIGHT),
	codeAlias (KEY_END),
	codeAlias (KEY_DOWN),
	codeAlias (KEY_PAGEDOWN),
	codeAlias (KEY_INSERT),
	codeAlias (KEY_DELETE),
	codeAlias (KEY_MACRO),
	codeAlias (KEY_MUTE),
	codeAlias (KEY_VOLUMEDOWN),
	codeAlias (KEY_VOLUMEUP),
	codeAlias (KEY_POWER),
	codeAlias (KEY_PAUSE),
	codeAlias (KEY_SCALE),

	codeAlias (KEY_LEFTMETA),
	codeAlias (KEY_RIGHTMETA),

	codeAlias (BTN_MISC),
	codeAlias (BTN_0),
	codeAlias (BTN_1),
	codeAlias (BTN_2),
	codeAlias (BTN_3),
	codeAlias (BTN_4),
	codeAlias (BTN_5),
	codeAlias (BTN_6),
	codeAlias (BTN_7),
	codeAlias (BTN_8),
	codeAlias (BTN_9),

	codeAlias (BTN_MOUSE),
	codeAlias (BTN_LEFT),
	codeAlias (BTN_RIGHT),
	codeAlias (BTN_MIDDLE),
	codeAlias (BTN_SIDE),
	codeAlias (BTN_EXTRA),
	codeAlias (BTN_FORWARD),
	codeAlias (BTN_BACK),
	codeAlias (BTN_TASK),

	codeAlias (BTN_JOYSTICK),
	codeAlias (BTN_TRIGGER),
	codeAlias (BTN_THUMB),
	codeAlias (BTN_THUMB2),
	codeAlias (BTN_TOP),
	codeAlias (BTN_TOP2),
	codeAlias (BTN_PINKIE),
	codeAlias (BTN_BASE),
	codeAlias (BTN_BASE2),
	codeAlias (BTN_BASE3),
	codeAlias (BTN_BASE4),
	codeAlias (BTN_BASE5),
	codeAlias (BTN_BASE6),
	codeAlias (BTN_DEAD),

	codeAlias (BTN_GAMEPAD),
	codeAlias (BTN_SOUTH),
	codeAlias (BTN_A),
	codeAlias (BTN_EAST),
	codeAlias (BTN_B),
	codeAlias (BTN_C),
	codeAlias (BTN_NORTH),
	codeAlias (BTN_X),
	codeAlias (BTN_WEST),
	codeAlias (BTN_Y),
	codeAlias (BTN_Z),
	codeAlias (BTN_TL),
	codeAlias (BTN_TR),
	codeAlias (BTN_TL2),
	codeAlias (BTN_TR2),
	codeAlias (BTN_SELECT),
	codeAlias (BTN_START),
	codeAlias (BTN_MODE),
	codeAlias (BTN_THUMBL),
	codeAlias (BTN_THUMBR),

	codeAlias (BTN_DIGI),
	codeAlias (BTN_TOOL_PEN),
	codeAlias (BTN_TOOL_RUBBER),
	codeAlias (BTN_TOOL_BRUSH),
	codeAlias (BTN_TOOL_PENCIL),
	codeAlias (BTN_TOOL_AIRBRUSH),
	codeAlias (BTN_TOOL_FINGER),
	codeAlias (BTN_TOOL_MOUSE),
	codeAlias (BTN_TOOL_LENS),
	codeAlias (BTN_TOOL_QUINTTAP),
	codeAlias (BTN_STYLUS3),
	codeAlias (BTN_TOUCH),
	codeAlias (BTN_STYLUS),
	codeAlias (BTN_STYLUS2),
	codeAlias (BTN_TOOL_DOUBLETAP),
	codeAlias (BTN_TOOL_TRIPLETAP),
	codeAlias (BTN_TOOL_QUADTAP),

	codeAlias (BTN_WHEEL),
	codeAlias (BTN_GEAR_DOWN),
	codeAlias (BTN_GEAR_UP),

	codeAlias (KEY_OK),
	codeAlias (KEY_SELECT),
	codeAlias (KEY_GOTO),
	codeAlias (KEY_CLEAR),
	codeAlias (KEY_POWER2),
	codeAlias (KEY_OPTION),
	codeAlias (KEY_INFO),
	codeAlias (KEY_TIME),
	codeAlias (KEY_VENDOR),
	codeAlias (KEY_ARCHIVE),
	codeAlias (KEY_PROGRAM),
	codeAlias (KEY_CHANNEL),
	codeAlias (KEY_FAVORITES),
	codeAlias (KEY_EPG),
	codeAlias (KEY_PVR),
	codeAlias (KEY_MHP),
	codeAlias (KEY_LANGUAGE),
	codeAlias (KEY_TITLE),
	codeAlias (KEY_SUBTITLE),
	codeAlias (KEY_ANGLE),
	codeAlias (KEY_FULL_SCREEN),
	codeAlias (KEY_ZOOM),
	codeAlias (KEY_MODE),
	codeAlias (KEY_KEYBOARD),
	codeAlias (KEY_ASPECT_RATIO),
	codeAlias (KEY_SCREEN),
	codeAlias (KEY_PC),
	codeAlias (KEY_TV),
	codeAlias (KEY_TV2),
	codeAlias (KEY_VCR),
	codeAlias (KEY_VCR2),
	codeAlias (KEY_SAT),
	codeAlias (KEY_SAT2),
	codeAlias (KEY_CD),
	codeAlias (KEY_TAPE),
	codeAlias (KEY_RADIO),
	codeAlias (KEY_TUNER),
	codeAlias (KEY_PLAYER),
	codeAlias (KEY_TEXT),
	codeAlias (KEY_DVD),
	codeAlias (KEY_AUX),
	codeAlias (KEY_MP3),
	codeAlias (KEY_AUDIO),
	codeAlias (KEY_VIDEO),
	codeAlias (KEY_DIRECTORY),
	codeAlias (KEY_LIST),
	codeAlias (KEY_MEMO),
	codeAlias (KEY_CALENDAR),
	codeAlias (KEY_RED),
	codeAlias (KEY_GREEN),
	codeAlias (KEY_YELLOW),
	codeAlias (KEY_BLUE),
	codeAlias (KEY_CHANNELUP),
	codeAlias (KEY_CHANNELDOWN),
	codeAlias (KEY_FIRST),
	codeAlias (KEY_LAST),
	codeAlias (KEY_AB),
	codeAlias (KEY_NEXT),
	codeAlias (KEY_RESTART),
	codeAlias (KEY_SLOW),
	codeAlias (KEY_SHUFFLE),
	codeAlias (KEY_BREAK),
	codeAlias (KEY_PREVIOUS),
	codeAlias (KEY_DIGITS),
	codeAlias (KEY_TEEN),
	codeAlias (KEY_TWEN),
	codeAlias (KEY_VIDEOPHONE),
	codeAlias (KEY_GAMES),
	codeAlias (KEY_ZOOMIN),
	codeAlias (KEY_ZOOMOUT),
	codeAlias (KEY_ZOOMRESET),
	codeAlias (KEY_WORDPROCESSOR),
	codeAlias (KEY_EDITOR),
	codeAlias (KEY_SPREADSHEET),
	codeAlias (KEY_GRAPHICSEDITOR),
	codeAlias (KEY_PRESENTATION),
	codeAlias (KEY_DATABASE),
	codeAlias (KEY_NEWS),
	codeAlias (KEY_VOICEMAIL),
	codeAlias (KEY_ADDRESSBOOK),
	codeAlias (KEY_MESSENGER),
	codeAlias (KEY_DISPLAYTOGGLE),
	codeAlias (KEY_BRIGHTNESS_TOGGLE),
	codeAlias (KEY_SPELLCHECK),
	codeAlias (KEY_LOGOFF),

	codeAlias (KEY_FN),
	codeAlias (KEY_FN_ESC),
	codeAlias (KEY_FN_F1),
	codeAlias (KEY_FN_F2),
	codeAlias (KEY_FN_F3),
	codeAlias (KEY_FN_F4),
	codeAlias (KEY_FN_F5),
	codeAlias (KEY_FN_F6),
	codeAlias (KEY_FN_F7),
	codeAlias (KEY_FN_F8),
	codeAlias (KEY_FN_F9),
	codeAlias (KEY_FN_F10),
	codeAlias (KEY_FN_F11),
	codeAlias (KEY_FN_F12),
	codeAlias (KEY_FN_1),
	codeAlias (KEY_FN_2),
	codeAlias (KEY_FN_D),
	codeAlias (KEY_FN_E),
	codeAlias (KEY_FN_F),
	codeAlias (KEY_FN_S),
	codeAlias (KEY_FN_B),
	codeAlias (KEY_FN_RIGHT_SHIFT),

	codeAlias (KEY_RIGHT_UP),
	codeAlias (KEY_RIGHT_DOWN),
	codeAlias (KEY_LEFT_UP),
	codeAlias (KEY_LEFT_DOWN),
};
const size_t KEY_ALIAS_COUNT = sizeof (KEY_ALIASES) / sizeof (codeAlias_t);

const codeAlias_t ABS_ALIASES [] =
{
	codeAlias (ABS_X),
	codeAlias (ABS_Y),
	codeAlias (ABS_Z),
	codeAlias (ABS_RX),
	codeAlias (ABS_RY),
	codeAlias (ABS_RZ),
	codeAlias (ABS_HAT0X),
	codeAlias (ABS_HAT0Y)
};
const size_t ABS_ALIAS_COUNT = sizeof (ABS_ALIASES) / sizeof (codeAlias_t);

const int REQUIRED_ABS [] =
{
	ABS_X, ABS_Y, ABS_RX, ABS_RY,
	ABS_Z, ABS_RZ, ABS_HAT0X, ABS_HAT0Y
};
const size_t REQUIRED_ABS_COUNT = sizeof (REQUIRED_ABS) / sizeof (int);

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

int getKeyCode (const char* alias)
{
	for (size_t index = 0; index < KEY_ALIAS_COUNT; ++index)
		if (strcmp (alias, KEY_ALIASES [index].alias) == 0)
			return KEY_ALIASES [index].code;

	fprintf (stderr, "Unknown key alias '%s'.\n", alias);
	return -1;
}

int getAbsCode (const char* alias)
{
	for (size_t index = 0; index < ABS_ALIAS_COUNT; ++index)
		if (strcmp (alias, ABS_ALIASES [index].alias) == 0)
			return ABS_ALIASES [index].code;

	fprintf (stderr, "Unknown abs alias '%s'.\n", alias);
	return -1;
}

void inputEmit (int fd, int type, int code, int value)
{
	struct input_event event =
	{
		.type	= type,
		.code	= code,
		.value	= value
	};
	gettimeofday (&event.time, NULL);

	if (write (fd, &event, sizeof(event)) < (ssize_t) sizeof (event))
		errorPrintf ("Failed to emit input");
}

void inputSync (int fd)
{
	inputEmit (fd, EV_SYN, SYN_REPORT, 0);
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

keySignal_t* keySignalsLoad (cJSON* config, int descriptor, canDatabase_t* database, size_t* count)
{
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

		config.keyboardKey = getKeyCode (keyboardKey);
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

	if (canDatabaseGetFloat (keySignal->config.database, keySignal->index, &value) == CAN_DATABASE_VALID)
	{
		if ((value >= keySignal->config.threshold) != keySignal->config.inverted && !keySignal->pressed)
		{
			keySignal->pressed = true;
			inputEmit (keySignal->config.descriptor, EV_KEY, keySignal->config.keyboardKey, 1);
		}

		if ((value < keySignal->config.threshold) != keySignal->config.inverted && keySignal->pressed)
		{
			keySignal->pressed = false;
			inputEmit (keySignal->config.descriptor, EV_KEY, keySignal->config.keyboardKey, 0);
		}
	}
	else if (keySignal->pressed)
	{
		keySignal->pressed = false;
		inputEmit (keySignal->config.descriptor, EV_KEY, keySignal->config.keyboardKey, 0);
	}

	return 0;
}

typedef struct
{
	const char* positiveSignalName;
	const char* negativeSignalName;
	int code;
	float positiveZero;
	float positiveMax;
	float negativeZero;
	float negativeMax;
} absSignalConfig_t;

typedef struct
{
	absSignalConfig_t config;
	int fd;
	canDatabase_t* database;
	ssize_t positiveSignal;
	ssize_t negativeSignal;
} absSignal_t;

int absSignalInit (absSignal_t* abs, absSignalConfig_t* config, int fd, canDatabase_t* database)
{
	abs->config = *config;
	abs->fd = fd;
	abs->database = database;

	abs->positiveSignal = canDatabaseFindSignal (database, config->positiveSignalName);
	if (abs->positiveSignal < 0)
		return errno;

	if (config->negativeSignalName != NULL)
	{
		abs->negativeSignal = canDatabaseFindSignal (database, config->negativeSignalName);
		if (abs->negativeSignal < 0)
			return errno;
	}
	else
		abs->negativeSignal = -1;

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

	if (ioctl (fd, UI_SET_ABSBIT, config->code) < 0)
		return errno;

	if (ioctl (fd, UI_ABS_SETUP, &setup) < 0)
		return errno;

	return 0;
}

absSignal_t* absSignalsLoad (cJSON* config, int fd, canDatabase_t* database, size_t* count)
{
	cJSON* absArray;
	if (jsonGetObject (config, "abs", &absArray) != 0)
		return NULL;

	*count = cJSON_GetArraySize (absArray);
	absSignal_t* abs = malloc (sizeof (absSignal_t) * *count);
	if (abs == NULL)
		return NULL;

	for (size_t index = 0; index < *count; ++index)
	{
		absSignalConfig_t config = {};

		cJSON* absConfig = cJSON_GetArrayItem (absArray, index);
		if (absConfig == NULL)
		{
			errno = ERRNO_CJSON_PARSE_FAIL;
			return NULL;
		}

		char* positiveSignalName;
		if (jsonGetString (absConfig, "positiveSignalName", &positiveSignalName) != 0)
			return NULL;
		config.positiveSignalName = positiveSignalName;

		if (jsonGetFloat (absConfig, "positiveZero", &config.positiveZero) != 0)
			return NULL;

		if (jsonGetFloat (absConfig, "positiveMax", &config.positiveMax) != 0)
			return NULL;

		char* negativeSignalName;
		if (jsonGetString (absConfig, "negativeSignalName", &negativeSignalName) == 0)
		{
			config.negativeSignalName = negativeSignalName;

			if (jsonGetFloat (absConfig, "negativeZero", &config.negativeZero) != 0)
				return NULL;

			if (jsonGetFloat (absConfig, "negativeMax", &config.negativeMax) != 0)
				return NULL;
		}
		else
			config.negativeSignalName = NULL;

		char* codeString;
		if (jsonGetString (absConfig, "code", &codeString) != 0)
			return NULL;

		config.code = getAbsCode (codeString);
		if (config.code < 0)
			return NULL;

		debugPrintf (
			"Loaded abs:\n"
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

void absSignalUpdate (absSignal_t* abs)
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

	inputEmit (abs->fd, EV_ABS, abs->config.code, positiveInt + negativeInt);
}

bool running = true;

void sigtermHandler (int sig)
{
	(void) sig;

	printf ("Terminating...\n");
	running = false;
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

	// TODO(Barach): Docs
	int descriptor = open("/dev/uinput", O_WRONLY | O_NONBLOCK);
	if (descriptor < 0)
		return errorPrintf ("Failed to open /dev/uinput");

	// TODO(Barach): Docs?
	if (ioctl (descriptor, UI_SET_EVBIT, EV_KEY) == -1)
		return errorPrintf ("UI_SET_EVBIT");

	if (ioctl (descriptor, UI_SET_EVBIT, EV_ABS) == -1)
		return errorPrintf ("UI_SET_EVBIT");

	char* deviceName = argv [argc - 3];
	canDevice_t* device = canInit (deviceName);
	if (device == NULL)
		return errorPrintf ("Failed to initialize CAN device '%s'", deviceName);

	char* dbcPath = argv [argc - 2];
	canDatabase_t database;
	if (canDatabaseInit (&database, device, dbcPath) != 0)
		return errorPrintf ("Failed to initialize CAN database");

	for (size_t index = 0; index < REQUIRED_ABS_COUNT; ++index)
	{
		ioctl(descriptor, UI_SET_ABSBIT, REQUIRED_ABS [index]);
		struct uinput_abs_setup abs_setup = {};
		memset (&abs_setup, 0, sizeof(abs_setup));
		abs_setup.code = REQUIRED_ABS [index];
		abs_setup.absinfo.minimum = 0;
		abs_setup.absinfo.maximum = 255;
		ioctl (descriptor, UI_ABS_SETUP, &abs_setup);
	}

	cJSON* config = jsonLoad (argv [argc - 1]);
	if (config == NULL)
		return -1;

	size_t signalCount;
	keySignal_t* signals = keySignalsLoad (config, descriptor, &database, &signalCount);
	if (signals == NULL)
		return errorPrintf ("Failed to load key signals");

	size_t absCount;
	absSignal_t* abs = absSignalsLoad (config, descriptor, &database, &absCount);
	if (abs == NULL)
		return -1;

	// TODO(Barach): Docs
	struct uinput_setup usetup;
	memset(&usetup, 0, sizeof(usetup));
	usetup.id.bustype = BUS_USB;
	usetup.id.vendor = 0x054C; /* sample vendor */
	usetup.id.product = 0x0CE6; /* sample product */
	strcpy(usetup.name, "DualSense Wireless Controller");
	if (ioctl (descriptor, UI_DEV_SETUP, &usetup) == -1)
		return errorPrintf ("UI_DEV_SETUP");

	// TODO(Barach): Docs
	if (ioctl (descriptor, UI_DEV_CREATE) == -1)
		return errorPrintf ("UI_DEV_CREATE");

	if (signal (SIGTERM, sigtermHandler) == SIG_ERR)
		return errorPrintf ("Failed to bind SIGTERM handler");

	if (signal (SIGINT, sigtermHandler) == SIG_ERR)
		return errorPrintf ("Failed to bind SIGINT handler");

	while (running)
	{
		for (size_t index = 0; index < signalCount; ++index)
			keySignalUpdate (&signals [index]);

		for (size_t index = 0; index < absCount; ++index)
			absSignalUpdate (&abs [index]);

		inputSync (descriptor);

		usleep (10000);
	}

	ioctl (descriptor, UI_DEV_DESTROY);
	close (descriptor);

	return 0;
}