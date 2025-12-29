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
#include "page.h"

// GTK
#include <gtk/gtk.h>

// Datatypes ------------------------------------------------------------------------------------------------------------------

// TODO(Barach): Polymorphism docs

typedef struct
{
	pageVmt_t vmt;

	canProgressBar_t bse;
	canProgressBar_t apps;
	canLabelBool_t dataLoggerStatus;
	canLabelFloat_t dataLoggerSession;
	canLabelFloat_t glvVoltage;
	canLabelFloat_t hvVoltage;
	canLabelFloat_t inverterMaxTemp;
	canLabelFloat_t motorMaxTemp;
	canLabelFloat_t drivingTorque;
	canLabelFloat_t regenTorque;
	canLabelFloat_t torqueIndex;
	canLabelFloat_t speed;
	canIndicator_t drsStatus;
	canIndicator_t vcuFault;
	canIndicator_t bmsFault;
	canIndicator_t amkFault;
	canIndicator_t gpsFault;
} pageAutox_t;

#define PAGE_AUTOX_TO_WIDGET(page) ((page)->widget)

// Functions ------------------------------------------------------------------------------------------------------------------

page_t* pageAutoxInit (canDatabase_t* database);

void pageAutoxUpdate (void* page);

#endif // PAGE_AUTOX_H