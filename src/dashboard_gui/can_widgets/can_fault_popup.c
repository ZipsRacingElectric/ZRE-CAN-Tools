// Header
#include "can_fault_popup.h"

// Includes
#include "../gtk_util.h"
#include "cjson/cjson_util.h"

// Base datatype for the widget.
typedef struct
{
	// Every CAN widget must start with the VMT.
	canWidgetVmt_t vmt;

	// Configuration
	canFaultPopupConfig_t config;

	// CAN database reference.
	canDatabase_t* database;

	// Internal widgets
	stylizedFrame_t* frame;
	GtkLabel* label;

	// Last fault state
	bool faulted;
	size_t index;
} canFaultPopup_t;

static void update (void* widget)
{
	canFaultPopup_t* popup = widget;

	// Check if a fault is present
	size_t index;
	bool faulted = faultSignalsGetIndex (&popup->config.faults, &index) == FAULT_SIGNAL_FAULTED;

	// Ignore if nothing has changed
	if (faulted == popup->faulted && index == popup->index)
		return;
	popup->faulted = faulted;
	popup->index = index;

	// Enable / disable the popup based on whether a fault is present
	gtk_widget_set_visible (CAN_WIDGET_TO_WIDGET (popup), faulted);

	// Get and update the label's text
	if (faulted)
	{
		char* status = faultSignalsGetString (&popup->config.faults);
		char buffer [512];
		snprintf (buffer, 512, popup->config.format, status);
		gtk_label_set_text (popup->label, buffer);
	}
}

canWidget_t* canFaultPopupInit (canDatabase_t* database, canFaultPopupConfig_t* config)
{
	// Allocate the object
	canFaultPopup_t* popup = malloc (sizeof (canFaultPopup_t));
	if (popup == NULL)
		return NULL;

	// Init the object
	*popup = (canFaultPopup_t)
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
		.frame		= stylizedFrameInit (&config->frameConfig),
		.label		= GTK_LABEL (gtk_label_new ("")),
		.faulted	= false,
		.index		= -1
	};

	// Set the base widget
	if (popup->frame == NULL)
		return NULL;
	popup->vmt.widget = STYLIZED_FRAME_TO_WIDGET (popup->frame);

	// Default to not faulted
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

canWidget_t* canFaultPopupLoad (canDatabase_t* database, cJSON* config)
{
	canFaultPopupConfig_t widgetConfig;

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

	char* color;
	if (jsonGetString (config, "backgroundColor", &color) != 0)
		return NULL;
	widgetConfig.frameConfig.backgroundColor = gdkHexToColor (color);

	if (jsonGetString (config, "borderColor", &color) != 0)
		return NULL;
	widgetConfig.frameConfig.borderColor = gdkHexToColor (color);

	jsonGetFloat (config, "borderThickness", &widgetConfig.frameConfig.borderThickness);

	jsonGetFloat (config, "cornerRadius", &widgetConfig.frameConfig.cornerRadius);

	return canFaultPopupInit (database, &widgetConfig);
}