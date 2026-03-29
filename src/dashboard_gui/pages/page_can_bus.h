#ifndef PAGE_CAN_BUS_H
#define PAGE_CAN_BUS_H

// CAN Bus Page ---------------------------------------------------------------------------------------------------------------
//
// Author: Cole Barach
// Date Created: 2026.01.06
//
// Description: Page detailing all information in a CAN database. Used for debugging.

// Includes -------------------------------------------------------------------------------------------------------------------

// Includes
#include "page.h"
#include "../stylized_widgets/stylized_button.h"
#include "../stylized_widgets/stylized_terminal.h"
#include "can_database/can_database.h"

// GTK
#include <gtk/gtk.h>

// Datatypes ------------------------------------------------------------------------------------------------------------------

typedef struct
{
	pageStyle_t* baseStyle;

	GdkRGBA terminalBackgroundColor;
	float animationTime;

	float decalSlope;
	int decalCount;
} pageCanBusStyle_t;

typedef struct
{
	pageVmt_t vmt;
	pageCanBusStyle_t style;

	canDatabase_t* databases;
	size_t databaseCount;

	GtkWidget* grid;
	GtkWidget* buttonPanel;
	size_t buttonCount;

	stylizedTerminal_t* term;
	stylizedButton_t* dbcButton;

	size_t dbcIndex;

	float animationTimer;
} pageCanBus_t;

// Functions ------------------------------------------------------------------------------------------------------------------

page_t* pageCanBusLoad (cJSON* config, canDatabase_t* databases, size_t databaseCount, pageStyle_t* style);

#endif // PAGE_CAN_BUS_H