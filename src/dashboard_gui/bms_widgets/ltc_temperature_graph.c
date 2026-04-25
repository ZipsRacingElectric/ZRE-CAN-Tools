// Header
#include "ltc_temperature_graph.h"

// Includes
#include "bms/bms.h"

stylizedBarGraphAccessorReturn_t bmsLtcTemperatureGraphAccessor (void* arg, size_t index, float* value)
{
	bms_t* bms = arg;

	bool valid = bmsGetLtcTemperature (bms, index, value) == CAN_DATABASE_VALID;
	if (!valid)
		*value = 0;

	return valid ? STYLIZED_BAR_GRAPH_VALID : STYLIZED_BAR_GRAPH_INVALID;
}

void bmsLtcTemperatureGraphDrawForeground (stylizedBarGraph_t* graph, cairo_t* cr, stylizedBarGraphDrawContext_t* context)
{
	bms_t* bms = graph->accessorArg;

	gdk_cairo_set_source_rgba (cr, &graph->config.tickColor2);
	cairo_set_line_width (cr, 1.5f);

	for (size_t index = 0; index <= bms->ltcCount; ++index)
	{
		float x = context->xAxisPosition + context->barSpacing * index;
		if (index == 0)
			x += 1.5f / 2.0f;
		else if (index == bms->cellCount)
			x -= 1.5f / 2.0f;

		if (index % bms->ltcsPerSegment == 0)
		{
			cairo_move_to (cr, x, context->yAxisPosition + 8);
			cairo_line_to (cr, x, context->yAxisPosition - 8);

			for (float y = context->yAxisPosition; y > 0; y -= context->tickSpacing / 2.0f)
			{
				cairo_move_to (cr, x - 2.5f, y);
				cairo_line_to (cr, x + 2.5f, y);
			}
		}
		else
		{
			cairo_move_to (cr, x, context->yAxisPosition + 4);
			cairo_line_to (cr, x, context->yAxisPosition - 4);
		}
	}

	cairo_stroke (cr);
}