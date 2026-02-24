#ifndef CAN_SHUTDOWN_LOOP_INDICATOR_H
#define CAN_SHUTDOWN_LOOP_INDICATOR_H

// CAN Database Shutdown Loop Indicator Widget --------------------------------------------------------------------------------
//
// Author: Cole Barach
// Date Created: 2026.02.23
//
// Description: GTK widget indicating the status of a point in the shutdown loop.

// Includes -------------------------------------------------------------------------------------------------------------------

// Includes
#include "can_widget.h"
#include "can_database/can_database.h"

// GTK
#include <gtk/gtk.h>

// Datatypes ------------------------------------------------------------------------------------------------------------------

// TODO(Barach): Need to clean this up majorly

typedef struct
{
	int width;
	int height;
	int openPosition;
	int closedPosition;
	int plugLength;
	int socketSize;
	int socketDepth;
	int plugThickness;
	int lineThickness;
	int peakHeight;
	GdkRGBA lineColor;
	GdkRGBA plugColor;
	GdkRGBA textColor;
	GdkRGBA borderColor;
	float borderThickness;
	float threshold;
	bool inverted;
	char* signal;
	char* text;
	float fontSize;
	int textOffset;
	float animationTime;
	float animationDelay;
} canShutdownLoopIndicatorConfig_t;

// Functions ------------------------------------------------------------------------------------------------------------------

/**
 * @brief Creates and initializes a shutdown loop indicator CAN widget.
 * @param database The CAN database to bind to.
 * @param config The configuration to use. Note a copy is made, so temporary pointers are acceptable.
 * @return The created widget, if successful, @c NULL otherwise.
 */
canWidget_t* canShutdownLoopIndicatorInit (canDatabase_t* database, canShutdownLoopIndicatorConfig_t* config);

/**
 * @brief Creates and initializes a shutdown loop indicator CAN widget from a JSON configuration.
 * @param database The CAN database to bind to.
 * @param config The JSON configuration to load from.
 * @return The created widget, if successful, @c NULL otherwise.
 */
canWidget_t* canShutdownLoopIndicatorLoad (canDatabase_t* database, cJSON* config);

#endif // CAN_SHUTDOWN_LOOP_INDICATOR_H