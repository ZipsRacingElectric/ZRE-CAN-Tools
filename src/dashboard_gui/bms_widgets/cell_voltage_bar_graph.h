#ifndef CELL_VOLTAGE_BAR_GRAPH_H
#define CELL_VOLTAGE_BAR_GRAPH_H

// Includes
#include "../stylized_widgets/stylized_bar_graph.h"

bool bmsCellVoltageBarGraphAccessor (void* arg, size_t index, float* value);

void bmsCellVoltageBarGraphDrawForeground (stylizedBarGraph_t* graph, cairo_t* cr, stylizedBarGraphDrawContext_t* context);

#endif // CELL_VOLTAGE_BAR_GRAPH_H