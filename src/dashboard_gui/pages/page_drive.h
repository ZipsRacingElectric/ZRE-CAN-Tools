#ifndef PAGE_DRIVE_H
#define PAGE_DRIVE_H

// Driving Page ---------------------------------------------------------------------------------------------------------------
//
// Author: Cole Barach
// Date Created: 2025.12.16
//
// Description: Page intended to be used while driving the vehicle.

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

	GdkRGBA appsColor;
	GdkRGBA bseColor;

	GdkRGBA decalTickColorLight;
	GdkRGBA decalTickColorDark;
	float decalUpperTickSpacing;
	float decalUpperTickMargin;
	float decalCenterTickSpacing;
	float decalCenterTickMargin;
	float decalHeight;
	float decalThickness;
	float decalWidth;

	GdkRGBA faultActiveColor;
	GdkRGBA faultInactiveColor;

	GdkRGBA gradientStartColor;
	float gradientStartPosition;
	GdkRGBA gradientMiddleColor;
	float gradientMiddlePosition;
	GdkRGBA gradientEndColor;
	float gradientEndPosition;

	char* dataLoggerTitleFont;
	char* dataLoggerStatFont;
	char* centerPanelTitleFont;
	char* centerPanelLabelFont;
	char* centerPanelStatFont;
	char* sidePanelTitleFont;
	char* sidePanelStatFont;
	char* faultIndicatorFont;
} pageDriveStyle_t;

typedef struct
{
	pageVmt_t vmt;

	pageDriveStyle_t style;

	GtkGrid* grid;
	GtkGrid* dataLoggerPanel;
	GtkWidget* leftPanel;
	GtkWidget* rightPanel;
	GtkWidget* centerPanelTitle;
	GtkWidget* centerPanel;
	GtkGrid* faultPanel;
	GtkGrid* buttonPanel;
	size_t buttonCount;

	canWidget_t** leftPanelWidgets;
	size_t leftPanelWidgetCount;

	canWidget_t** rightPanelWidgets;
	size_t rightPanelWidgetCount;

	canWidget_t** centerPanelWidgets;
	size_t centerPanelWidgetCount;

	canWidget_t* bse;
	canWidget_t* apps;
	canWidget_t* dataLoggerTitle;
	canWidget_t* dataLoggerStat;
	canWidget_t* vcuFault;
	canWidget_t* bmsFault;
	canWidget_t* amkFault;
	canWidget_t* gpsFault;
} pageDrive_t;

// Functions ------------------------------------------------------------------------------------------------------------------

page_t* pageDriveLoad (cJSON* config, canDatabase_t* database, pageStyle_t* style);

#endif // PAGE_DRIVE_H