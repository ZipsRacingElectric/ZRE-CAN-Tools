#ifndef PAGE_AUTOX_H
#define PAGE_AUTOX_H

// Autocross Page -------------------------------------------------------------------------------------------------------------
//
// Author: Cole Barach
// Date Created: 2025.12.16
//
// Description: TODO(Barach)

// Includes -------------------------------------------------------------------------------------------------------------------

// Includes
#include "page.h"
#include "can_widgets/can_widget.h"
#include "can_database/can_database.h"
#include "cjson/cjson.h"

// GTK
#include <gtk/gtk.h>

// Datatypes ------------------------------------------------------------------------------------------------------------------

// TODO(Barach): Polymorphism docs

typedef struct
{
	pageStyle_t pageStyle;

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
	char* centerPanelStatFont;
	char* leftPanelTitleFont;
	char* leftPanelStatFont;
	char* rightPanelTitleFont;
	char* rightPanelStatFont;
	char* faultIndicatorFont;
} pageAutoxStyle_t;

typedef struct
{
	pageVmt_t vmt;

	pageAutoxStyle_t style;

	GtkGrid* grid;
	GtkGrid* dataLoggerPanel;
	GtkGrid* leftPanel;
	GtkGrid* rightPanel;
	GtkWidget* centerTitle;
	GtkGrid* faultPanel;
	GtkGrid* buttonPanel;
	size_t buttonCount;

	canWidget_t* bse;
	canWidget_t* apps;
	canWidget_t* dataLoggerTitle;
	canWidget_t* dataLoggerStat;
	canWidget_t* glvVoltage;
	canWidget_t* hvVoltage;
	canWidget_t* inverterMaxTemp;
	canWidget_t* motorMaxTemp;
	canWidget_t* drivingTorque;
	canWidget_t* regenTorque;
	canWidget_t* torqueIndex;
	canWidget_t* centerStat;
	canWidget_t* drsStatus;
	canWidget_t* vcuFault;
	canWidget_t* bmsFault;
	canWidget_t* amkFault;
	canWidget_t* gpsFault;
} pageAutox_t;

// Functions ------------------------------------------------------------------------------------------------------------------

page_t* pageAutoxInit (canDatabase_t* database, pageStyle_t* style, cJSON* config);

void pageAutoxAppendButton (void* pageArg, const char* label, pageButtonCallback_t* callback, void* arg, bool currentPage);

void pageAutoxUpdate (void* pageArg);

#endif // PAGE_AUTOX_H