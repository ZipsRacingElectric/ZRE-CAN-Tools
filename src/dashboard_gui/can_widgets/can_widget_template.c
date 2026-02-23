// Header
#include "can_widget_template.h"

// Base datatype for the widget.
typedef struct
{
	// Every CAN widget must start with the VMT.
	canWidgetVmt_t vmt;

	// Configuration
	canWidgetTemplateConfig_t config;

	// CAN database reference.
	canDatabase_t* database;
} canWidgetTemplate_t;

static void update (void* widget)
{
	canWidgetTemplate_t* template = widget;

	// Perform update here...
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

			// If not using label as the base widget, can be changed here...
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

canWidget_t* canWidgetTemplateLoad (canDatabase_t* database, cJSON* config)
{
	// Load JSON fields here (ex jsonGetString)...
	(void) config;

	return canWidgetTemplateInit (database, &(canWidgetTemplateConfig_t)
	{
		// Populate fields here...
	});
}