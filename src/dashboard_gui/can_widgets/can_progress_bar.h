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
#include "can_widget.h"
#include "can_database/can_database.h"
#include "../stylized_widgets/stylized_progress_bar.h"

// GTK
#include <gtk/gtk.h>

// Datatypes ------------------------------------------------------------------------------------------------------------------

/// @brief Configuration for the @c canProgressBar_t widget.
typedef struct
{
	/// @brief Configuration of the base progress bar.
	stylizedProgressBarConfig_t barConfig;

	/// @brief The name of the CAN database signal to bind to.
	const char* signalName;

	/// @brief The signal value to map to the minimum bar fill (0%).
	float min;

	/// @brief The signal value to map to the maximum bar fill (100%).
	float max;
} canProgressBarConfig_t;

// Functions ------------------------------------------------------------------------------------------------------------------

/**
 * @brief Creates and initializes a CAN progress bar.
 * @param database The CAN database to bind to.
 * @param config The configuration to use. Note a copy is made, so temporary pointers are acceptable.
 * @return The created widget, if successful, @c NULL otherwise.
 */
canWidget_t* canProgressBarInit (canDatabase_t* database, canProgressBarConfig_t* config);

#endif // CAN_PROGRESS_BAR_H