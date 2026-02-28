#ifndef SENSE_LINE_TEMPERATURE_GRAPH_H
#define SENSE_LINE_TEMPERATURE_GRAPH_H

// Includes
#include "../stylized_widgets/stylized_bar_graph.h"

bool bmsSenseLineTemperatureGraphAccessor (void* arg, size_t index, float* value);

void bmsSenseLineTemperatureGraphDrawForeground (stylizedBarGraph_t* graph, cairo_t* cr, stylizedBarGraphDrawContext_t* context);

#endif // SENSE_LINE_TEMPERATURE_GRAPH_H