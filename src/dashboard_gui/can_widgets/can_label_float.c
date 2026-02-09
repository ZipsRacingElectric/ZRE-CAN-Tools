// Header
#include "can_label_float.h"

// Includes
#include "can_database/can_database_stdio.h"
#include "cjson/cjson_util.h"

typedef struct
{
	canWidgetVmt_t vmt;
	canLabelFloatConfig_t config;
	canDatabase_t* database;
	ssize_t index;
} canLabelFloat_t;

static void update (void* widget)
{
	canLabelFloat_t* label = widget;

	char text [64] = "";
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

	// Update initial value
	update (label);

	// Cast into the base type
	return (canWidget_t*) label;
}

canWidget_t* canLabelFloatLoad (canDatabase_t* database, cJSON* config)
{
	char* signalName;
	if (jsonGetString (config, "signalName", &signalName) != 0)
		return NULL;

	char* formatValue;
	if (jsonGetString (config, "formatValue", &formatValue) != 0)
		return NULL;

	char* formatInvalid;
	if (jsonGetString (config, "formatInvalid", &formatInvalid) != 0)
		return NULL;

	return canLabelFloatInit (database, &(canLabelFloatConfig_t)
	{
		.signalName		= signalName,
		.formatValue	= formatValue,
		.formatInvalid	= formatInvalid
	});
}