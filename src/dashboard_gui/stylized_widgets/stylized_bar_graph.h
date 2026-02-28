#ifndef STYLIZED_BAR_GRAPH_H
#define STYLIZED_BAR_GRAPH_H

// Stylized Bar Graph Widget --------------------------------------------------------------------------------------------------
//
// Author: Cole Barach
// Date Created: 2025.12.29
//
// Description: An implementation of a stylized bar graph.

// Includes -------------------------------------------------------------------------------------------------------------------

// GTK
#include <gtk/gtk.h>

// Datatypes ------------------------------------------------------------------------------------------------------------------

typedef struct
{
	int canvasWidth;
	int canvasHeight;
	int graphWidth;
	int graphHeight;
	float xAxisPosition;
	float yAxisPosition;
	float tickSpacing;
	float barSize;
	float barSpacing;
} stylizedBarGraphDrawContext_t;

// Forward declaration
struct stylizedBarGraph;
typedef struct stylizedBarGraph stylizedBarGraph_t;

typedef bool (stylizedBarGraphAccessor_t) (void* arg, size_t index, float* value);

typedef void (stylizedBarGraphDraw_t) (stylizedBarGraph_t* graph, cairo_t* cr, stylizedBarGraphDrawContext_t* context);

typedef struct
{
	stylizedBarGraphAccessor_t* accessor;
	stylizedBarGraphDraw_t* backgroundDraw;
	stylizedBarGraphDraw_t* foregroundDraw;
	size_t offset;
	size_t count;
	float barSize;
	float barSpacing;
	int length;
	float min;
	float max;
	float tickSpacing;
	const char* tickFormat;
	float axisPosition;
	GdkRGBA validBarColor;
	GdkRGBA invalidBarColor;
	GdkRGBA tickColor;
	GdkRGBA axisColor;
} stylizedBarGraphConfig_t;

struct stylizedBarGraph
{
	GtkWidget* widget;
	void* accessorArg;
	stylizedBarGraphConfig_t config;
	float zero;
	char** tickLabels;
	size_t tickCount;
	float tickScalar;
};

#define STYLIZED_BAR_GRAPH_TO_WIDGET(graph) ((graph)->widget)

// Functions ------------------------------------------------------------------------------------------------------------------

void stylizedBarGraphInit (stylizedBarGraph_t* graph, stylizedBarGraphConfig_t* config, void* accessorArg);

void stylizedBarGraphSetBounds (stylizedBarGraph_t* graph, float min, float max);

void stylizedBarGraphUpdate (stylizedBarGraph_t* graph);

#endif // STYLIZED_BAR_GRAPH_H