// Header
#include "can_indicator.h"

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
} canIndicator_t;

static void draw (GtkDrawingArea* area, cairo_t* cr, int width, int height, gpointer arg)
{
	(void) area;
	canIndicator_t* indicator = arg;

	switch (indicator->state)
	{
	case CAN_INDICATOR_ACTIVE:
		gdk_cairo_set_source_rgba (cr, &indicator->config.activeColor);
		break;

	case CAN_INDICATOR_INACTIVE:
		gdk_cairo_set_source_rgba (cr, &indicator->config.inactiveColor);
		break;

	case CAN_INDICATOR_INVALID:
		gdk_cairo_set_source_rgba (cr, &indicator->config.invalidColor);
		break;
	}

	if (indicator->config.polygonSize == 0)
	{
		// If no polygon is specified, default to a circle.
		cairo_arc (cr, width / 2.0, height / 2.0, MIN (width, height) / 2.0, 0, 2 * G_PI);
	}
	else
	{
		// Draw the provided polygon

		cairo_move_to (cr,
			width * indicator->config.polygon [0][0],
			height * indicator->config.polygon [0][1]);

		for (size_t index = 1; index < indicator->config.polygonSize; ++index)
			cairo_line_to (cr,
				roundf (width * indicator->config.polygon [index][0]),
				roundf (height * indicator->config.polygon [index][1]));

		cairo_close_path (cr);
	}

	cairo_fill (cr);
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

	// If the state has changed, trigger a redraw
	if (state != indicator->state)
	{
		indicator->state = state;
		gtk_widget_queue_draw (CAN_WIDGET_TO_WIDGET (indicator));
	}
}

canWidget_t* canIndicatorInit (canDatabase_t* database, canIndicatorConfig_t* config)
{
	// Allocate the object
	canIndicator_t* indicator = malloc (sizeof (canIndicator_t));
	if (indicator == NULL)
		return NULL;

	// Init the object
	*indicator = (canIndicator_t)
	{
		.vmt =
		{
			.update	= update,
			.widget	= gtk_drawing_area_new ()
		},
		.config		= *config,
		.database	= database,
		.index		= canDatabaseFindSignal (database, config->signalName),
		.state		= CAN_INDICATOR_INVALID
	};

	gtk_drawing_area_set_content_width (CAN_INDICATOR_TO_DRAWING_AREA (indicator), config->width);
	gtk_drawing_area_set_content_height (CAN_INDICATOR_TO_DRAWING_AREA (indicator), config->height);
	gtk_drawing_area_set_draw_func (CAN_INDICATOR_TO_DRAWING_AREA (indicator), draw, indicator, NULL);

	// For some reason, drawing area appears to stretch to fit area by default. Disable this so it defaults to the minimum size.
	gtk_widget_set_halign (CAN_WIDGET_TO_WIDGET (indicator), GTK_ALIGN_CENTER);
	gtk_widget_set_valign (CAN_WIDGET_TO_WIDGET (indicator), GTK_ALIGN_CENTER);

	// Update initial value
	update (indicator);

	// Cast into the base type
	return (canWidget_t*) indicator;
}