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
#include "can_database/can_database.h"
#include "../can_widgets/can_label_timer.h"
#include "cjson/cjson.h"

// GTK
#include <gtk/gtk.h>

// Datatypes ------------------------------------------------------------------------------------------------------------------

typedef struct
{
	pageStyle_t* baseStyle;
} pageStatusStyle_t;

typedef struct
{
	pageVmt_t vmt;

	pageStatusStyle_t style;

	GtkGrid* grid;

	canWidget_t* timer1;
	canWidget_t* timer2;
	canWidget_t* timer3;

	GtkGrid* timerPanel;

	GtkGrid* buttonPanel;
	size_t buttonCount;

} pageStatus_t;

// Functions ------------------------------------------------------------------------------------------------------------------

page_t* pageStatusLoad (cJSON* config, canDatabase_t* database, pageStyle_t* style);

#endif // PAGE_STATUS_H