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

// TODO(Barach): Comments needed

// Includes -------------------------------------------------------------------------------------------------------------------

// Includes
#include "can_widget.h"
#include "can_database/can_database.h"

// GTK
#include <gtk/gtk.h>

// Datatypes ------------------------------------------------------------------------------------------------------------------

typedef enum
{
	CAN_INDICATOR_CIRCLE,
	CAN_INDICATOR_RECT
} canIndicatorShape_t;

/// @brief Configuration for the @c canIndicator_t widget.
typedef struct
{
	/// @brief The name of the CAN database signal to bind to.
	const char* signalName;

	/// @brief The threshold to compare the signal against.
	float threshold;

	/// @brief False for non-inverted, true for inverted.
	bool inverted;

	/// @brief Default / minimum width of the widget.
	int width;

	/// @brief Default / minimum height of the widget.
	int height;

	GdkRGBA bgActiveColor;
	GdkRGBA bgInactiveColor;
	GdkRGBA bgInvalidColor;

	GdkRGBA borderActiveColor;
	GdkRGBA borderInactiveColor;
	GdkRGBA borderInvalidColor;

	GdkRGBA fontActiveColor;
	GdkRGBA fontInactiveColor;
	GdkRGBA fontInvalidColor;

	/// @brief The shape to render the indicator as.
	canIndicatorShape_t shape;

	/// @brief The radius of the shape's corners (if any).
	float cornerRadius;

	float borderThickness;

	char* text;
	char* font;
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

// TODO(Barach)
canWidget_t* canIndicatorLoad (canDatabase_t* database, cJSON* config);

#endif // CAN_WIDGETS_H