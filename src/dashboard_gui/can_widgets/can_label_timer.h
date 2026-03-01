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
    struct timespec startTime;
    struct timespec currentTime;

    int height;
    int width;

    size_t signalIndex;

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

	struct timespec lastTime;
	struct timespec bestTime;

    canLabelTimerMode_t mode;

} canLabelTimer_t;

canWidget_t* canLabelTimerInit (canDatabase_t* database, canLabelTimerConfig_t* config);

canWidget_t* canLabelTimerLoad (canDatabase_t* database, cJSON* config);

void setMode (canLabelTimer_t* timer, char* mode);

#endif // CAN_LABEL_TIMER_H