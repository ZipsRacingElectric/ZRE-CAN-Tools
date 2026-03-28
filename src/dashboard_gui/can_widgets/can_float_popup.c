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
		.frame		= stylizedFrameInit (&config->style.frameConfig),
		.label		= GTK_LABEL (gtk_label_new (""))
	};

	// Validate the signal exists
	if (popup->index < 0)
		return NULL;

	// Set the base widget
	if (popup->frame == NULL)
		return NULL;
	popup->vmt.widget = STYLIZED_FRAME_TO_WIDGET (popup->frame);

	// Default to not active
	gtk_widget_set_visible (CAN_WIDGET_TO_WIDGET (popup), false);

	// Set the frame's child
	stylizedFrameSetChild (popup->frame, GTK_WIDGET (popup->label));

	// Set the font and font color
	if (config->style.font != NULL)
		gtkLabelSetFont (popup->label, config->style.font);
	gtkLabelSetColor (popup->label, &popup->config.style.frameConfig.borderColor);

	// Set the label padding
	int margin = config->style.padding + config->style.frameConfig.borderThickness;
	gtk_widget_set_margin_bottom (GTK_WIDGET (popup->label), margin);
	gtk_widget_set_margin_top (GTK_WIDGET (popup->label), margin);
	gtk_widget_set_margin_start (GTK_WIDGET (popup->label), margin);
	gtk_widget_set_margin_end (GTK_WIDGET (popup->label), margin);

	// Update initial value
	update (popup);

	// Cast into the base type
	return (canWidget_t*) popup;
}

canWidget_t* canFloatPopupLoad (canDatabase_t* database, cJSON* config, canFloatPopupStyle_t* parentStyle)
{
	canFloatPopupConfig_t widgetConfig;

	// Load config fields. Exit early is required field is not specified.

	if (jsonGetString (config, "signalName", &widgetConfig.signalName) != 0)
		return NULL;

	if (jsonGetString (config, "format", &widgetConfig.format) != 0)
		return NULL;

	if (jsonGetFloat (config, "threshold", &widgetConfig.threshold) != 0)
		return NULL;

	widgetConfig.inverted = false;
	jsonGetBool (config, "inverted", &widgetConfig.inverted);
	widgetConfig.width = 0;
	jsonGetInt (config, "width", &widgetConfig.width);
	widgetConfig.height = 0;
	jsonGetInt (config, "height", &widgetConfig.height);

	cJSON* styleConfig = jsonGetObjectV2 (config, "style");
	canFloatPopupLoadStyle (styleConfig, &widgetConfig.style, parentStyle);

	return canFloatPopupInit (database, &widgetConfig);
}

void canFloatPopupLoadStyle (cJSON* config, canFloatPopupStyle_t* style, canFloatPopupStyle_t* parent)
{
	if (parent != NULL)
		// If a parent is specified, inherit everything.
		*style = *parent;
	else
	{
		// If no parent is specified, use default values.
		*style = (canFloatPopupStyle_t)
		{
			.padding				= 0,
			.font					= NULL,
			.frameConfig			=
			{
				.backgroundColor	= gdkHexToColor ("#000000"),
				.borderColor		= gdkHexToColor ("#FFFFFF"),
				.borderThickness	= 1.5f,
				.cornerRadius		= 0
			}
		};
	}

	// If no config was provided, use what we have.
	if (config == NULL)
		return;

	// Load style fields, if specified.

	jsonGetInt (config, "padding", &style->padding);
	jsonGetString (config, "font", &style->font);

	char* color;
	if (jsonGetString (config, "backgroundColor", &color) == 0)
		style->frameConfig.backgroundColor = gdkHexToColor (color);
	if (jsonGetString (config, "borderColor", &color) == 0)
		style->frameConfig.borderColor = gdkHexToColor (color);

	jsonGetFloat (config, "borderThickness", &style->frameConfig.borderThickness);
	jsonGetFloat (config, "cornerRadius", &style->frameConfig.cornerRadius);
}