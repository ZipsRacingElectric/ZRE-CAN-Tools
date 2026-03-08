// Header
#include "can_widget_template.h"

// Includes
#include "cjson/cjson_util.h"

// Base datatype for the widget.
typedef struct
{
	// Every CAN widget must start with the VMT.
	canWidgetVmt_t vmt;

	// Configuration
	canWidgetTemplateConfig_t config;

	// CAN database reference.
	canDatabase_t* database;

	// TODO: More fields here...
} canWidgetTemplate_t;

static void update (void* widget)
{
	canWidgetTemplate_t* template = widget;

	// TODO: Perform update here...
	(void) template;
}

canWidget_t* canWidgetTemplateInit (canDatabase_t* database, canWidgetTemplateConfig_t* config)
{
	// Allocate the object
	canWidgetTemplate_t* template = malloc (sizeof (canWidgetTemplate_t));
	if (template == NULL)
		return NULL;

	// Init the object
	*template = (canWidgetTemplate_t)
	{
		// Init the VMT
		.vmt =
		{
			.update = update,

			// TODO: If not using label as the base widget, can be changed here...
			.widget	= gtk_label_new ("")
		},
		.config		= *config,
		.database	= database
	};

	// Update initial value
	update (template);

	// Cast into the base type
	return (canWidget_t*) template;
}

canWidget_t* canWidgetTemplateLoad (canDatabase_t* database, cJSON* config, canWidgetTemplateStyle_t* parentStyle)
{
	canWidgetTemplateConfig_t widgetConfig;

	// Load config fields. Exit early is required field is not specified.

	// TODO: Load fields here (ex: jsonGetString)...

	cJSON* styleConfig = jsonGetObjectV2 (config, "style");
	canWidgetTemplateLoadStyle (styleConfig, &widgetConfig.style, parentStyle);

	return canWidgetTemplateInit (database, &widgetConfig);
}

void canWidgetTemplateLoadStyle (cJSON* config, canWidgetTemplateStyle_t* style, canWidgetTemplateStyle_t* parent)
{
	if (parent != NULL)
		// If a parent is specified, inherit everything.
		*style = *parent;
	else
	{
		// If no parent is specified, use default values.
		*style = (canWidgetTemplateStyle_t)
		{
			// Set default values here...
		};
	}

	// If no config was provided, use what we have.
	if (config == NULL)
		return;

	// Load style fields, if specified.

	// TODO: Load fields here (ex: jsonGetString)...
}