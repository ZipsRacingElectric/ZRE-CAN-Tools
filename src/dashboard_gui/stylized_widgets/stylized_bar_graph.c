// For asprintf. Note this must be the first include in this file.
#define _GNU_SOURCE
#include <stdio.h>

// Header
#include "stylized_bar_graph.h"

// Includes
#include "zre_math.h"

static void draw (GtkDrawingArea* area, cairo_t* cr, int width, int height, gpointer arg)
{
	(void) area;
	stylizedBarGraph_t* graph = arg;

	float barSize = (float) (width - graph->config.axisPosition - graph->config.barSpacing) / graph->config.count - graph->config.barSpacing;

	stylizedBarGraphDrawContext_t context =
	{
		.canvasWidth	= width,
		.canvasHeight	= height,
		.graphWidth		= width - graph->config.axisPosition,
		.graphHeight	= height - 20,
		.xAxisPosition	= graph->config.axisPosition,
		.yAxisPosition	= height - 20,
		.barSize		= barSize,
		.barSpacing		= barSize + graph->config.barSpacing,
		.tickSpacing	= graph->tickScalar * (height - 20)
	};

	if (graph->config.backgroundDraw != NULL)
		graph->config.backgroundDraw (graph, cr, &context);

	gdk_cairo_set_source_rgba (cr, &graph->config.axisColor);
	cairo_set_line_width (cr, 1);

	cairo_move_to (cr, context.xAxisPosition, 0);
	cairo_line_to (cr, context.xAxisPosition, context.yAxisPosition);

	cairo_move_to (cr, context.xAxisPosition, context.graphHeight * (1 - graph->zero));
	cairo_line_to (cr, context.canvasWidth, context.graphHeight * (1 - graph->zero));

	cairo_stroke (cr);

	gdk_cairo_set_source_rgba (cr, &graph->config.tickColor);

	for (size_t index = 0; index < graph->tickCount; ++index)
	{
		float y = context.graphHeight * (1 - graph->tickScalar * index);

		cairo_text_extents_t extents;
		cairo_text_extents (cr, graph->tickLabels [index], &extents);

		if (index == graph->tickCount - 1)
			cairo_move_to (cr, context.xAxisPosition - extents.width - graph->config.barSpacing, y + extents.height);
		else
			cairo_move_to (cr, context.xAxisPosition - extents.width - graph->config.barSpacing, y + extents.height / 2.0f);

		cairo_text_path (cr, graph->tickLabels [index]);
		cairo_fill (cr);

		cairo_move_to (cr, graph->config.axisPosition, y);
		cairo_line_to (cr, width, y);
		cairo_stroke (cr);
	}

	for (size_t index = 0; index < graph->config.count; ++index)
	{
		// Get the value and color of the bar.
		float value;
		if (graph->config.accessor (graph->accessorArg, index + graph->config.offset, &value))
			gdk_cairo_set_source_rgba (cr, &graph->config.validBarColor);
		else
			gdk_cairo_set_source_rgba (cr, &graph->config.invalidBarColor);

		// Uncomment below for test values:
		// float value = (graph->config.max - graph->config.min) * index / (graph->config.count - 1.0f) + graph->config.min;
		float scale = inverseLerp (value, graph->config.min, graph->config.max);
		if (scale < 0)
			scale = 0;
		if (scale > 1)
			scale = 1;

		cairo_rectangle (cr,
			index * (context.barSpacing) + context.xAxisPosition + graph->config.barSpacing, context.graphHeight * (1 - graph->zero),
			context.barSize, context.graphHeight * -(scale - graph->zero));

		cairo_fill (cr);
	}

	if (graph->config.foregroundDraw != NULL)
		graph->config.foregroundDraw (graph, cr, &context);
}

static void deallocTicks (stylizedBarGraph_t* graph)
{
	if (graph->tickCount != 0)
	{
		for (size_t index = 0; index < graph->tickCount; ++index)
			free (graph->tickLabels [index]);

		free (graph->tickLabels);
	}
}

void stylizedBarGraphInit (stylizedBarGraph_t* graph, stylizedBarGraphConfig_t* config, void* accessorArg)
{
	GtkWidget* widget = gtk_drawing_area_new ();

	*graph = (stylizedBarGraph_t)
	{
		.widget			= widget,
		.accessorArg	= accessorArg,
		.config			= *config,
		.tickCount		= 0
	};
	stylizedBarGraphSetBounds (graph, config->min, config->max);

	int width = (graph->config.barSize + graph->config.barSpacing) * graph->config.count + 2 * graph->config.barSpacing + graph->config.axisPosition;
	int height = graph->config.length;

	gtk_drawing_area_set_content_width (GTK_DRAWING_AREA (widget), width);
	gtk_drawing_area_set_content_height (GTK_DRAWING_AREA (widget), height);
	gtk_drawing_area_set_draw_func (GTK_DRAWING_AREA (widget), draw, graph, NULL);

	// For some reason, drawing area appears to stretch to fit area by default. Disable this so it defaults to the minimum size.
	gtk_widget_set_halign (STYLIZED_BAR_GRAPH_TO_WIDGET (graph), GTK_ALIGN_START);
	gtk_widget_set_valign (STYLIZED_BAR_GRAPH_TO_WIDGET (graph), GTK_ALIGN_START);
}

void stylizedBarGraphSetBounds (stylizedBarGraph_t* graph, float min, float max)
{
	deallocTicks (graph);

	graph->config.min = min;
	graph->config.max = max;
	graph->zero = inverseLerp (0, graph->config.min, graph->config.max);
	if (graph->zero < 0)
		graph->zero = 0;
	if (graph->zero > 1)
		graph->zero = 1;

	int tickMin = ceilf (graph->config.min / graph->config.tickSpacing);
	int tickMax = floorf (graph->config.max / graph->config.tickSpacing);
	graph->tickCount = tickMax - tickMin + 1;

	graph->tickLabels = malloc (sizeof (char*) * graph->tickCount);

	graph->tickScalar = inverseLerp (graph->config.tickSpacing + graph->config.min, graph->config.min, graph->config.max);

	for (size_t index = 0; index < graph->tickCount; ++index)
	{
		int tickInt = index + tickMin;
		float tickValue = tickInt * graph->config.tickSpacing;

		if (asprintf (&graph->tickLabels [index], graph->config.tickFormat, tickValue) < 0)
			continue;
	}
}

void stylizedBarGraphUpdate (stylizedBarGraph_t* graph)
{
	gtk_widget_queue_draw (STYLIZED_BAR_GRAPH_TO_WIDGET (graph));
}