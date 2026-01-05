// Header
#include "bms_widgets.h"

// Includes
#include "can_database/can_database_stdio.h"
#include "zre_math.h"

static void draw (GtkDrawingArea* area, cairo_t* cr, int width, int height, gpointer arg)
{
	(void) area;
	bmsBarGraph_t* graph = arg;

	gdk_cairo_set_source_rgba (cr, &graph->config.axisColor);

	cairo_set_line_width (cr, 1);
	cairo_move_to (cr, graph->config.axisPosition, 0);
	cairo_line_to (cr, graph->config.axisPosition, height);
	cairo_stroke (cr);

	cairo_set_line_width (cr, 1);
	cairo_move_to (cr, graph->config.axisPosition, height * graph->zero);
	cairo_line_to (cr, width, height * graph->zero);
	cairo_stroke (cr);

	gdk_cairo_set_source_rgba (cr, &graph->config.tickColor);

	for (size_t index = 0; index < graph->tickCount; ++index)
	{
		float y = height * (1 - graph->tickScalars [index]);

		cairo_text_extents_t extents;
		cairo_text_extents (cr, graph->tickLabels [index], &extents);

		if (index == 0)
			cairo_move_to (cr, graph->config.axisPosition - extents.width - graph->config.barSpacing, y);
		else if (index == graph->tickCount - 1)
			cairo_move_to (cr, graph->config.axisPosition - extents.width - graph->config.barSpacing, y + extents.height);
		else
			cairo_move_to (cr, graph->config.axisPosition - extents.width - graph->config.barSpacing, y + extents.height / 2.0f);

		cairo_text_path (cr, graph->tickLabels [index]);
		cairo_fill (cr);

		cairo_move_to (cr, graph->config.axisPosition, y);
		cairo_line_to (cr, width, y);
		cairo_stroke (cr);
	}

	gdk_cairo_set_source_rgba (cr, &graph->config.barColor);

	float barSize = (float) (width - graph->config.axisPosition - graph->config.barSpacing) / graph->config.count - graph->config.barSpacing;

	for (size_t index = 0; index < graph->config.count; ++index)
	{
		float value;
		if (graph->config.accessor (graph->bms, index + graph->config.offset, &value) != CAN_DATABASE_VALID)
			value = 0;

		// Uncomment below for test values:
		// float value = (graph->config.max - graph->config.min) * index / (graph->config.count - 1.0f) + graph->config.min;
		float scale = inverseLerp (value, graph->config.min, graph->config.max);

		cairo_rectangle (cr,
			index * (barSize + graph->config.barSpacing) + graph->config.axisPosition + graph->config.barSpacing, height * (1 - graph->zero),
			barSize, height * -(scale - graph->zero));

		cairo_fill (cr);
	}
}

static void deallocTicks (bmsBarGraph_t* graph)
{
	if (graph->tickCount != 0)
	{
		for (size_t index = 0; index < graph->tickCount; ++index)
			free (graph->tickLabels [index]);

		free (graph->tickLabels);
		free (graph->tickScalars);
	}
}

void canLabelFloatStaticInit (canLabelFloatStatic_t* label)
{
	label->widget = gtk_label_new ("");
	canLabelFloatStaticUpdate (label, 0, CAN_DATABASE_MISSING, NULL);
}

void canLabelFloatStaticUpdate (canLabelFloatStatic_t* label, float value, canDatabaseSignalState_t state, const char* unit)
{
	char buffer [64];
	snprintCanDatabaseFloatStatic (buffer, sizeof (buffer), label->formatValue, label->formatInvalid, value, state, unit);
	gtk_label_set_text (GTK_LABEL (label->widget), buffer);
}

void bmsBarGraphInit (bmsBarGraph_t* graph, bms_t* bms, bmsBarGraphConfig_t* config)
{
	*graph = (bmsBarGraph_t)
	{
		.widget		= gtk_drawing_area_new (),
		.bms		= bms,
		.config		= *config,
		.tickCount	= 0
	};
	bmsBarSetBounds (graph, config->min, config->max);

	int width = (graph->config.barSize + graph->config.barSpacing) * graph->config.count + 2 * graph->config.barSpacing + graph->config.axisPosition;
	int height = graph->config.length;

	gtk_drawing_area_set_content_width (BMS_BAR_GRAPH_TO_DRAWING_AREA (graph), width);
	gtk_drawing_area_set_content_height (BMS_BAR_GRAPH_TO_DRAWING_AREA (graph), height);
	gtk_drawing_area_set_draw_func (BMS_BAR_GRAPH_TO_DRAWING_AREA (graph), draw, graph, NULL);

	// For some reason, drawing area appears to stretch to fit area by default. Disable this so it defaults to the minimum size.
	gtk_widget_set_halign (BMS_BAR_GRAPH_TO_WIDGET (graph), GTK_ALIGN_START);
	gtk_widget_set_valign (BMS_BAR_GRAPH_TO_WIDGET (graph), GTK_ALIGN_START);
}

void bmsBarSetBounds (bmsBarGraph_t* graph, float min, float max)
{
	deallocTicks (graph);

	graph->config.min = min;
	graph->config.max = max;
	graph->zero = inverseLerp (0, graph->config.min, graph->config.max);

	int tickMin = ceilf (graph->config.min / graph->config.tickSpacing);
	int tickMax = floorf (graph->config.max / graph->config.tickSpacing);
	graph->tickCount = tickMax - tickMin + 1;

	graph->tickLabels = malloc (sizeof (char*) * graph->tickCount);
	graph->tickScalars = malloc (sizeof (float) * graph->tickCount);

	for (size_t index = 0; index < graph->tickCount; ++index)
	{
		int tickInt = index + tickMin;
		float tickValue = tickInt * graph->config.tickSpacing;
		graph->tickScalars [index] = inverseLerp (tickValue, graph->config.min, graph->config.max);

		if (asprintf (&graph->tickLabels [index], graph->config.tickFormat, tickValue) < 0)
			continue;
	}
}

void bmsBarGraphUpdate (bmsBarGraph_t* graph)
{
	gtk_widget_queue_draw (BMS_BAR_GRAPH_TO_WIDGET (graph));
}

void bmsFaultLabelInit (bmsFaultLabel_t* label, bms_t* bms)
{
	*label = (bmsFaultLabel_t)
	{
		.widget	= gtk_label_new (""),
		.bms	= bms
	};
}

void bmsFaultLabelUpdate (bmsFaultLabel_t* label)
{
	char* state = bmsGetFaultState (label->bms);
	if (state == label->state)
		return;

	label->state = state;
	gtk_label_set_text (GTK_LABEL (label->widget), state);
}