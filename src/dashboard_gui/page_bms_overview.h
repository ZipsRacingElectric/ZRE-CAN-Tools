#ifndef PAGE_BMS_OVERVIEW_H
#define PAGE_BMS_OVERVIEW_H

// BMS Overview Page ----------------------------------------------------------------------------------------------------------
//
// Author: Cole Barach
// Date Created: 2025.12.29
//
// Description: TODO(Barach)

// Includes -------------------------------------------------------------------------------------------------------------------

// Includes
#include "page.h"
#include "bms_widgets.h"
#include "can_widgets.h"
#include "bms/bms.h"

// Datatypes ------------------------------------------------------------------------------------------------------------------

typedef struct
{
	pageVmt_t vmt;
	bms_t* bms;
	bmsBarGraph_t voltages;
	bmsBarGraph_t temperatures;
	bmsBarGraph_t ltcTemperatures;
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
	canLabelFloat_t* statusLabels;
	size_t statusCount;
} pageBmsOverview_t;

// Functions ------------------------------------------------------------------------------------------------------------------

page_t* pageBmsOverviewInit (canDatabase_t* database, bms_t* bms);

void pageBmsOverviewUpdate (void* page);

#endif // PAGE_BMS_OVERVIEW_H