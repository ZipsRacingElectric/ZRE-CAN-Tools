// Header
#include "cell_voltage_bar_graph.h"

// Includes
#include "../gtk_util.h"
#include "bms/bms.h"

bool bmsCellVoltageBarGraphAccessor (void* arg, size_t index, float* value)
{
	bms_t* bms = arg;

	bool loOpen = true;
	bmsGetSenseLineOpen (bms, bmsGetCellLoSenseLineIndex (bms, index), &loOpen);

	bool hiOpen = true;
	bmsGetSenseLineOpen (bms, bmsGetCellHiSenseLineIndex (bms, index), &hiOpen);

	bool valid = bmsGetCellVoltage (bms, index, value) == CAN_DATABASE_VALID;
	if (!valid)
		*value = 0;

	return valid && !loOpen && !hiOpen;
}

void bmsCellVoltageBarGraphDrawForeground (stylizedBarGraph_t* graph, cairo_t* cr, stylizedBarGraphDrawContext_t* context)
{
	bms_t* bms = graph->accessorArg;

	GdkRGBA a = gdkHexToColor ("#00FFAA");
	gdk_cairo_set_source_rgba (cr, &a);
	cairo_set_line_width (cr, 1.5f);

	for (size_t index = 0; index <= bms->cellCount; ++index)
	{
		float x = context->xAxisPosition + context->barSpacing * index;
		if (index == 0)
			x += 1.5f / 2.0f;
		else if (index == bms->cellCount)
			x -= 1.5f / 2.0f;

		if (index % (bms->cellsPerLtc * bms->ltcsPerSegment) == 0)
		{
			// cairo_move_to (cr, x, 0);
			// cairo_line_to (cr, x, 10);

			cairo_move_to (cr, x, context->yAxisPosition + 8);
			cairo_line_to (cr, x, context->yAxisPosition - 8);
		}
		else if (index % bms->cellsPerLtc == 0)
		{
			// cairo_move_to (cr, x, 0);
			// cairo_line_to (cr, x, 5);

			cairo_move_to (cr, x, context->yAxisPosition + 4);
			cairo_line_to (cr, x, context->yAxisPosition - 4);
		}
		else
		{
			// cairo_move_to (cr, x, 0);
			// cairo_line_to (cr, x, 2.5);

			cairo_move_to (cr, x, context->yAxisPosition + 2);
			cairo_line_to (cr, x, context->yAxisPosition - 2);
		}

		if (index % (bms->cellsPerLtc * bms->ltcsPerSegment) == 0)
		{
			for (float y = context->yAxisPosition; y > 0; y -= context->tickSpacing / 2.0f)
			{
				cairo_move_to (cr, x - 2.5f, y);
				cairo_line_to (cr, x + 2.5f, y);
			}
		}
	}

	cairo_stroke (cr);

	char buffer [] = "SEGMENT #####";

	float segmentSpacing = context->barSpacing * bms->cellsPerLtc * bms->ltcsPerSegment;

	for (uint16_t index = 0; index < bms->segmentCount; ++index)
	{
		snprintf (buffer, sizeof (buffer), "SEGMENT %lu", (long unsigned) index);

		cairo_text_extents_t extents;
		cairo_text_extents (cr, buffer, &extents);

		cairo_move_to (cr, context->xAxisPosition + segmentSpacing * index + segmentSpacing / 2.0f - extents.width / 2.0f, context->canvasHeight);
		cairo_text_path (cr, buffer);
	}

	cairo_fill (cr);
}