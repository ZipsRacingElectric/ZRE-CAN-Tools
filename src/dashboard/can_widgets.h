#ifndef CAN_WIDGETS_H
#define CAN_WIDGETS_H

// CAN Bus GTK Widgets --------------------------------------------------------------------------------------------------------
//
// Author: Cole Barach
// Date Created: 2025.12.16
//
// Description: TODO(Barach)

// Includes -------------------------------------------------------------------------------------------------------------------

// Includes
#include "can_database/can_database.h"

// GTK
#include <gtk/gtk.h>

// Datatypes ------------------------------------------------------------------------------------------------------------------

typedef struct
{
	GtkWidget* widget;
	const char* signalName;
	const char* formatValue;
	const char* formatInvalid;
	canDatabase_t* database;
	ssize_t index;
} canLabel_t;

#define CAN_LABEL_TO_LABEL(label) GTK_LABEL ((label)->widget)
#define CAN_LABEL_TO_WIDGET(label) ((label)->widget)

typedef struct
{
	GtkWidget* widget;
	const char* signalName;
	float min;
	float max;
	canDatabase_t* database;
	ssize_t index;
} canProgressBar_t;

#define CAN_PROGRESS_BAR_TO_PROGRESS_BAR(label) GTK_PROGRESS_BAR ((label)->widget)
#define CAN_PROGRESS_BAR_TO_WIDGET(label) ((label)->widget)

// Functions ------------------------------------------------------------------------------------------------------------------

void canLabelInit (canLabel_t* label, canDatabase_t* database);

void canLabelUpdate (canLabel_t* label);

void canProgessBarInit (canProgressBar_t* bar, canDatabase_t* database);

void canProgressBarUpdate (canProgressBar_t* bar);

#endif // CAN_WIDGETS_H