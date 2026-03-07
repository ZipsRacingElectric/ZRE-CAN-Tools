#ifndef CAN_LABEL_TIMER_H
#define CAN_LABEL_TIMER_H

#include "time_port.h"
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
    int height;
    int width;

    size_t signalIndex;

	// REVIEW(Barach): For now the stylization is probably fine, but we'll likely want to simplify in favor of using the page's
	//   style. Don't worry about this until we start overhauling most CAN widget styles, just wanted to put the note here now.

    float borderThickness;

    GdkRGBA backgroundColor;
    GdkRGBA borderColor;
    GdkRGBA fontColor;

} canLabelTimerConfig_t;

typedef struct
{
	canWidgetVmt_t vmt;
	canLabelTimerConfig_t config;
	canDatabase_t* database;
	GtkWidget* overlay;
    GtkWidget* timer;
    GtkWidget* area;

	bool running;
	bool buttonPressed;

    struct timespec startTime;
    struct timespec currentTime;

	struct timespec lastTime;
	struct timespec bestTime;

    canLabelTimerMode_t mode;

} canLabelTimer_t;

canWidget_t* canLabelTimerInit (canDatabase_t* database, canLabelTimerConfig_t* config);

canWidget_t* canLabelTimerLoad (canDatabase_t* database, cJSON* config);

// REVIEW(Barach): Either this should have the same prefix as the other functions ("canLabelTimerSetMode") or it should be
//   made static and not included in the header. Static is probably best, as this string should come from the config JSON.
void setMode (void* widget, char* mode);

#endif // CAN_LABEL_TIMER_H