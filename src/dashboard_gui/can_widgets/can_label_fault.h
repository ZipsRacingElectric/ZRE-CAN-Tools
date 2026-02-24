#ifndef CAN_LABEL_FAULT_H
#define CAN_LABEL_FAULT_H

// CAN Database Fault Label --------------------------------------------------------------------------------------------------------
//
// Author: Cole Barach
// Date Created: 2026.02.23
//
// Description: GTK widget displaying a set of fault signals as a fault code.

// Includes -------------------------------------------------------------------------------------------------------------------

// Includes
#include "can_widget.h"
#include "can_database/can_database.h"
#include "can_node/fault_signal.h"

// GTK
#include <gtk/gtk.h>

// Datatypes ------------------------------------------------------------------------------------------------------------------

typedef struct
{
	/// @brief The array of fault signals to display.
	faultSignals_t faults;

	/// @brief The format string to use. Must contain the '%s' specifier once.
	char* format;
} canLabelFaultConfig_t;

// Functions ------------------------------------------------------------------------------------------------------------------

/**
 * @brief Creates and initializes a fault CAN widget.
 * @param database The CAN database to bind to.
 * @param config The configuration to use. Note a copy is made, so temporary pointers are acceptable.
 * @return The created widget, if successful, @c NULL otherwise.
 */
canWidget_t* canLabelFaultInit (canDatabase_t* database, canLabelFaultConfig_t* config);

/**
 * @brief Creates and initializes a fault CAN widget from a JSON configuration.
 * @param database The CAN database to bind to.
 * @param config The JSON configuration to load from.
 * @return The created widget, if successful, @c NULL otherwise.
 */
canWidget_t* canLabelFaultLoad (canDatabase_t* database, cJSON* config);

#endif // CAN_LABEL_FAULT_H