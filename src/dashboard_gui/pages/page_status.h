#ifndef PAGE_STATUS_H
#define PAGE_STATUS_H

// Status Page ----------------------------------------------------------------------------------------------------------------
//
// Author: ...
// Date Created: 2026.02.09
//
// Description: TODO

// Includes -------------------------------------------------------------------------------------------------------------------

// Includes
#include "page.h"
#include "../can_widgets/can_widget.h"
#include "can_database/can_database.h"
#include "cjson/cjson.h"

// GTK
#include <gtk/gtk.h>

// Datatypes ------------------------------------------------------------------------------------------------------------------

typedef struct
{
	pageStyle_t* baseStyle;
	GdkRGBA shutdownLineColor;
	float shutdownLineThickness;
	float shutdownLineSlope;
} pageStatusStyle_t;

typedef struct
{
	pageVmt_t vmt;

	pageStatusStyle_t style;

	GtkGrid* buttonPanel;
	size_t buttonCount;

	GtkGrid* grid;

	canWidget_t** shutdownIndicators;
	size_t shutdownIndicatorCount;

	canWidget_t* positiveIr;
	canWidget_t* negativeIr;

	canWidget_t** widgets;
	size_t widgetCount;

	canWidget_t** diagramIndicators;
	size_t diagramIndicatorCount;
} pageStatus_t;

// Functions ------------------------------------------------------------------------------------------------------------------

page_t* pageStatusLoad (cJSON* config, canDatabase_t* database, pageStyle_t* style);

#endif // PAGE_STATUS_H