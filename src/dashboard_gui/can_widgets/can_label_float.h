#ifndef CAN_LABEL_FLOAT_H
#define CAN_LABEL_FLOAT_H

// Floating-Point CAN Database Label ------------------------------------------------------------------------------------------
//
// Author: Cole Barach
// Date Created: 2026.01.02
//
// Description: GTK widget displaying a CAN database signal as a floating point number.

// Includes -------------------------------------------------------------------------------------------------------------------

// Includes
#include "can_widget.h"
#include "can_database/can_database.h"

// GTK
#include <gtk/gtk.h>

// Datatypes ------------------------------------------------------------------------------------------------------------------

/// @brief Configuration for the @c canLabelFloat_t widget.
typedef struct
{
	/// @brief The name of the CAN database signal to bind to.
	const char* signalName;

	/// @brief Format string for valid floating-point values. See @c snprintCanDatabaseFloat in
	/// @c can_database/can_database_stdio.h for more details.
	const char* formatValue;

	/// @brief Format string for invalid values. See @c snprintCanDatabaseFloat in @c can_database/can_database_stdio.h for
	/// more details.
	const char* formatInvalid;
} canLabelFloatConfig_t;

/// @brief Converts a @c canLabelFloat_t into a GTK label.
#define CAN_LABEL_FLOAT_TO_LABEL(wdgt) GTK_LABEL (CAN_WIDGET_TO_WIDGET (wdgt))

// Functions ------------------------------------------------------------------------------------------------------------------

/**
 * @brief Creates and initializes a CAN label.
 * @param database The CAN database to bind to.
 * @param config The configuration to use. Note a copy is made, so temporary pointers are acceptable.
 * @return The created widget, if successful, @c NULL otherwise.
 */
canWidget_t* canLabelFloatInit (canDatabase_t* database, canLabelFloatConfig_t* config);

#endif // CAN_LABEL_FLOAT_H