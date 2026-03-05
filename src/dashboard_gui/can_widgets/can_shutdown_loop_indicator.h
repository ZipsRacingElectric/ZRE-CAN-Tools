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
#include "can_widget_internal.h"
#include "can_database/can_database.h"
#include "cjson/cjson.h"

// GTK
#include <gtk/gtk.h>

// Datatypes ------------------------------------------------------------------------------------------------------------------

typedef struct
{
	/// @brief The position of the text on the socket, in pixels. Default 0.
	float textPosition;

	/// @brief The size of the text's font, in pixels. Default 12.
	float fontSize;

	/// @brief The x position of the socket when open, in pixels. Default 10.
	float openPosition;

	/// @brief The x position of the socket when closed, in pixels. Default 20.
	float closedPosition;

	/// @brief The length of the socket, in pixels. Default 80.
	float socketLength;

	/// @brief The size (height) of the socket's insert, in pixels. Default 5.
	float socketSize;

	/// @brief The depth of the socket's insert, in pixels. Default 5.
	float socketDepth;

	/// @brief The thickness (height) of the socket, in pixels. Default 20.
	float plugThickness;

	/// @brief The thickness of the line connecting to the socket/plug, in pixels. Default 1.5.
	float lineThickness;

	/// @brief The height of the line's fold, in pixels. Default 20.
	float peakHeight;

	/// @brief The color of the line. Default gray.
	GdkRGBA lineColor;

	/// @brief The color of the plug/socket. Default gray.
	GdkRGBA plugColor;

	/// @brief The color of the text. Default black.
	GdkRGBA textColor;

	/// @brief The color of the plug/socket border. Default white.
	GdkRGBA borderColor;

	/// @brief The thickness of the plug/socket border, in pixels. Default 1.5
	float borderThickness;

	/// @brief The amount of time, roughly in seconds, the animation takes to open/close.
	float animationTime;

	/// @brief The delay, roughly in seconds, the animation takes to start.
	float animationDelay;
} canShutdownLoopIndicatorStyle_t;

typedef struct
{
	/// @brief The name of the database signal to bind to. Required.
	char* signalName;

	/// @brief The activation threshold of the signal. Required.
	float threshold;

	/// @brief Inverts signal activation. Default false.
	bool inverted;

	/// @brief The text to label the socket with. Default none.
	char* text;

	/// @brief The minimum width of the widget, in pixels. Default 0.
	int width;

	/// @brief The minimum width of the widget, in pixels. Default 0.
	int height;

	/// @brief The style of the widget.
	canShutdownLoopIndicatorStyle_t style;
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
 * @param parentStyle The parent style to inherit from, or @c NULL .
 * @return The created widget, if successful, @c NULL otherwise.
 */
canWidget_t* canShutdownLoopIndicatorLoad (canDatabase_t* database, cJSON* config, canShutdownLoopIndicatorStyle_t* parentStyle);

/**
 * @brief Loads a CAN shutdown loop indicator's style from a JSON configuration.
 * @param config JSON configuration to use.
 * @param style The style to load into.
 * @param parent The parent style to inherit from, or @c NULL .
 */
void canShutdownLoopIndicatorLoadStyle (cJSON* config, canShutdownLoopIndicatorStyle_t* style, canShutdownLoopIndicatorStyle_t* parent);

#endif // CAN_SHUTDOWN_LOOP_INDICATOR_H