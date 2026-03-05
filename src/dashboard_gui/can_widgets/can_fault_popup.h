#ifndef CAN_FAULT_POPUP_H
#define CAN_FAULT_POPUP_H

// Fault Popup CAN Widget -----------------------------------------------------------------------------------------------------
//
// Author: Cole Barach
// Date Created: 2026.02.25
//
// Description: TODO(Barach)

// Includes -------------------------------------------------------------------------------------------------------------------

// Includes
#include "can_widget_internal.h"
#include "../stylized_widgets/stylized_frame.h"
#include "can_database/can_database.h"
#include "can_node/fault_signal.h"

// GTK
#include <gtk/gtk.h>

// Datatypes ------------------------------------------------------------------------------------------------------------------

typedef struct
{
	/// @brief Config of the popup's frame.
	stylizedFrameConfig_t frameConfig;

	/// @brief The font of the label.
	char* font;

	/// @brief The padding, in pixels.
	int padding;
} canFaultPopupStyle_t;

typedef struct
{
	/// @brief The array of fault signals to display.
	faultSignals_t faults;

	/// @brief The format string to use. Must contain the '%s' specifier once. Required.
	char* format;

	/// @brief The minimum width, in pixels. Default 0.
	int width;

	/// @brief The minimum height, in pixels. Default 0.
	int height;

	/// @brief The style of the widget.
	canFaultPopupStyle_t style;
} canFaultPopupConfig_t;

// Functions ------------------------------------------------------------------------------------------------------------------

/**
 * @brief Creates and initializes a fault popup CAN widget.
 * @param database The CAN database to bind to.
 * @param config The configuration to use. Note a copy is made, so temporary pointers are acceptable.
 * @return The created widget, if successful, @c NULL otherwise.
 */
canWidget_t* canFaultPopupInit (canDatabase_t* database, canFaultPopupConfig_t* config);

/**
 * @brief Creates and initializes a fault popup CAN widget from a JSON configuration.
 * @param database The CAN database to bind to.
 * @param config The JSON configuration to load from.
 * @param parentStyle The parent style to inherit from, or @c NULL .
 * @return The created widget, if successful, @c NULL otherwise.
 */
canWidget_t* canFaultPopupLoad (canDatabase_t* database, cJSON* config, canFaultPopupStyle_t* parentStyle);

/**
 * @brief Loads a fault popup CAN widget's style from a JSON configuration.
 * @param config JSON configuration to use.
 * @param style The style to load into.
 * @param parent The parent style to inherit from, or @c NULL .
 */
void canFaultPopupLoadStyle (cJSON* config, canFaultPopupStyle_t* style, canFaultPopupStyle_t* parent);

#endif // CAN_FAULT_POPUP_H