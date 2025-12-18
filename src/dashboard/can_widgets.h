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
} canLabelFloat_t;

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

#define CAN_PROGRESS_BAR_TO_PROGRESS_BAR(bar) GTK_PROGRESS_BAR ((bar)->widget)
#define CAN_PROGRESS_BAR_TO_WIDGET(bar) ((bar)->widget)

typedef enum
{
	CAN_INDICATOR_INVALID,
	CAN_INDICATOR_ACTIVE,
	CAN_INDICATOR_INACTIVE
} canIndicatorState_t;

typedef struct
{
	GtkWidget* widget;
	canDatabase_t* database;
	ssize_t index;
	const char* signalName;
	float threshold;
	bool inverted;
	canIndicatorState_t state;
	float (*points) [2];
	size_t pointCount;
	// TODO(Barach): Hex parser
	GdkRGBA activeColor;
	GdkRGBA inactiveColor;
	GdkRGBA invalidColor;
} canIndicator_t;

#define CAN_INDICATOR_TO_DRAWING_AREA(indicator) GTK_DRAWING_AREA ((indicator)->widget)
#define CAN_INDICATOR_TO_WIDGET(indicator) ((indicator)->widget)

// Functions ------------------------------------------------------------------------------------------------------------------

void canLabelFloatInit (canLabelFloat_t* label, canDatabase_t* database);

void canLabelFloatUpdate (canLabelFloat_t* label);

void canProgessBarInit (canProgressBar_t* bar, canDatabase_t* database);

void canProgressBarUpdate (canProgressBar_t* bar);

void canIndicatorInit (canIndicator_t* indicator, canDatabase_t* database, int width, int height);

void canIndicatorUpdate (canIndicator_t* indicator);

#endif // CAN_WIDGETS_H