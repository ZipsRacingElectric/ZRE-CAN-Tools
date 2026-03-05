// Header
#include "can_shutdown_loop_indicator.h"

// Includes
#include "can_widget.h"
#include "../gtk_util.h"
#include "cjson/cjson_util.h"

// TODO(Barach): Docs and cleanup

// Base datatype for the widget.
typedef struct
{
	// Every CAN widget must start with the VMT.
	canWidgetVmt_t vmt;

	// Configuration
	canShutdownLoopIndicatorConfig_t config;

	// CAN database reference.
	canDatabase_t* database;

	ssize_t index;
	bool closed;
	float animationTimer;
} canShutdownLoopIndicator_t;

static void update (void* widget)
{
	canShutdownLoopIndicator_t* indicator = widget;

	// Get the indicator state
	float value;
	bool closed;
	if (canDatabaseGetFloat (indicator->database, indicator->index, &value) == CAN_DATABASE_VALID)
		closed = ((value >= indicator->config.threshold) != indicator->config.inverted);
	else
		closed = false;

	if (closed)
	{
		indicator->animationTimer += 1 / 30.0f;
		if (indicator->animationTimer > indicator->config.style.animationTime + indicator->config.style.animationDelay)
			indicator->animationTimer = indicator->config.style.animationTime + indicator->config.style.animationDelay;
	}
	else
	{
		indicator->animationTimer -= 1 / 30.0f;
		if (indicator->animationTimer < -indicator->config.style.animationDelay)
			indicator->animationTimer = -indicator->config.style.animationDelay;
	}

	gtk_widget_queue_draw (CAN_WIDGET_TO_WIDGET (indicator));
}

static void draw (GtkDrawingArea* area, cairo_t* cr, int width, int height, gpointer arg)
{
	(void) area;
	(void) height;
	canShutdownLoopIndicator_t* indicator = arg;

	float t = MIN (MAX ((indicator->animationTimer) / indicator->config.style.animationTime, 0), 1);

	float x0 = t * (indicator->config.style.closedPosition - indicator->config.style.openPosition) +
		indicator->config.style.openPosition;
	float y0 = 0;

	float x1 = x0 + indicator->config.style.socketLength;
	float y1 = y0;

	float x2 = x1;
	float y2 = (indicator->config.style.plugThickness - indicator->config.style.socketSize) / 2.0f;

	float x3 = x2 - indicator->config.style.socketDepth;
	float y3 = y2;

	float x4 = x3;
	float y4 = y2 + indicator->config.style.socketSize;

	float x5 = x2;
	float y5 = y4;

	float x6 = x2;
	float y6 = indicator->config.style.plugThickness;

	float x7 = x0;
	float y7 = y6;

	gdk_cairo_set_source_rgba (cr, &indicator->config.style.plugColor);

	cairo_move_to (cr, x0, y0);
	cairo_line_to (cr, x1, y1);
	cairo_line_to (cr, x2, y2);
	cairo_line_to (cr, x3, y3);
	cairo_line_to (cr, x4, y4);
	cairo_line_to (cr, x5, y5);
	cairo_line_to (cr, x6, y6);
	cairo_line_to (cr, x7, y7);
	cairo_close_path (cr);

	cairo_fill_preserve (cr);

	gdk_cairo_set_source_rgba (cr, &indicator->config.style.borderColor);
	cairo_set_line_width (cr, indicator->config.style.borderThickness);

	cairo_stroke (cr);

	float x10 = indicator->config.style.closedPosition + indicator->config.style.socketLength;
	float y10 = 0;
	float x11 = width - indicator->config.style.openPosition;
	float y11 = y10;
	float x12 = x11;
	float y12 = indicator->config.style.plugThickness;
	float x13 = x10;
	float y13 = y12;
	float x14 = x13;
	float y14 = y4;
	float x15 = x14 - indicator->config.style.socketDepth;
	float y15 = y14;
	float x16 = x15;
	float y16 = y2;
	float x17 = x14;
	float y17 = y16;

	gdk_cairo_set_source_rgba (cr, &indicator->config.style.plugColor);

	cairo_move_to (cr, x10, y10);
	cairo_line_to (cr, x11, y11);
	cairo_line_to (cr, x12, y12);
	cairo_line_to (cr, x13, y13);
	cairo_line_to (cr, x14, y14);
	cairo_line_to (cr, x15, y15);
	cairo_line_to (cr, x16, y16);
	cairo_line_to (cr, x17, y17);
	cairo_close_path (cr);

	cairo_fill_preserve (cr);

	gdk_cairo_set_source_rgba (cr, &indicator->config.style.borderColor);
	cairo_set_line_width (cr, indicator->config.style.borderThickness);

	cairo_stroke (cr);

	float xLineMin = 0;
	float xLineMax = x0;
	float xRange = xLineMax - xLineMin;
	float yLineMin = (indicator->config.style.plugThickness - indicator->config.style.lineThickness) / 2.0f;
	float yLineMax = (indicator->config.style.plugThickness + indicator->config.style.lineThickness) / 2.0f;

	float w = 1 - t;

	gdk_cairo_set_source_rgba (cr, &indicator->config.style.lineColor);

	cairo_move_to (cr, xLineMin, yLineMin);

	for (int index = 0; index < 15; ++index)
	{
		float x = (index / (15 - 1.0f) - 0.5f) / (w + 1);
		float y = indicator->config.style.peakHeight * MAX (w * cosf (M_PI * x * (2*w + 1)), 0) + yLineMin;

		cairo_line_to (cr, xRange * (x + 0.5f) + xLineMin, y);
	}

	cairo_line_to (cr, xLineMax, yLineMin);
	cairo_line_to (cr, xLineMax, yLineMax);

	for (int index = 14; index >= 0; --index)
	{
		float x = (index / (15 - 1.0f) - 0.5f) / (w + 1);
		float y = indicator->config.style.peakHeight * MAX (w * cosf (M_PI * x * (w + 1)), 0) + yLineMax;

		cairo_line_to (cr, xRange * (x + 0.5f) + xLineMin, y);
	}

	cairo_line_to (cr, xLineMin, yLineMax);

	cairo_fill (cr);

	cairo_move_to (cr, x11, yLineMin);
	cairo_line_to (cr, width, yLineMin);
	cairo_line_to (cr, width, yLineMax);
	cairo_line_to (cr, x11, yLineMax);
	cairo_fill (cr);

	gdk_cairo_set_source_rgba (cr, &indicator->config.style.textColor);
	cairo_set_font_size (cr, indicator->config.style.fontSize);
	cairo_move_to (cr, x7 + indicator->config.style.textPosition, y7 - indicator->config.style.textPosition);
	cairo_text_path (cr, indicator->config.text);
	cairo_fill (cr);
}

canWidget_t* canShutdownLoopIndicatorInit (canDatabase_t* database, canShutdownLoopIndicatorConfig_t* config)
{
	// Allocate the object
	canShutdownLoopIndicator_t* indicator = malloc (sizeof (canShutdownLoopIndicator_t));
	if (indicator == NULL)
		return NULL;

	// Init the object
	*indicator = (canShutdownLoopIndicator_t)
	{
		// Init the VMT
		.vmt =
		{
			.update 	= update,
			.widget		= gtk_drawing_area_new ()
		},
		.config			= *config,
		.database		= database,
		.index			= canDatabaseFindSignal (database, config->signalName),
		.animationTimer	= config->style.animationTime + config->style.animationDelay
	};

	gtk_drawing_area_set_content_width (GTK_DRAWING_AREA (CAN_WIDGET_TO_WIDGET (indicator)), config->width);
	gtk_drawing_area_set_content_height (GTK_DRAWING_AREA (CAN_WIDGET_TO_WIDGET (indicator)), config->height);
	gtk_drawing_area_set_draw_func (GTK_DRAWING_AREA (CAN_WIDGET_TO_WIDGET (indicator)), draw, indicator, NULL);

	// For some reason, drawing area appears to stretch to fit area by default. Disable this so it defaults to the minimum size.
	gtk_widget_set_halign (CAN_WIDGET_TO_WIDGET (indicator), GTK_ALIGN_CENTER);
	gtk_widget_set_valign (CAN_WIDGET_TO_WIDGET (indicator), GTK_ALIGN_CENTER);

	// Update initial value
	update (indicator);

	// Cast into the base type
	return (canWidget_t*) indicator;
}

canWidget_t* canShutdownLoopIndicatorLoad (canDatabase_t* database, cJSON* config, canShutdownLoopIndicatorStyle_t* parentStyle)
{
	canShutdownLoopIndicatorConfig_t widgetConfig;

	if (jsonGetString (config, "signalName", &widgetConfig.signalName) != 0)
		return NULL;

	if (jsonGetFloat (config, "threshold", &widgetConfig.threshold) != 0)
		return NULL;

	widgetConfig.inverted = false;
	jsonGetBool (config, "inverted", &widgetConfig.inverted);

	widgetConfig.text = "";
	jsonGetString (config, "text", &widgetConfig.text);

	widgetConfig.width = 0;
	jsonGetInt (config, "width", &widgetConfig.width);

	widgetConfig.height = 0;
	jsonGetInt (config, "height", &widgetConfig.height);

	cJSON* style = jsonGetObjectV2 (config, "style");
	canShutdownLoopIndicatorLoadStyle (style, &widgetConfig.style, parentStyle);

	return canShutdownLoopIndicatorInit (database, &widgetConfig);
}

void canShutdownLoopIndicatorLoadStyle (cJSON* config, canShutdownLoopIndicatorStyle_t* style, canShutdownLoopIndicatorStyle_t* parent)
{
	if (parent != NULL)
		*style = *parent;
	else
	{
		*style = (canShutdownLoopIndicatorStyle_t)
		{
			.textPosition		= 0.0f,
			.fontSize			= 12.0f,
			.openPosition		= 10.0f,
			.closedPosition		= 20.0f,
			.socketLength		= 80.0f,
			.socketSize			= 5.0f,
			.socketDepth		= 5.0f,
			.plugThickness		= 20.0f,
			.lineThickness		= 1.5f,
			.peakHeight			= 20.0f,
			.lineColor			= gdkHexToColor ("#AAAAAA"),
			.plugColor			= gdkHexToColor ("#AAAAAA"),
			.textColor			= gdkHexToColor ("#000000"),
			.borderColor		= gdkHexToColor ("#FFFFFF"),
			.borderThickness	= 1.5f,
			.animationTime		= 0.1f,
			.animationDelay		= 0.0f
		};
	}

	if (config == NULL)
		return;

	jsonGetFloat (config, "textPosition", &style->textPosition);
	jsonGetFloat (config, "fontSize", &style->fontSize);
	jsonGetFloat (config, "openPosition", &style->openPosition);
	jsonGetFloat (config, "closedPosition", &style->closedPosition);
	jsonGetFloat (config, "socketLength", &style->socketLength);
	jsonGetFloat (config, "socketSize", &style->socketSize);
	jsonGetFloat (config, "socketDepth", &style->socketDepth);
	jsonGetFloat (config, "plugThickness", &style->plugThickness);
	jsonGetFloat (config, "lineThickness", &style->lineThickness);
	jsonGetFloat (config, "peakHeight", &style->peakHeight);

	char* color;

	if (jsonGetString (config, "lineColor", &color) == 0)
		style->lineColor = gdkHexToColor (color);
	if (jsonGetString (config, "plugColor", &color) == 0)
		style->plugColor = gdkHexToColor (color);
	if (jsonGetString (config, "textColor", &color) == 0)
		style->textColor = gdkHexToColor (color);
	if (jsonGetString (config, "borderColor", &color) == 0)
		style->borderColor = gdkHexToColor (color);

	jsonGetFloat (config, "borderThickness", &style->borderThickness);
	jsonGetFloat (config, "animationTime", &style->animationTime);
	jsonGetFloat (config, "animationDelay", &style->animationDelay);
}