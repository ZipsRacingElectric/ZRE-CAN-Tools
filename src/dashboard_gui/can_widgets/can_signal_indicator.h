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

	// TODO: fix comment
    /// @brief The names of the CAN database signals that are associated with the ...
	const char* signalName;

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

/// @brief Converts a canSignalIndicator_t widget into a GTK drawing area.
#define CAN_SIGNAL_INDICATOR_TO_DRAWING_AREA(wdgt) GTK_DRAWING_AREA (CAN_WIDGET_TO_WIDGET (wdgt))

// Functions ------------------------------------------------------------------------------------------------------------------

canWidget_t* canSignalIndicatorInit (canDatabase_t* database, canSignalIndicatorConfig_t* config);

bool canSignalIndicatorGetState (void* widget);

#endif // CAN_SIGNAL_INDICATOR