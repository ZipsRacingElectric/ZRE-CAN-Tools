#ifndef PAGE_BMS_H
#define PAGE_BMS_H

// BMS Page -------------------------------------------------------------------------------------------------------------------
//
// Author: Cole Barach
// Date Created: 2025.12.29
//
// Description: TODO(Barach)

// Includes -------------------------------------------------------------------------------------------------------------------

// Includes
#include "page.h"
#include "../bms_widgets.h"
#include "bms/bms.h"

// Datatypes ------------------------------------------------------------------------------------------------------------------

typedef struct
{
	pageStyle_t* baseStyle;

	char* cellVoltageGraphTitleFont;
	GdkRGBA cellVoltageGraphTitleColor;
	GdkRGBA cellVoltageGraphBarColor;
	GdkRGBA cellVoltageGraphTickColor;
	GdkRGBA cellVoltageGraphAxisColor;

	char* senseLineTemperatureGraphTitleFont;
	GdkRGBA senseLineTemperatureGraphTitleColor;
	GdkRGBA senseLineTemperatureGraphBarColor;
	GdkRGBA senseLineTemperatureGraphTickColor;
	GdkRGBA senseLineTemperatureGraphAxisColor;

	char* ltcTemperatureGraphTitleFont;
	GdkRGBA ltcTemperatureGraphTitleColor;
	GdkRGBA ltcTemperatureGraphBarColor;
	GdkRGBA ltcTemperatureGraphTickColor;
	GdkRGBA ltcTemperatureGraphAxisColor;
} pageBmsStyle_t;

typedef struct
{
	pageVmt_t vmt;

	GtkGrid* buttonPanel;
	size_t buttonCount;

	pageBmsStyle_t style;

	bms_t bms;
	bmsBarGraph_t voltages;
	bmsBarGraph_t temperatures;
	bmsBarGraph_t ltcTemperatures;
	bmsFaultLabel_t faultLabel;
	canLabelFloatStatic_t voltageLabel;
	canLabelFloatStatic_t currentLabel;
	canLabelFloatStatic_t powerLabel;
	canLabelFloatStatic_t minCellLabel;
	canLabelFloatStatic_t maxCellLabel;
	canLabelFloatStatic_t avgCellLabel;
	canLabelFloatStatic_t minTempLabel;
	canLabelFloatStatic_t maxTempLabel;
	canLabelFloatStatic_t avgTempLabel;
	canLabelFloatStatic_t maxDeltaLabel;
	canLabelFloatStatic_t avgDeltaLabel;
} pageBms_t;

// Functions ------------------------------------------------------------------------------------------------------------------

page_t* pageBmsLoad (cJSON* config, canDatabase_t* database, pageStyle_t* style);

#endif // PAGE_BMS_H