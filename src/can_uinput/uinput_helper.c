// Header
#include "uinput_helper.h"

// Includes
#include "debug.h"

// Linux
#include <linux/uinput.h>

// POSIX
#include <fcntl.h>
#include <sys/ioctl.h>
#include <unistd.h>

// C Standard Library
#include <errno.h>
#include <stddef.h>
#include <string.h>

#define codeAlias(macro)		\
{								\
	.alias	= #macro,			\
	.code	= macro				\
}

const uinputCodeAlias_t KEY_ALIASES [] =
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

const size_t KEY_ALIAS_COUNT = sizeof (KEY_ALIASES) / sizeof (uinputCodeAlias_t);

const uinputCodeAlias_t ABS_ALIASES [] =
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

const size_t ABS_ALIAS_COUNT = sizeof (ABS_ALIASES) / sizeof (uinputCodeAlias_t);

int uinputInit (bool key, bool abs)
{
	// Open the uintput file
	int fd = open ("/dev/uinput", O_WRONLY | O_NONBLOCK);
	if (fd < 0)
	{
		debugPrintf ("Failed to open /dev/uinput.\n");
		return -1;
	}

	// Enable key inputs
	if (key && ioctl (fd, UI_SET_EVBIT, EV_KEY) < 0)
	{
		debugPrintf ("ioctl UI_SET_EVBIT failed.\n");
		return -1;
	}

	// Enable abs inputs
	if (abs)
	{
		if (ioctl (fd, UI_SET_EVBIT, EV_ABS) < 0)
		{
			debugPrintf ("ioctl UI_SET_EVBIT failed.\n");
			return -1;
		}

		// Enable the required absolute axes. Not sure why these are required, however the device will not enumerate as a
		// controller otherwise.
		const int REQUIRED_ABS [] =
		{
			ABS_X, ABS_Y, ABS_RX, ABS_RY,
			ABS_Z, ABS_RZ, ABS_HAT0X, ABS_HAT0Y
		};
		for (size_t index = 0; index < sizeof (REQUIRED_ABS) / sizeof (int); ++index)
		{
			if (ioctl (fd, UI_SET_ABSBIT, REQUIRED_ABS [index]) < 0)
			{
				debugPrintf ("ioctl UI_SET_ABSBIT failed.\n");
				return -1;
			}
		}
	}

	return fd;
}

int uinputSetup (int fd, uint16_t vendor, uint16_t product, char* name)
{
	struct uinput_setup setup =
	{
		.id =
		{
			.bustype	= BUS_USB,
			.vendor		= vendor,
			.product	= product
		}
	};
	strncpy (setup.name, name, sizeof (setup.name) - 1);
	setup.name [sizeof (setup.name) - 1] = '\0';

	if (ioctl (fd, UI_DEV_SETUP, &setup) < 0)
	{
		debugPrintf ("ioctl UI_DEV_SETUP failed.\n");
		return errno;
	}

	if (ioctl (fd, UI_DEV_CREATE) < 0)
	{
		debugPrintf ("ioctl UI_DEV_CREATE failed.\n");
		return errno;
	}

	return 0;
}

int uinputEmit (int fd, int type, int code, int value)
{
	struct input_event event =
	{
		.type	= type,
		.code	= code,
		.value	= value
	};
	gettimeofday (&event.time, NULL);

	if (write (fd, &event, sizeof(event)) < (ssize_t) sizeof (event))
	{
		debugPrintf ("Failed to emit input event.\n");
		return errno;
	}

	return 0;
}

int uinputSync (int fd)
{
	return uinputEmit (fd, EV_SYN, SYN_REPORT, 0);
}

int uinputKeyAlias (const char* alias)
{
	for (size_t index = 0; index < KEY_ALIAS_COUNT; ++index)
		if (strcmp (alias, KEY_ALIASES [index].alias) == 0)
			return KEY_ALIASES [index].code;

	debugPrintf ("Unknown key alias '%s'.\n", alias);
	errno = EINVAL;
	return -1;
}

int uinputAbsAlias (const char* alias)
{
	for (size_t index = 0; index < ABS_ALIAS_COUNT; ++index)
		if (strcmp (alias, ABS_ALIASES [index].alias) == 0)
			return ABS_ALIASES [index].code;

	debugPrintf ("Unknown absolute axis alias '%s'.\n", alias);
	errno = EINVAL;
	return -1;
}

void uinputClose (int fd)
{
	ioctl (fd, UI_DEV_DESTROY);
	close (fd);
}