// Header
#include "can_label_fault.h"

// Includes
#include "cjson/cjson_util.h"

// Base datatype for the widget.
typedef struct
{
	// Every CAN widget must start with the VMT.
	canWidgetVmt_t vmt;

	// Configuration
	canLabelFaultConfig_t config;

	// CAN database reference.
	canDatabase_t* database;
} canLabelFault_t;

static void update (void* widget)
{
	canLabelFault_t* label = widget;

	char* status = faultSignalsGetString (&label->config.faults);
	char buffer [64];
	snprintf (buffer, 64, label->config.format, status);

	gtk_label_set_text (GTK_LABEL (CAN_WIDGET_TO_WIDGET (label)), buffer);
}

canWidget_t* canLabelFaultInit (canDatabase_t* database, canLabelFaultConfig_t* config)
{
	// Allocate the object
	canLabelFault_t* label = malloc (sizeof (canLabelFault_t));
	if (label == NULL)
		return NULL;

	// Init the object
	*label = (canLabelFault_t)
	{
		// Init the VMT
		.vmt =
		{
			.update = update,
			.widget	= gtk_label_new ("")
		},
		.config		= *config,
		.database	= database
	};

	// Update initial value
	update (label);

	// Cast into the base type
	return (canWidget_t*) label;
}

canWidget_t* canLabelFaultLoad (canDatabase_t* database, cJSON* config)
{
	canLabelFaultConfig_t widgetConfig;

	// Get the path to the node's config file
	char* nodeConfigPath;
	if (jsonGetString (config, "configPath", &nodeConfigPath) != 0)
		return NULL;

	// Load the node's config file
	cJSON* nodeConfig = jsonLoadPath (nodeConfigPath);
	if (nodeConfig == NULL)
		return NULL;

	// Load the fault signals
	if (faultSignalsLoad (&widgetConfig.faults, nodeConfig, database) != 0)
		return NULL;

	// Get the format string
	if (jsonGetString (config, "format", &widgetConfig.format) != 0)
		return NULL;

	return canLabelFaultInit (database, &widgetConfig);
}