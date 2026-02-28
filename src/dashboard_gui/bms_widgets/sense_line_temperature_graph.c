// Header
#include "sense_line_temperature_graph.h"

// Includes
#include "../gtk_util.h"
#include "bms/bms.h"

bool bmsSenseLineTemperatureGraphAccessor (void* arg, size_t index, float* value)
{
	bms_t* bms = arg;

	bool valid = bmsGetLogicalTemperature (bms, index, value) == CAN_DATABASE_VALID;
	if (!valid)
		*value = 0;

	return valid;
}

void bmsSenseLineTemperatureGraphDrawForeground (stylizedBarGraph_t* graph, cairo_t* cr, stylizedBarGraphDrawContext_t* context)
{
	bms_t* bms = graph->accessorArg;

	GdkRGBA a = gdkHexToColor ("#00FFAA");
	gdk_cairo_set_source_rgba (cr, &a);
	cairo_set_line_width (cr, 1.5f);

	// uint16_t segmentIndex = 0;
	for (size_t index = 0; index <= bms->logicalTemperatureCount; ++index)
	{
		float x = context->xAxisPosition + context->barSpacing * index;
		if (index == 0)
			x += 1.5f / 2.0f;
		else if (index == bms->cellCount)
			x -= 1.5f / 2.0f;

		if (index % (5 * bms->ltcsPerSegment) == 0)
		{
			cairo_move_to (cr, x, context->yAxisPosition + 8);
			cairo_line_to (cr, x, context->yAxisPosition - 8);
		}
		else if (index % 5 == 0)
		{
			cairo_move_to (cr, x, context->yAxisPosition + 4);
			cairo_line_to (cr, x, context->yAxisPosition - 4);
		}
		else
		{
			cairo_move_to (cr, x, context->yAxisPosition + 2);
			cairo_line_to (cr, x, context->yAxisPosition - 2);
		}

		// ssize_t senseLineIndex = bmsGetLogicalTemperatureIndex (bms, index);
		// printf ("IDX %li\n", senseLineIndex);

		// uint16_t segment = senseLineIndex / (bms->senseLinesPerLtc * bms->ltcsPerSegment);
		// if (segment == segmentIndex)
		// 	continue;

		if (index % 10 != 0)
			continue;

		// segmentIndex = segment;

		for (float y = context->yAxisPosition; y > 0; y -= context->tickSpacing / 2.0f)
		{
			cairo_move_to (cr, x - 2.5f, y);
			cairo_line_to (cr, x + 2.5f, y);
		}
	}

	cairo_stroke (cr);
}