#ifndef CAN_LABEL_TIMER_H
#define CAN_LABEL_TIMER_H

// CAN Label Timer Widget -----------------------------------------------------------------------------------------------------
//
// Author: Owen DiBacco
// Date Created: 2026.03.13
//
// Description: GTK widget displaying a timer. The timer can be set to one of three modes:
//  1. Current Time - displays time elapsed since that last button press.
//  2. Last Time - displays the total time of the last interval.
//  3. Best Time - displays the total time of the best interval.

// Includes -------------------------------------------------------------------------------------------------------------------

#include "can_widget.h"
#include "../pages/page.h"
#include "can_database/can_database.h"

#include "time.h"

// GTK
#include <gtk/gtk.h>

typedef enum
{
	CURRENT_TIME,
	LAST_TIME,
	BEST_TIME
} canLabelTimerMode_t;

typedef struct
{
    pageStyle_t* baseStyle;

} canLabelTimerStyle_t;

typedef struct
{
    GdkRGBA backgroundColor;
    GdkRGBA borderColor;
    GdkRGBA fontColor;

    int height;
    int width;

    char* signalName;
    double threshold;

	// REVIEW(Barach): For now the stylization is probably fine, but we'll likely want to simplify in favor of using the page's
	//   style. Don't worry about this until we start overhauling most CAN widget styles, just wanted to put the note here now.

    char* font;
    float borderThickness;

    char** modes;
    size_t modeCount;

} canLabelTimerConfig_t;

typedef struct
{
	canWidgetVmt_t vmt;
	canLabelTimerConfig_t config;
	canDatabase_t* database;
	GtkWidget* overlay;
    GtkWidget* timer;
    GtkWidget* area;

    size_t signalIndex;

    struct timespec startTime;
    struct timespec currentTime;

	struct timespec lastTime;
	struct timespec bestTime;

    canLabelTimerMode_t mode;

    bool running;
	bool buttonPressed;

    char* formattedTime;

} canLabelTimer_t;

canWidget_t* canLabelTimerInit (canDatabase_t* database, canLabelTimerConfig_t* config);

canWidget_t* canLabelTimerLoad (canDatabase_t* database, cJSON* config);

size_t canLabelTimerSetMode (void* widget, char* mode);

void canLabelTimerTrySetFont (void* widget, char* font);

#endif // CAN_LABEL_TIMER_H