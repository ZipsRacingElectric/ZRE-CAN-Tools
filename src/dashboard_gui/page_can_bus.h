#ifndef PAGE_CAN_BUS_H
#define PAGE_CAN_BUS_H

// CAN Bus Page ---------------------------------------------------------------------------------------------------------------
//
// Author: Cole Barach
// Date Created: 2026.01.06
//
// Description: TODO(Barach)

// Includes -------------------------------------------------------------------------------------------------------------------

// Includes
#include "page.h"
#include "can_database/can_database.h"
#include "stylized_widgets/stylized_terminal.h"

// GTK
#include <gtk/gtk.h>

// Datatypes ------------------------------------------------------------------------------------------------------------------

typedef struct
{
	pageStyle_t* pageStyle;
} pageCanBusStyle_t;

typedef struct
{
	pageVmt_t vmt;
	pageCanBusStyle_t style;

	canDatabase_t* database;

	GtkWidget* grid;
	GtkWidget* buttonPanel;
	size_t buttonCount;

	stylizedTerminal_t* term;
} pageCanBus_t;

// Functions ------------------------------------------------------------------------------------------------------------------

page_t* pageCanBusInit (canDatabase_t* database, pageStyle_t* style);

void pageCanBusAppendButton (void* pageArg, const char* label, pageButtonCallback_t* callback, void* arg, bool currentPage);

void pageCanBusUpdate (void* pageArg);

#endif // PAGE_CAN_BUS_H