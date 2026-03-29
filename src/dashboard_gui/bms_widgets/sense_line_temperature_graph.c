// Header
#include "sense_line_temperature_graph.h"

// Includes
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

	gdk_cairo_set_source_rgba (cr, &graph->config.tickColor2);
	cairo_set_line_width (cr, 1.5f);

	size_t lastLtcIndex = -1;
	size_t lastSegmentIndex = -1;
	for (size_t index = 0; index <= bms->logicalTemperatureCount; ++index)
	{
		float x = context->xAxisPosition + context->barSpacing * index;
		if (index == 0)
			x += 1.5f / 2.0f;
		else if (index == bms->cellCount)
			x -= 1.5f / 2.0f;

		size_t senseLineIndex = index < bms->logicalTemperatureCount ? bmsGetLogicalTemperatureSenseLineIndex (bms, index) : -1;
		size_t ltcIndex = senseLineIndex / bms->senseLinesPerLtc;
		size_t segmentIndex = ltcIndex / bms->ltcsPerSegment;

		if (lastSegmentIndex != segmentIndex)
		{
			cairo_move_to (cr, x, context->yAxisPosition + 8);
			cairo_line_to (cr, x, context->yAxisPosition - 8);

			for (float y = context->yAxisPosition; y > 0; y -= context->tickSpacing / 2.0f)
			{
				cairo_move_to (cr, x - 2.5f, y);
				cairo_line_to (cr, x + 2.5f, y);
			}
		}
		else if (lastLtcIndex != ltcIndex)
		{
			cairo_move_to (cr, x, context->yAxisPosition + 4);
			cairo_line_to (cr, x, context->yAxisPosition - 4);
		}
		else
		{
			cairo_move_to (cr, x, context->yAxisPosition + 2);
			cairo_line_to (cr, x, context->yAxisPosition - 2);
		}

		lastLtcIndex = ltcIndex;
		lastSegmentIndex = segmentIndex;
	}

	cairo_stroke (cr);
}