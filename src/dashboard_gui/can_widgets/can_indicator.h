#ifndef CAN_WIDGETS_H
#define CAN_WIDGETS_H

// CAN Bus Indicator Widget ---------------------------------------------------------------------------------------------------
//
// Author: Cole Barach
// Date Created: 2025.01.02
//
// Description: GTK widget displaying the state of a CAN bus signal as an indicator. The signal is converted to a boolean using
//   a threshold value. If the signal value exceeds the threshold value (greater than for non-inverted, less than for
//   inverted), the indicator is considered active.

// Includes -------------------------------------------------------------------------------------------------------------------

// Includes
#include "can_widget_internal.h"
#include "can_database/can_database.h"
#include "cjson/cjson.h"

// GTK
#include <gtk/gtk.h>

// Datatypes ------------------------------------------------------------------------------------------------------------------

typedef enum
{
	/// @brief Indicator is a rectange.
	CAN_INDICATOR_RECT,

	/// @brief Indicator is a circle.
	CAN_INDICATOR_CIRCLE
} canIndicatorShape_t;

typedef struct
{
	/// @brief The font to use for the displayed text.
	char* font;

	/// @brief The shape to render the indicator as. Default rectangle.
	canIndicatorShape_t shape;

	/// @brief The color of the background when active. Default black.
	GdkRGBA bgActiveColor;

	/// @brief The color of the background when inactive. Default black.
	GdkRGBA bgInactiveColor;

	/// @brief The color of the background when invalid. Default black.
	GdkRGBA bgInvalidColor;

	/// @brief The color of the border when active. Default white.
	GdkRGBA borderActiveColor;

	/// @brief The color of the border when inactive. Default white.
	GdkRGBA borderInactiveColor;

	/// @brief The color of the border when invalid. Default white.
	GdkRGBA borderInvalidColor;

	/// @brief The color of the font when active. Default white.
	GdkRGBA fontActiveColor;

	/// @brief The color of the font when inactive. Default white.
	GdkRGBA fontInactiveColor;

	/// @brief The color of the font when invalid. Default white.
	GdkRGBA fontInvalidColor;

	/// @brief The amount of padding to use for the text (if any), in pixels. Default 0.
	int padding;

	/// @brief The radius of the shape's corners (if any), in pixels. Default 0.
	float cornerRadius;

	/// @brief The thickness of the shape's border, in pixels. Default 1.5.
	float borderThickness;

	/// @brief The amount of time, in seconds, of the interval to blink at. Use 0 for no blinking. Default 0.
	float blinkInterval;
} canIndicatorStyle_t;

/// @brief Configuration for the @c canIndicator_t widget.
typedef struct
{
	/// @brief The name of the CAN database signal to bind to. Required.
	char* signalName;

	/// @brief The threshold to compare the signal against. Required.
	float threshold;

	/// @brief False => active if signal > threshold. True active if signal < threshold. Default false.
	bool inverted;

	/// @brief The text to display on the indicator, if any. Default none.
	char* text;

	/// @brief Default / minimum width of the widget. Default 0.
	int width;

	/// @brief Default / minimum height of the widget. Default 0.
	int height;

	/// @brief The style of the widget.
	canIndicatorStyle_t style;
} canIndicatorConfig_t;

// Functions ------------------------------------------------------------------------------------------------------------------

/**
 * @brief Creates and initializes a CAN indicator.
 * @param database The CAN database to bind to.
 * @param config The configuration to use. Note a copy is made, so temporary pointers are acceptable.
 * @return The created widget, if successful, @c NULL otherwise.
 */
canWidget_t* canIndicatorInit (canDatabase_t* database, canIndicatorConfig_t* config);

/**
 * @brief Creates and initializes a CAN indicator widget from a JSON configuration.
 * @param database The CAN database to bind to.
 * @param config The JSON configuration to load from.
 * @param parentStyle The parent style to inherit from, or @c NULL .
 * @return The created widget, if successful, @c NULL otherwise.
 */
canWidget_t* canIndicatorLoad (canDatabase_t* database, cJSON* config, canIndicatorStyle_t* parentStyle);

/**
 * @brief Loads a CAN indicator widget's style from a JSON configuration.
 * @param config JSON configuration to use.
 * @param style The style to load into.
 * @param parent The parent style to inherit from, or @c NULL .
 */
void canIndicatorLoadStyle (cJSON* config, canIndicatorStyle_t* style, canIndicatorStyle_t* parent);

#endif // CAN_WIDGETS_H