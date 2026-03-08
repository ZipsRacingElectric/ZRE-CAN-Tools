// Header
#include "can_progress_bar.h"

// Includes
#include "../gtk_util.h"
#include "cjson/cjson_util.h"
#include "debug.h"
#include "zre_math.h"

typedef struct
{
	// Every CAN widget must start with the VMT.
	canWidgetVmt_t vmt;

	// Configuration
	canProgressBarConfig_t config;

	// CAN database reference.
	canDatabase_t* database;

	// CAN database signal index.
	ssize_t index;

	// Progress bar widget.
	stylizedProgressBar_t* bar;
} canProgressBar_t;

static void update (void* widget)
{
	canProgressBar_t* bar = widget;

	// Get the signal value, default to min if invalid
	float value;
	if (canDatabaseGetFloat (bar->database, bar->index, &value) != CAN_DATABASE_VALID)
		value = bar->config.min;

	// Map signal value to progess bar fill
	value = inverseLerp (value, bar->config.min, bar->config.max);

	// Set fill
	stylizedProgressBarSetValue (bar->bar, value);
}

canWidget_t* canProgressBarInit (canDatabase_t* database, canProgressBarConfig_t* config)
{
	// Allocate the object
	canProgressBar_t* bar = malloc (sizeof (canProgressBar_t));
	if (bar == NULL)
		return NULL;

	// Init the object
	*bar = (canProgressBar_t)
	{
		.vmt =
		{
			.update = update,
			.widget	= NULL
		},
		.config		= *config,
		.database	= database,
		.index		= canDatabaseFindSignal (database, config->signalName),
		.bar		= stylizedProgressBarInit (&config->style.barConfig)
	};

	// Store a reference to the base widget
	bar->vmt.widget = STYLIZED_PROGRESS_BAR_TO_WIDGET (bar->bar);

	// Update initial value
	update (bar);

	// Cast into the base type
	return (canWidget_t*) bar;
}

canWidget_t* canProgressBarLoad (canDatabase_t* database, cJSON* config, canProgressBarStyle_t* parentStyle)
{
	canProgressBarConfig_t widgetConfig;

	// Load config fields. Exit early is required field is not specified.

	if (jsonGetString (config, "signalName", &widgetConfig.signalName) != 0)
		return NULL;

	if (jsonGetFloat (config, "min", &widgetConfig.min) != 0)
		return NULL;

	if (jsonGetFloat (config, "max", &widgetConfig.max) != 0)
		return NULL;

	cJSON* styleConfig = jsonGetObjectV2 (config, "style");
	canProgressBarLoadStyle (styleConfig, &widgetConfig.style, parentStyle);

	return canProgressBarInit (database, &widgetConfig);
}

void canProgressBarLoadStyle (cJSON* config, canProgressBarStyle_t* style, canProgressBarStyle_t* parent)
{
	if (parent != NULL)
		// If a parent is specified, inherit everything.
		*style = *parent;
	else
	{
		// If no parent is specified, use default values.
		*style = (canProgressBarStyle_t)
		{
			.barConfig =
			{
				.width				= 100,
				.height				= 10,
				.borderThickness	= 1.5f,
				.orientation		= GTK_ORIENTATION_HORIZONTAL,
				.inverted			= false,
				.backgroundColor	= gdkHexToColor ("#000000"),
				.fillColor			= gdkHexToColor ("#FFFFFF"),
				.borderColor		= gdkHexToColor ("#FFFFFF")
			}
		};
	}

	// If no config was provided, use what we have.
	if (config == NULL)
		return;

	// Load style fields, if specified.

	jsonGetInt (config, "width", &style->barConfig.width);
	jsonGetInt (config, "height", &style->barConfig.height);
	jsonGetFloat (config, "borderThickness", &style->barConfig.borderThickness);

	char* orientationStr;
	if (jsonGetString (config, "orientation", &orientationStr) == 0)
	{
		if (strcmp (orientationStr, "horizontal") == 0)
			style->barConfig.orientation = GTK_ORIENTATION_HORIZONTAL;
		else if (strcmp (orientationStr, "vertical") == 0)
			style->barConfig.orientation = GTK_ORIENTATION_VERTICAL;
		else
			debugPrintf ("Invalid CAN progress bar orientation '%s'.\n", orientationStr);
	}

	jsonGetBool (config, "inverted", &style->barConfig.inverted);

	char* color;
	if (jsonGetString (config, "backgroundColor", &color) == 0)
		style->barConfig.backgroundColor = gdkHexToColor (color);
	if (jsonGetString (config, "fillColor", &color) == 0)
		style->barConfig.fillColor = gdkHexToColor (color);
	if (jsonGetString (config, "borderColor", &color) == 0)
		style->barConfig.borderColor = gdkHexToColor (color);
}