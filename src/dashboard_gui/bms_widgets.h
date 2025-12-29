#ifndef BMS_WIDGETS_H
#define BMS_WIDGETS_H

// BMS Widgets ----------------------------------------------------------------------------------------------------------------
//
// Author: Cole Barach
// Date Created: 2025.12.29
//
// Description: TODO(Barach)

// Includes -------------------------------------------------------------------------------------------------------------------

// Includes
#include "bms/bms.h"
#include "zre_math.h"

// GTK
#include <gtk/gtk.h>

// Datatypes ------------------------------------------------------------------------------------------------------------------

typedef canDatabaseSignalState_t (bmsBarGraphAccessor_t) (bms_t* bms, size_t index, float* value);

typedef struct
{
	bmsBarGraphAccessor_t* accessor;
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
	GdkRGBA barColor;
	GdkRGBA tickColor;
	GdkRGBA axisColor;
} bmsBarGraphConfig_t;

typedef struct
{
	GtkWidget* widget;
	bms_t* bms;
	bmsBarGraphConfig_t config;
	float zero;
	float* tickScalars;
	char** tickLabels;
	size_t tickCount;
} bmsBarGraph_t;

#define BMS_BAR_GRAPH_TO_DRAWING_AREA(graph) GTK_DRAWING_AREA ((graph)->widget)
#define BMS_BAR_GRAPH_TO_WIDGET(graph) ((graph)->widget)

// Functions ------------------------------------------------------------------------------------------------------------------

void bmsBarGraphInit (bmsBarGraph_t* graph, bms_t* bms, bmsBarGraphConfig_t* config);

void bmsBarSetBounds (bmsBarGraph_t* graph, float min, float max);

void bmsBarGraphUpdate (bmsBarGraph_t* graph);

#endif // BMS_WIDGETS_H