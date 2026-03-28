// Header
#include "can_label_float.h"

// Includes
#include "can_database/can_database_stdio.h"
#include "cjson/cjson_util.h"

typedef struct
{
	// Every CAN widget must start with the VMT.
	canWidgetVmt_t vmt;

	// Configuration
	canLabelFloatConfig_t config;

	// CAN database reference.
	canDatabase_t* database;

	// CAN database signal index.
	ssize_t index;
} canLabelFloat_t;

static void update (void* widget)
{
	canLabelFloat_t* label = widget;

	char text [512] = "";
	snprintCanDatabaseFloat (text, sizeof (text), label->config.formatValue, label->config.formatInvalid,
		label->database, label->index);
	gtk_label_set_text (GTK_LABEL (label->vmt.widget), text);
}

canWidget_t* canLabelFloatInit (canDatabase_t* database, canLabelFloatConfig_t* config)
{
	// Allocate the object
	canLabelFloat_t* label = malloc (sizeof (canLabelFloat_t));
	if (label == NULL)
		return NULL;

	// Init the object
	*label = (canLabelFloat_t)
	{
		.vmt =
		{
			.update = update,
			.widget	= gtk_label_new ("")
		},
		.config		= *config,
		.database	= database,
		.index		= canDatabaseFindSignal (database, config->signalName)
	};

	// Validate the signal exists
	if (label->index < 0)
		return NULL;

	// Update initial value
	update (label);

	// Cast into the base type
	return (canWidget_t*) label;
}

canWidget_t* canLabelFloatLoad (canDatabase_t* database, cJSON* config, canLabelFloatStyle_t* parentStyle)
{
	canLabelFloatConfig_t widgetConfig;

	// Load config fields. Exit early is required field is not specified.

	if (jsonGetString (config, "signalName", &widgetConfig.signalName) != 0)
		return NULL;

	if (jsonGetString (config, "formatValue", &widgetConfig.formatValue) != 0)
		return NULL;

	if (jsonGetString (config, "formatInvalid", &widgetConfig.formatInvalid) != 0)
		return NULL;

	// No style to load right now.
	(void) parentStyle;

	return canLabelFloatInit (database, &widgetConfig);
}

void canLabelFloatLoadStyle (cJSON* config, canLabelFloatStyle_t* style, canLabelFloatStyle_t* parent)
{
	(void) config;
	(void) style;
	(void) parent;

	// Nothing to load right now.

	return;
}