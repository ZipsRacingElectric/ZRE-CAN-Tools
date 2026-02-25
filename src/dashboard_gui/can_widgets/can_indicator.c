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
		gdk_cairo_set_source_rgba (cr, &indicator->config.bgActiveColor);
		break;

	case CAN_INDICATOR_INACTIVE:
		gdk_cairo_set_source_rgba (cr, &indicator->config.bgInactiveColor);
		break;

	case CAN_INDICATOR_INVALID:
		gdk_cairo_set_source_rgba (cr, &indicator->config.bgInvalidColor);
		break;
	}

	switch (indicator->config.shape)
	{
		case CAN_INDICATOR_CIRCLE:
			cairo_arc (cr, width / 2.0, height / 2.0, MIN (width, height) / 2.0, 0, 2 * G_PI);
			break;

		case CAN_INDICATOR_RECT:
		{
			float x0 = indicator->config.borderThickness / 2.0f;
			float y0 = indicator->config.borderThickness / 2.0f;
			float x3 = width - indicator->config.borderThickness / 2.0f;
			float y3 = height - indicator->config.borderThickness / 2.0f;
			float x1 = x0 + indicator->config.cornerRadius;
			float y1 = y0 + indicator->config.cornerRadius;
			float x2 = x3 - indicator->config.cornerRadius;
			float y2 = y3 - indicator->config.cornerRadius;

			cairo_move_to (cr, x1, y0);
			cairo_line_to (cr, x2, y0);
			cairo_arc (cr, x2, y1, indicator->config.cornerRadius, -G_PI_2, 0);
			cairo_line_to (cr, x3, y2);
			cairo_arc (cr, x2, y2, indicator->config.cornerRadius, 0, G_PI_2);
			cairo_line_to (cr, x1, y3);
			cairo_arc (cr, x1, y2, indicator->config.cornerRadius, G_PI_2, G_PI);
			cairo_line_to (cr, x0, y1);
			cairo_arc (cr, x1, y1, indicator->config.cornerRadius, G_PI, 3 * G_PI_2);
		}
		break;
	}

	cairo_fill_preserve (cr);

	switch (indicator->state)
	{
	case CAN_INDICATOR_ACTIVE:
		gdk_cairo_set_source_rgba (cr, &indicator->config.borderActiveColor);
		break;

	case CAN_INDICATOR_INACTIVE:
		gdk_cairo_set_source_rgba (cr, &indicator->config.borderInactiveColor);
		break;

	case CAN_INDICATOR_INVALID:
		gdk_cairo_set_source_rgba (cr, &indicator->config.borderInvalidColor);
		break;
	}

	cairo_set_line_width (cr, indicator->config.borderThickness);
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

	if (indicator->config.blinkInterval > 0.0f && state == CAN_INDICATOR_ACTIVE)
	{
		indicator->blinkTimer += 1/30.0f;
		if (indicator->blinkTimer < indicator->config.blinkInterval / 2.0f)
			state = CAN_INDICATOR_ACTIVE;
		else if (indicator->blinkTimer < indicator->config.blinkInterval)
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
				gtkLabelSetColor (indicator->label, &indicator->config.fontActiveColor);
				break;

			case CAN_INDICATOR_INACTIVE:
				gtkLabelSetColor (indicator->label, &indicator->config.fontInactiveColor);
				break;

			case CAN_INDICATOR_INVALID:
				gtkLabelSetColor (indicator->label, &indicator->config.fontInvalidColor);
				break;
			}
		}

		gtk_widget_queue_draw (indicator->drawingArea);
	}
}

canWidget_t* canIndicatorInit (canDatabase_t* database, canIndicatorConfig_t* config)
{
	// Allocate the object
	canIndicator_t* indicator = malloc (sizeof (canIndicator_t));
	if (indicator == NULL)
		return NULL;

	GtkWidget* baseWidget;
	GtkWidget* drawingArea = gtk_drawing_area_new ();
	GtkLabel* label = NULL;
	if (config->text == NULL)
	{
		baseWidget = drawingArea;
	}
	else
	{
		baseWidget = gtk_overlay_new ();

		gtk_overlay_set_child (GTK_OVERLAY (baseWidget), drawingArea);

		label = GTK_LABEL (gtk_label_new (config->text));
		gtkLabelSetFont (label, config->font);
		gtk_label_set_justify (label, GTK_JUSTIFY_CENTER);
		gtkLabelSetColor (label, &config->fontActiveColor);

		gtk_overlay_add_overlay (GTK_OVERLAY (baseWidget), GTK_WIDGET (label));
		gtk_overlay_set_measure_overlay (GTK_OVERLAY (baseWidget), GTK_WIDGET (label), true);
	}

	// Init the object
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

canWidget_t* canIndicatorLoad (canDatabase_t* database, cJSON* config)
{
	char* signalName;
	if (jsonGetString (config, "signalName", &signalName) != 0)
		return NULL;

	float threshold;
	if (jsonGetFloat (config, "threshold", &threshold) != 0)
		return NULL;

	bool inverted;
	if (jsonGetBool (config, "inverted", &inverted) != 0)
		return NULL;

	uint16_t width = 0;
	jsonGetUint16_t (config, "width", &width);

	uint16_t height = 0;
	jsonGetUint16_t (config, "height", &height);

	char* bgActiveColor;
	if (jsonGetString (config, "bgActiveColor", &bgActiveColor) != 0)
		return NULL;

	char* bgInactiveColor;
	if (jsonGetString (config, "bgInactiveColor", &bgInactiveColor) != 0)
		bgInactiveColor = bgActiveColor;

	char* bgInvalidColor;
	if (jsonGetString (config, "bgInvalidColor", &bgInvalidColor) != 0)
		bgInvalidColor = bgInactiveColor;

	char* borderActiveColor = "#000000";
	jsonGetString (config, "borderActiveColor", &borderActiveColor);

	char* borderInactiveColor;
	if (jsonGetString (config, "borderInactiveColor", &borderInactiveColor) != 0)
		borderInactiveColor = borderActiveColor;

	char* borderInvalidColor;
	if (jsonGetString (config, "borderInvalidColor", &borderInvalidColor) != 0)
		borderInvalidColor = borderInactiveColor;

	char* fontActiveColor = "#000000";
	jsonGetString (config, "fontActiveColor", &fontActiveColor);

	char* fontInactiveColor;
	if (jsonGetString (config, "fontInactiveColor", &fontInactiveColor) != 0)
		fontInactiveColor = fontActiveColor;

	char* fontInvalidColor;
	if (jsonGetString (config, "fontInvalidColor", &fontInvalidColor) != 0)
		fontInvalidColor = fontInactiveColor;

	float borderThickness = 0;
	jsonGetFloat (config, "borderThickness", &borderThickness);

	float cornerRadius = 0;
	jsonGetFloat (config, "cornerRadius", &cornerRadius);

	float blinkInterval = 0;
	jsonGetFloat (config, "blinkInterval", &blinkInterval);

	canIndicatorShape_t shape;
	char* shapeStr;
	if (jsonGetString (config, "shape", &shapeStr) != 0)
		return NULL;

	if (strcmp (shapeStr, "circle") == 0)
		shape = CAN_INDICATOR_CIRCLE;
	else if (strcmp (shapeStr, "rect") == 0)
		shape = CAN_INDICATOR_RECT;
	else
	{
		debugPrintf ("Invalid CAN indicator shape '%s'.\n", shapeStr);
		errno = EINVAL;
		return NULL;
	}

	char* text = NULL;
	jsonGetString (config, "text", &text);

	char* font = "";
	jsonGetString (config, "font", &font);

	char* fontColor = "#000000";
	jsonGetString (config, "fontColor", &fontColor);

	return canIndicatorInit (database, &(canIndicatorConfig_t)
	{
		.signalName				= signalName,
		.threshold				= threshold,
		.inverted				= inverted,
		.width					= width,
		.height					= height,
		.bgActiveColor			= gdkHexToColor (bgActiveColor),
		.bgInactiveColor		= gdkHexToColor (bgInactiveColor),
		.bgInvalidColor			= gdkHexToColor (bgInvalidColor),
		.borderActiveColor		= gdkHexToColor (borderActiveColor),
		.borderInactiveColor	= gdkHexToColor (borderInactiveColor),
		.borderInvalidColor		= gdkHexToColor (borderInvalidColor),
		.fontActiveColor		= gdkHexToColor (fontActiveColor),
		.fontInactiveColor		= gdkHexToColor (fontInactiveColor),
		.fontInvalidColor		= gdkHexToColor (fontInvalidColor),
		.shape					= shape,
		.cornerRadius			= cornerRadius,
		.text					= text,
		.font					= font,
		.borderThickness		= borderThickness,
		.blinkInterval			= blinkInterval
	});
}