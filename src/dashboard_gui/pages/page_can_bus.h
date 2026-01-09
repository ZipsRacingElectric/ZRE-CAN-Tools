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
#include "../stylized_widgets/stylized_terminal.h"
#include "can_database/can_database.h"

// GTK
#include <gtk/gtk.h>

// Datatypes ------------------------------------------------------------------------------------------------------------------

typedef struct
{
	pageStyle_t* baseStyle;
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

page_t* pageCanBusLoad (cJSON* config, canDatabase_t* database, pageStyle_t* style);

#endif // PAGE_CAN_BUS_H