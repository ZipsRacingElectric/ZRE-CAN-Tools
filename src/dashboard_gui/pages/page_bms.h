#ifndef PAGE_BMS_H
#define PAGE_BMS_H

// BMS Page -------------------------------------------------------------------------------------------------------------------
//
// Author: Cole Barach
// Date Created: 2025.12.29
//
// Description: Page displaying detailed information about a battery management system.

// Includes -------------------------------------------------------------------------------------------------------------------

// Includes
#include "page.h"
#include "../bms_widgets.h"
#include "../stylized_widgets/stylized_terminal.h"
#include "bms/bms.h"
#include "can_node/fault_signal.h"

// Datatypes ------------------------------------------------------------------------------------------------------------------

typedef struct
{
	pageStyle_t* baseStyle;

	GdkRGBA terminalBackgroundColor;

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

	stylizedTerminal_t* term;

	bms_t bms;
	bmsBarGraph_t voltages;
	bmsBarGraph_t temperatures;
	bmsBarGraph_t ltcTemperatures;
	faultSignals_t faults;
} pageBms_t;

// Functions ------------------------------------------------------------------------------------------------------------------

page_t* pageBmsLoad (cJSON* config, canDatabase_t* database, pageStyle_t* style);

#endif // PAGE_BMS_H