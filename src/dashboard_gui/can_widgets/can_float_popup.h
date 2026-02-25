#ifndef CAN_FLOAT_POPUP_H
#define CAN_FLOAT_POPUP_H

// Floating-Point Popup CAN Widget --------------------------------------------------------------------------------------------
//
// Author: Cole Barach
// Date Created: 2026.02.25
//
// Description: TODO(Barach)

// Includes -------------------------------------------------------------------------------------------------------------------

// Includes
#include "can_widget.h"
#include "../stylized_widgets/stylized_frame.h"
#include "can_database/can_database.h"

// GTK
#include <gtk/gtk.h>

// Datatypes ------------------------------------------------------------------------------------------------------------------

typedef struct
{
	/// @brief The minimum width, in pixels.
	int width;

	/// @brief The minimum height, in pixels.
	int height;

	/// @brief The padding, in pixels.
	int padding;

	char* signalName;

	/// @brief The format string to use. Must contain the '%f' specifier once.
	char* format;

	/// @brief The font of the label.
	char* font;

	float threshold;
	bool inverted;

	/// @brief Config of the popup's frame.
	stylizedFrameConfig_t frameConfig;
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
 * @return The created widget, if successful, @c NULL otherwise.
 */
canWidget_t* canFloatPopupLoad (canDatabase_t* database, cJSON* config);

#endif // CAN_FLOAT_POPUP_H