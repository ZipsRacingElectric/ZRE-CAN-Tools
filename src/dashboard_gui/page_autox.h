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

// GTK
#include <gtk/gtk.h>

// Datatypes ------------------------------------------------------------------------------------------------------------------

// TODO(Barach): Polymorphism docs

typedef struct
{
	pageVmt_t vmt;

	canWidget_t* bse;
	canWidget_t* apps;
	canWidget_t* dataLoggerStatus;
	canWidget_t* dataLoggerSession;
	canWidget_t* glvVoltage;
	canWidget_t* hvVoltage;
	canWidget_t* inverterMaxTemp;
	canWidget_t* motorMaxTemp;
	canWidget_t* drivingTorque;
	canWidget_t* regenTorque;
	canWidget_t* torqueIndex;
	canWidget_t* speed;
	canWidget_t* drsStatus;
	canWidget_t* vcuFault;
	canWidget_t* bmsFault;
	canWidget_t* amkFault;
	canWidget_t* gpsFault;
} pageAutox_t;

#define PAGE_AUTOX_TO_WIDGET(page) ((page)->widget)

// Functions ------------------------------------------------------------------------------------------------------------------

page_t* pageAutoxInit (canDatabase_t* database);

void pageAutoxAppendButton (void* page, const char* label, pageButtonCallback_t* callback, void* arg);

void pageAutoxUpdate (void* page);

#endif // PAGE_AUTOX_H