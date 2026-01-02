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

// GTK
#include <gtk/gtk.h>

// Datatypes ------------------------------------------------------------------------------------------------------------------

/// @brief Configuration for the @c canProgressBar_t widget.
typedef struct
{
	/// @brief The name of the CAN database signal to bind to.
	const char* signalName;

	/// @brief The signal value to map to the minimum bar fill (0%).
	float min;

	/// @brief The signal value to map to the maximum bar fill (100%).
	float max;
} canProgressBarConfig_t;

/// @brief Converts a @c canProgressBar_t into a GTK progress bar.
#define CAN_PROGRESS_BAR_TO_PROGRESS_BAR(wdgt) GTK_PROGRESS_BAR (CAN_WIDGET_TO_WIDGET (wdgt))

/// @brief Converts a @c canProgressBar_t into an orientable object.
#define CAN_PROGRESS_BAR_TO_ORIENTABLE(wdgt) GTK_ORIENTABLE (CAN_WIDGET_TO_WIDGET (wdgt))

// Functions ------------------------------------------------------------------------------------------------------------------

/**
 * @brief Creates and initializes a CAN progress bar.
 * @param database The CAN database to bind to.
 * @param config The configuration to use. Note a copy is made, so temporary pointers are acceptable.
 * @return The created widget, if successful, @c NULL otherwise.
 */
canWidget_t* canProgressBarInit (canDatabase_t* database, canProgressBarConfig_t* config);

#endif // CAN_PROGRESS_BAR_H