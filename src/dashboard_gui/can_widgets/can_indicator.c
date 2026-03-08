// Header
#include "can_indicator.h"

// Includes
#include "../gtk_util.h"
#include "cjson/cjson_util.h"
#include "debug.h"

typedef enum
{
	CAN_INDICATOR_INVALID,
	CAN_INDICATOR_ACTIVE,
	CAN_INDICATOR_INACTIVE
} canIndicatorState_t;

typedef struct
{
	canWidgetVmt_t vmt;
	canIndicatorConfig_t config;
	canDatabase_t* database;
	ssize_t index;
	canIndicatorState_t state;
	GtkWidget* drawingArea;
	GtkLabel* label;
	float blinkTimer;
} canIndicator_t;

static void draw (GtkDrawingArea* area, cairo_t* cr, int width, int height, gpointer arg)
{
	(void) area;
	canIndicator_t* indicator = arg;

	switch (indicator->state)
	{
	case CAN_INDICATOR_ACTIVE:
		gdk_cairo_set_source_rgba (cr, &indicator->config.style.bgActiveColor);
		break;

	case CAN_INDICATOR_INACTIVE:
		gdk_cairo_set_source_rgba (cr, &indicator->config.style.bgInactiveColor);
		break;

	case CAN_INDICATOR_INVALID:
		gdk_cairo_set_source_rgba (cr, &indicator->config.style.bgInvalidColor);
		break;
	}

	switch (indicator->config.style.shape)
	{
		case CAN_INDICATOR_CIRCLE:
			cairo_arc (cr, width / 2.0, height / 2.0, MIN (width, height) / 2.0, 0, 2 * G_PI);
			break;

		case CAN_INDICATOR_RECT:
		{
			float x0 = indicator->config.style.borderThickness / 2.0f;
			float y0 = indicator->config.style.borderThickness / 2.0f;
			float x3 = width - indicator->config.style.borderThickness / 2.0f;
			float y3 = height - indicator->config.style.borderThickness / 2.0f;
			float x1 = x0 + indicator->config.style.cornerRadius;
			float y1 = y0 + indicator->config.style.cornerRadius;
			float x2 = x3 - indicator->config.style.cornerRadius;
			float y2 = y3 - indicator->config.style.cornerRadius;

			cairo_move_to (cr, x1, y0);
			cairo_line_to (cr, x2, y0);
			cairo_arc (cr, x2, y1, indicator->config.style.cornerRadius, -G_PI_2, 0);
			cairo_line_to (cr, x3, y2);
			cairo_arc (cr, x2, y2, indicator->config.style.cornerRadius, 0, G_PI_2);
			cairo_line_to (cr, x1, y3);
			cairo_arc (cr, x1, y2, indicator->config.style.cornerRadius, G_PI_2, G_PI);
			cairo_line_to (cr, x0, y1);
			cairo_arc (cr, x1, y1, indicator->config.style.cornerRadius, G_PI, 3 * G_PI_2);
		}
		break;
	}

	cairo_fill_preserve (cr);

	switch (indicator->state)
	{
	case CAN_INDICATOR_ACTIVE:
		gdk_cairo_set_source_rgba (cr, &indicator->config.style.borderActiveColor);
		break;

	case CAN_INDICATOR_INACTIVE:
		gdk_cairo_set_source_rgba (cr, &indicator->config.style.borderInactiveColor);
		break;

	case CAN_INDICATOR_INVALID:
		gdk_cairo_set_source_rgba (cr, &indicator->config.style.borderInvalidColor);
		break;
	}

	cairo_set_line_width (cr, indicator->config.style.borderThickness);
	cairo_stroke (cr);
}

static void update (void* widget)
{
	canIndicator_t* indicator = widget;

	// Get the indicator state
	canIndicatorState_t state;
	float value;
	if (canDatabaseGetFloat (indicator->database, indicator->index, &value) == CAN_DATABASE_VALID)
	{
		if ((value >= indicator->config.threshold) != indicator->config.inverted)
			state = CAN_INDICATOR_ACTIVE;
		else
			state = CAN_INDICATOR_INACTIVE;
	}
	else
		state = CAN_INDICATOR_INVALID;

	if (indicator->config.style.blinkInterval > 0.0f && state == CAN_INDICATOR_ACTIVE)
	{
		indicator->blinkTimer += 1/30.0f;
		if (indicator->blinkTimer < indicator->config.style.blinkInterval / 2.0f)
			state = CAN_INDICATOR_ACTIVE;
		else if (indicator->blinkTimer < indicator->config.style.blinkInterval)
			state = CAN_INDICATOR_INACTIVE;
		else
			indicator->blinkTimer = 0.0f;
	}

	// If the state has changed, trigger a redraw
	if (state != indicator->state)
	{
		indicator->state = state;

		if (indicator->label != NULL)
		{
			switch (indicator->state)
			{
			case CAN_INDICATOR_ACTIVE:
				gtkLabelSetColor (indicator->label, &indicator->config.style.fontActiveColor);
				break;

			case CAN_INDICATOR_INACTIVE:
				gtkLabelSetColor (indicator->label, &indicator->config.style.fontInactiveColor);
				break;

			case CAN_INDICATOR_INVALID:
				gtkLabelSetColor (indicator->label, &indicator->config.style.fontInvalidColor);
				break;
			}
		}

		gtk_widget_queue_draw (indicator->drawingArea);
	}
}

canWidget_t* canIndicatorInit (canDatabase_t* database, canIndicatorConfig_t* config)
{
	// Allocate the CAN widget
	canIndicator_t* indicator = malloc (sizeof (canIndicator_t));
	if (indicator == NULL)
		return NULL;

	// Setup the widget(s)
	GtkWidget* baseWidget;
	GtkWidget* drawingArea = gtk_drawing_area_new ();
	GtkLabel* label = NULL;
	if (config->text == NULL)
	{
		// If no text, the widget is just the drawing area
		baseWidget = drawingArea;
	}
	else
	{
		// If text, use an overlay + drawing area + label.

		baseWidget = gtk_overlay_new ();

		gtk_overlay_set_child (GTK_OVERLAY (baseWidget), drawingArea);

		label = GTK_LABEL (gtk_label_new (config->text));
		gtk_label_set_justify (label, GTK_JUSTIFY_CENTER);
		gtkLabelSetColor (label, &config->style.fontActiveColor);
		if (config->style.font != NULL)
			gtkLabelSetFont (label, config->style.font);

		gtk_overlay_add_overlay (GTK_OVERLAY (baseWidget), GTK_WIDGET (label));
		gtk_overlay_set_measure_overlay (GTK_OVERLAY (baseWidget), GTK_WIDGET (label), true);
	}

	// Init the CAN widget
	*indicator = (canIndicator_t)
	{
		.vmt =
		{
			.update		= update,
			.widget		= baseWidget
		},
		.config			= *config,
		.database		= database,
		.index			= canDatabaseFindSignal (database, config->signalName),
		.state			= CAN_INDICATOR_ACTIVE,
		.drawingArea	= drawingArea,
		.label			= label
	};

	gtk_drawing_area_set_content_width (GTK_DRAWING_AREA (drawingArea), config->width);
	gtk_drawing_area_set_content_height (GTK_DRAWING_AREA (drawingArea), config->height);
	gtk_drawing_area_set_draw_func (GTK_DRAWING_AREA (drawingArea), draw, indicator, NULL);

	// For some reason, drawing area appears to stretch to fit area by default. Disable this so it defaults to the minimum size.
	gtk_widget_set_halign (drawingArea, GTK_ALIGN_CENTER);
	gtk_widget_set_valign (drawingArea, GTK_ALIGN_CENTER);

	// Update initial value
	update (indicator);

	// Cast into the base type
	return (canWidget_t*) indicator;
}

canWidget_t* canIndicatorLoad (canDatabase_t* database, cJSON* config, canIndicatorStyle_t* parentStyle)
{
	canIndicatorConfig_t widgetConfig;

	// Load config fields. Exit early is required field is not specified.

	if (jsonGetString (config, "signalName", &widgetConfig.signalName) != 0)
		return NULL;
	if (jsonGetFloat (config, "threshold", &widgetConfig.threshold) != 0)
		return NULL;

	widgetConfig.inverted = false;
	jsonGetBool (config, "inverted", &widgetConfig.inverted);

	widgetConfig.text = NULL;
	jsonGetString (config, "text", &widgetConfig.text);

	widgetConfig.width = 0;
	jsonGetInt (config, "width", &widgetConfig.width);
	widgetConfig.height = 0;
	jsonGetInt (config, "height", &widgetConfig.height);

	cJSON* styleConfig = jsonGetObjectV2 (config, "style");
	canIndicatorLoadStyle (styleConfig, &widgetConfig.style, parentStyle);

	return canIndicatorInit (database, &widgetConfig);
}

void canIndicatorLoadStyle (cJSON* config, canIndicatorStyle_t* style, canIndicatorStyle_t* parent)
{
	if (parent != NULL)
		// If a parent is specified, inherit everything.
		*style = *parent;
	else
	{
		// If no parent is specified, use default values.
		*style = (canIndicatorStyle_t)
		{
			.font					= NULL,
			.shape					= CAN_INDICATOR_RECT,
			.bgActiveColor			= gdkHexToColor ("#000000"),
			.bgInactiveColor		= gdkHexToColor ("#000000"),
			.bgInvalidColor			= gdkHexToColor ("#000000"),
			.borderActiveColor		= gdkHexToColor ("#FFFFFF"),
			.borderInactiveColor	= gdkHexToColor ("#FFFFFF"),
			.borderInvalidColor		= gdkHexToColor ("#FFFFFF"),
			.fontActiveColor		= gdkHexToColor ("#FFFFFF"),
			.fontInactiveColor		= gdkHexToColor ("#FFFFFF"),
			.fontInvalidColor		= gdkHexToColor ("#FFFFFF"),
			.cornerRadius			= 0,
			.borderThickness		= 1.5,
			.blinkInterval			= 0,
		};
	}

	// If no config was provided, use what we have.
	if (config == NULL)
		return;

	// Load style fields, if specified.

	jsonGetString (config, "font", &style->font);

	char* shapeStr;
	if (jsonGetString (config, "shape", &shapeStr) == 0)
	{
		if (strcmp (shapeStr, "circle") == 0)
			style->shape = CAN_INDICATOR_CIRCLE;
		else if (strcmp (shapeStr, "rect") == 0)
			style->shape = CAN_INDICATOR_RECT;
		else
			debugPrintf ("Invalid CAN indicator shape '%s'.\n", shapeStr);
	}

	char* color;
	if (jsonGetString (config, "bgActiveColor", &color) == 0)
		style->bgActiveColor = gdkHexToColor (color);
	if (jsonGetString (config, "bgInactiveColor", &color) == 0)
		style->bgInactiveColor = gdkHexToColor (color);
	if (jsonGetString (config, "bgInvalidColor", &color) == 0)
		style->bgInvalidColor = gdkHexToColor (color);
	if (jsonGetString (config, "borderActiveColor", &color) == 0)
		style->borderActiveColor = gdkHexToColor (color);
	if (jsonGetString (config, "borderInactiveColor", &color) == 0)
		style->borderInactiveColor = gdkHexToColor (color);
	if (jsonGetString (config, "borderInvalidColor", &color) == 0)
		style->borderInvalidColor = gdkHexToColor (color);
	if (jsonGetString (config, "fontActiveColor", &color) == 0)
		style->fontActiveColor = gdkHexToColor (color);
	if (jsonGetString (config, "fontInactiveColor", &color) == 0)
		style->fontInactiveColor = gdkHexToColor (color);
	if (jsonGetString (config, "fontInvalidColor", &color) == 0)
		style->fontInvalidColor = gdkHexToColor (color);

	jsonGetFloat (config, "cornerRadius", &style->cornerRadius);
	jsonGetFloat (config, "borderThickness", &style->borderThickness);
	jsonGetFloat (config, "blinkInterval", &style->blinkInterval);
}