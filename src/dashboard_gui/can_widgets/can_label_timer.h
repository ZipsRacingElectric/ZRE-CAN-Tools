#ifndef CAN_LABEL_TIMER_H
#define CAN_LABEL_TIMER_H

#include "time_port.h"
#include "can_widget.h"
#include "../pages/page.h"
#include "can_database/can_database.h"

#include "time.h"

// GTK
#include <gtk/gtk.h>

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

canWidget_t* canLabelTimerInit (canDatabase_t* database, canLabelTimerConfig_t* config);

#endif // CAN_LABEL_TIMER_H