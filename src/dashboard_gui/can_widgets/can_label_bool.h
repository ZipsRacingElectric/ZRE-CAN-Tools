#ifndef CAN_LABEL_BOOL_H
#define CAN_LABEL_BOOL_H

// Boolean CAN Database Label -------------------------------------------------------------------------------------------------
//
// Author: Cole Barach
// Date Created: 2026.01.02
//
// Description: GTK widget displaying a CAN database signal as a boolean value. The signal is converted to a boolean using
//   a threshold value. If the signal value exceeds the threshold value (greater than for non-inverted, less than for
//   inverted), the label is considered active.

// Includes -------------------------------------------------------------------------------------------------------------------

// Includes
#include "can_widget.h"
#include "can_database/can_database.h"

// GTK
#include <gtk/gtk.h>

// Datatypes ------------------------------------------------------------------------------------------------------------------

/// @brief Configuration for the @c canLabelBool_t widget.
typedef struct
{
	/// @brief The name of the CAN database signal to bind to.
	const char* signalName;

	/// @brief The string to display when the value is active.
	const char* activeValue;

	/// @brief The string to display when the value is inactive.
	const char* inactiveValue;

	/// @brief The string to display when the value is invalid.
	const char* invalidValue;

	/// @brief The threshold to compare the signal against.
	float threshold;

	/// @brief False for non-inverted, true for inverted.
	bool inverted;
} canLabelBoolConfig_t;

/// @brief Converts a @c canLabelBool_t into a GTK label.
#define CAN_LABEL_BOOL_TO_LABEL(wdgt) GTK_LABEL (CAN_WIDGET_TO_WIDGET (wdgt))

// Functions ------------------------------------------------------------------------------------------------------------------

/**
 * @brief Creates and initializes a CAN label.
 * @param database The CAN database to bind to.
 * @param config The configuration to use. Note a copy is made, so temporary pointers are acceptable.
 * @return The created widget, if successful, @c NULL otherwise.
 */
canWidget_t* canLabelBoolInit (canDatabase_t* database, canLabelBoolConfig_t* config);

#endif // CAN_LABEL_BOOL_H