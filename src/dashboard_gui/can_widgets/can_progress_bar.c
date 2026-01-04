// Header
#include "can_progress_bar.h"

// Includes
#include "zre_math.h"

typedef struct
{
	canWidgetVmt_t vmt;
	canProgressBarConfig_t config;
	stylizedProgressBar_t* bar;
	canDatabase_t* database;
	ssize_t index;
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
		.bar		= stylizedProgressBarInit (&config->barConfig),
		.database	= database,
		.index		= canDatabaseFindSignal (database, config->signalName)
	};

	// Store a reference to the base widget
	bar->vmt.widget = STYLIZED_PROGRESS_BAR_TO_WIDGET (bar->bar);

	// Update initial value
	update (bar);

	// Cast into the base type
	return (canWidget_t*) bar;
}