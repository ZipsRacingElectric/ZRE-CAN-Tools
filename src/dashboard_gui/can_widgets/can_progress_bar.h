#ifndef CAN_PROGRESS_BAR_H
#define CAN_PROGRESS_BAR_H

// CAN Database Progress Bar ------------------------------------------------------------------------------------------
//
// Author: Cole Barach
// Date Created: 2026.01.02
//
// Description: GTK widget displaying a CAN database signal as the amount of fill in a progress bar.

// Includes -------------------------------------------------------------------------------------------------------------------

// Includes
#include "can_widget_internal.h"
#include "../stylized_widgets/stylized_progress_bar.h"
#include "can_database/can_database.h"
#include "cjson/cjson.h"

// GTK
#include <gtk/gtk.h>

// Datatypes ------------------------------------------------------------------------------------------------------------------

typedef struct
{
	/// @brief Configuration of the base progress bar.
	stylizedProgressBarConfig_t barConfig;
} canProgressBarStyle_t;

/// @brief Configuration for the @c canProgressBar_t widget.
typedef struct
{
	/// @brief The name of the CAN database signal to bind to.
	char* signalName;

	/// @brief The signal value to map to the minimum bar fill (0%).
	float min;

	/// @brief The signal value to map to the maximum bar fill (100%).
	float max;

	/// @brief The style of the widget.
	canProgressBarStyle_t style;
} canProgressBarConfig_t;

// Functions ------------------------------------------------------------------------------------------------------------------

/**
 * @brief Creates and initializes a CAN progress bar.
 * @param database The CAN database to bind to.
 * @param config The configuration to use. Note a copy is made, so temporary pointers are acceptable.
 * @return The created widget, if successful, @c NULL otherwise.
 */
canWidget_t* canProgressBarInit (canDatabase_t* database, canProgressBarConfig_t* config);

/**
 * @brief Creates and initializes a CAN progress bar from a JSON configuration.
 * @param database The CAN database to bind to.
 * @param config The JSON configuration to load from.
 * @param parentStyle The parent style to inherit from, or @c NULL .
 * @return The created widget, if successful, @c NULL otherwise.
 */
canWidget_t* canProgressBarLoad (canDatabase_t* database, cJSON* config, canProgressBarStyle_t* parentStyle);

/**
 * @brief Loads a CAN progress bar's style from a JSON configuration.
 * @param config JSON configuration to use.
 * @param style The style to load into.
 * @param parent The parent style to inherit from, or @c NULL .
 */
void canProgressBarLoadStyle (cJSON* config, canProgressBarStyle_t* style, canProgressBarStyle_t* parent);

#endif // CAN_PROGRESS_BAR_H