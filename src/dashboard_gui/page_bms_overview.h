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
	bmsBarGraph_t voltages;
	bmsBarGraph_t temperatures;
	canLabelFloat_t* statusLabels;
} pageBmsOverview_t;

// Functions ------------------------------------------------------------------------------------------------------------------

page_t* pageBmsOverviewInit (canDatabase_t* database, bms_t* bms);

void pageBmsOverviewUpdate (void* page);

#endif // PAGE_BMS_OVERVIEW_H