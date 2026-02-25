// Header
#include "can_float_popup.h"

// Includes
#include "../gtk_util.h"
#include "cjson/cjson_util.h"

// Base datatype for the widget.
typedef struct
{
	// Every CAN widget must start with the VMT.
	canWidgetVmt_t vmt;

	// Configuration
	canFloatPopupConfig_t config;

	// CAN database reference.
	canDatabase_t* database;
	ssize_t index;

	// Internal widgets
	stylizedFrame_t* frame;
	GtkLabel* label;

	// Last state
	bool active;
} canFloatPopup_t;

static void update (void* widget)
{
	canFloatPopup_t* popup = widget;

	// Check if the popup is active or not
	bool active;
	float value;
	if (canDatabaseGetFloat (popup->database, popup->index, &value) == CAN_DATABASE_VALID)
		active = (value >= popup->config.threshold) != popup->config.inverted;
	else
		active = false;

	// Ignore if nothing has changed
	if (active == popup->active)
		return;
	popup->active = active;

	// Enable / disable the popup based on whether active
	gtk_widget_set_visible (CAN_WIDGET_TO_WIDGET (popup), active);

	// Get and update the label's text
	if (active)
	{
		char buffer [512];
		snprintf (buffer, 512, popup->config.format, value);
		gtk_label_set_text (popup->label, buffer);
	}
}

canWidget_t* canFloatPopupInit (canDatabase_t* database, canFloatPopupConfig_t* config)
{
	// Allocate the object
	canFloatPopup_t* popup = malloc (sizeof (canFloatPopup_t));
	if (popup == NULL)
		return NULL;

	// Init the object
	*popup = (canFloatPopup_t)
	{
		// Init the VMT
		.vmt =
		{
			.update = update,
			// Set later
			.widget	= NULL
		},
		.config		= *config,
		.database	= database,
		.index		= canDatabaseFindSignal (database, config->signalName),
		.frame		= stylizedFrameInit (&config->frameConfig),
		.label		= GTK_LABEL (gtk_label_new ("")),
		.active		= false
	};

	// Set the base widget
	if (popup->frame == NULL)
		return NULL;
	popup->vmt.widget = STYLIZED_FRAME_TO_WIDGET (popup->frame);

	// Default to not active
	gtk_widget_set_visible (CAN_WIDGET_TO_WIDGET (popup), false);

	// Set the frame's child
	stylizedFrameSetChild (popup->frame, GTK_WIDGET (popup->label));

	// Set the font color
	gtkLabelSetColor (popup->label, &popup->config.frameConfig.borderColor);
	int margin = config->padding + config->frameConfig.borderThickness;
	gtk_widget_set_margin_bottom (GTK_WIDGET (popup->label), margin);
	gtk_widget_set_margin_top (GTK_WIDGET (popup->label), margin);
	gtk_widget_set_margin_start (GTK_WIDGET (popup->label), margin);
	gtk_widget_set_margin_end (GTK_WIDGET (popup->label), margin);

	// Update initial value
	update (popup);

	// Cast into the base type
	return (canWidget_t*) popup;
}

canWidget_t* canFloatPopupLoad (canDatabase_t* database, cJSON* config)
{
	canFloatPopupConfig_t widgetConfig;

	if (jsonGetString (config, "signalName", &widgetConfig.signalName) != 0)
		return NULL;

	// Get the format string
	if (jsonGetString (config, "format", &widgetConfig.format) != 0)
		return NULL;

	if (jsonGetString (config, "font", &widgetConfig.font) != 0)
		return NULL;

	uint16_t width = 0;
	if (jsonGetUint16_t (config, "width", &width) == 0)
		widgetConfig.width = width;

	uint16_t height = 0;
	if (jsonGetUint16_t (config, "height", &height) == 0)
		widgetConfig.height = height;

	uint16_t padding = 0;
	if (jsonGetUint16_t (config, "padding", &padding) == 0)
		widgetConfig.padding = padding;

	if (jsonGetFloat (config, "threshold", &widgetConfig.threshold) != 0)
		return NULL;

	if (jsonGetBool (config, "inverted", &widgetConfig.inverted) != 0)
		return NULL;

	char* color;
	if (jsonGetString (config, "backgroundColor", &color) != 0)
		return NULL;
	widgetConfig.frameConfig.backgroundColor = gdkHexToColor (color);

	if (jsonGetString (config, "borderColor", &color) != 0)
		return NULL;
	widgetConfig.frameConfig.borderColor = gdkHexToColor (color);

	jsonGetFloat (config, "borderThickness", &widgetConfig.frameConfig.borderThickness);

	jsonGetFloat (config, "cornerRadius", &widgetConfig.frameConfig.cornerRadius);

	return canFloatPopupInit (database, &widgetConfig);
}