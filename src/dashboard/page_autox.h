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
#include "can_widgets.h"

// GTK
#include <gtk/gtk.h>

// Datatypes ------------------------------------------------------------------------------------------------------------------

typedef struct
{
	GtkWidget* widget;
	canProgressBar_t bse;
	canProgressBar_t apps;
	canLabel_t glvVoltage;
	canLabel_t hvVoltage;
	canLabel_t inverterMaxTemp;
	canLabel_t motorMaxTemp;
	canLabel_t drivingTorque;
	canLabel_t regenTorque;
	canLabel_t torqueIndex;
	canLabel_t drsStatus;
	canLabel_t speed;
} pageAutox_t;

#define PAGE_AUTOX_TO_WIDGET(page) ((page)->widget)

// Functions ------------------------------------------------------------------------------------------------------------------

void pageAutoxInit (pageAutox_t* page, canDatabase_t* database);

void pageAutoxUpdate (pageAutox_t* page);

#endif // PAGE_AUTOX_H