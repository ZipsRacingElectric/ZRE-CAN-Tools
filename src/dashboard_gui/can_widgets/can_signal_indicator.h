#ifndef CAN_SIGNAL_INDICATOR
#define CAN_SIGNAL_INDICATOR

// CAN Bus Indicator Widget ---------------------------------------------------------------------------------------------------
// TODO: Create Description

// Includes
#include "can_widget.h"
#include "can_database/can_database.h"

// GTK
#include <gtk/gtk.h>

typedef struct
{

	// TODO: fix comments
    /// @brief The names of the CAN database signals that are associated with the ...
	char** signalNames;

	size_t signalNamesSize;

    /// @brief False for non-inverted, true for inverted.
	bool inverted;

    /// @brief Default / minimum width of the widget.
	int width;

	/// @brief Default / minimum height of the widget.
	int height;

    /// @brief Color to draw the widget when the signal is in fault state.
	GdkRGBA faultColor;

    /// @brief Color to draw the widget when the signal is in no fault state.
	GdkRGBA noFaultColor;

	/// @brief Color to draw the widget when the signal is in invalid state.
	GdkRGBA invalidColor;

} canSignalIndicatorConfig_t;

// Functions ------------------------------------------------------------------------------------------------------------------

canWidget_t* canSignalIndicatorInit (canDatabase_t* database, canSignalIndicatorConfig_t* config);

#endif // CAN_SIGNAL_INDICATOR