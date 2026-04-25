#ifndef LTC_TEMPERATURE_GRAPH_H
#define LTC_TEMPERATURE_GRAPH_H

// Includes
#include "../stylized_widgets/stylized_bar_graph.h"

stylizedBarGraphAccessorReturn_t bmsLtcTemperatureGraphAccessor (void* arg, size_t index, float* value);

void bmsLtcTemperatureGraphDrawForeground (stylizedBarGraph_t* graph, cairo_t* cr, stylizedBarGraphDrawContext_t* context);

#endif // LTC_TEMPERATURE_GRAPH_H