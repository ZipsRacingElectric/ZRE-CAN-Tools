#ifndef CAN_FLOAT_POPUP_H
#define CAN_FLOAT_POPUP_H

// Floating-Point Popup CAN Widget --------------------------------------------------------------------------------------------
//
// Author: Cole Barach
// Date Created: 2026.02.25
//
// Description: GTK widget displaying a floating point value as popup. This widget uses a threshold to determine whether the
//   popup should be active or not.

// Includes -------------------------------------------------------------------------------------------------------------------

// Includes
#include "can_widget_internal.h"
#include "../stylized_widgets/stylized_frame.h"
#include "can_database/can_database.h"
#include "cjson/cjson.h"

// GTK
#include <gtk/gtk.h>

// Datatypes ------------------------------------------------------------------------------------------------------------------

typedef struct
{
	/// @brief The font of the label. Default is GTK's default font.
	char* font;

	/// @brief The padding, in pixels, to use for the popup's label. Default 0.
	int padding;

	/// @brief Config of the popup's frame.
	stylizedFrameConfig_t frameConfig;
} canFloatPopupStyle_t;

typedef struct
{
	/// @brief The name of the CAN database signal to bind to. Required
	char* signalName;

	/// @brief The format string to use. Must contain the '%f' specifier exactly once. Default 0.
	char* format;

	/// @brief The threshold to compare the signal against. Required.
	float threshold;

	/// @brief False => active if signal > threshold. True active if signal < threshold. Default false.
	bool inverted;

	/// @brief The minimum width, in pixels. Default 0.
	int width;

	/// @brief The minimum height, in pixels. Default 0.
	int height;

	/// @brief The style of the widget.
	canFloatPopupStyle_t style;
} canFloatPopupConfig_t;

// Functions ------------------------------------------------------------------------------------------------------------------

/**
 * @brief Creates and initializes a floating-point popup CAN widget.
 * @param database The CAN database to bind to.
 * @param config The configuration to use. Note a copy is made, so temporary pointers are acceptable.
 * @return The created widget, if successful, @c NULL otherwise.
 */
canWidget_t* canFloatPopupInit (canDatabase_t* database, canFloatPopupConfig_t* config);

/**
 * @brief Creates and initializes a floating-point popup CAN widget from a JSON configuration.
 * @param database The CAN database to bind to.
 * @param config The JSON configuration to load from.
 * @param parentStyle The parent style to inherit from, or @c NULL .
 * @return The created widget, if successful, @c NULL otherwise.
 */
canWidget_t* canFloatPopupLoad (canDatabase_t* database, cJSON* config, canFloatPopupStyle_t* parentStyle);

/**
 * @brief Loads a floating-point popup CAN widget's style from a JSON configuration.
 * @param config JSON configuration to use.
 * @param style The style to load into.
 * @param parent The parent style to inherit from, or @c NULL .
 */
void canFloatPopupLoadStyle (cJSON* config, canFloatPopupStyle_t* style, canFloatPopupStyle_t* parent);

#endif // CAN_FLOAT_POPUP_H