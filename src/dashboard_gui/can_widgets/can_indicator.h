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
#include "can_widget.h"
#include "can_database/can_database.h"

// GTK
#include <gtk/gtk.h>

// Datatypes ------------------------------------------------------------------------------------------------------------------

typedef enum
{
	/// @brief Indicator is a circle.
	CAN_INDICATOR_CIRCLE,

	/// @brief Indicator is a rectange.
	CAN_INDICATOR_RECT
} canIndicatorShape_t;

/// @brief Configuration for the @c canIndicator_t widget.
typedef struct
{
	/// @brief The name of the CAN database signal to bind to.
	const char* signalName;

	/// @brief The threshold to compare the signal against.
	float threshold;

	/// @brief False => active if signal > threshold. True active if signal < threshold.
	bool inverted;

	/// @brief The text to display on the indicator, if any.
	char* text;

	/// @brief The font to use for the displayed text, if any.
	char* font;

	/// @brief The color of the background when active.
	GdkRGBA bgActiveColor;

	/// @brief The color of the background when inactive.
	GdkRGBA bgInactiveColor;

	/// @brief The color of the background when invalid.
	GdkRGBA bgInvalidColor;

	/// @brief The color of the border when active.
	GdkRGBA borderActiveColor;

	/// @brief The color of the border when inactive.
	GdkRGBA borderInactiveColor;

	/// @brief The color of the border when invalid.
	GdkRGBA borderInvalidColor;

	/// @brief The color of the font when active.
	GdkRGBA fontActiveColor;

	/// @brief The color of the font when inactive.
	GdkRGBA fontInactiveColor;

	/// @brief The color of the font when invalid.
	GdkRGBA fontInvalidColor;

	/// @brief The shape to render the indicator as.
	canIndicatorShape_t shape;

	/// @brief Default / minimum width of the widget.
	int width;

	/// @brief Default / minimum height of the widget.
	int height;

	/// @brief The radius of the shape's corners (if any).
	float cornerRadius;

	/// @brief The thickness of the border, if any.
	float borderThickness;

	/// @brief The amount of time, in seconds, of the interval to blink at. Use 0 for no blinking.
	float blinkInterval;
} canIndicatorConfig_t;

// Functions ------------------------------------------------------------------------------------------------------------------

/**
 * @brief Creates and initializes a CAN indicator.
 * @param database The CAN database to bind to.
 * @param config The configuration to use. Note a copy is made, so temporary pointers are acceptable, however the @c polygon
 * array is not copied deeply, so it must match the lifetime of this widget.
 * @return The created widget, if successful, @c NULL otherwise.
 */
canWidget_t* canIndicatorInit (canDatabase_t* database, canIndicatorConfig_t* config);

/**
 * @brief Creates and initializes a CAN indicator widget from a JSON configuration.
 * @param database The CAN database to bind to.
 * @param config The JSON configuration to load from.
 * @return The created widget, if successful, @c NULL otherwise.
 */
canWidget_t* canIndicatorLoad (canDatabase_t* database, cJSON* config);

#endif // CAN_WIDGETS_H