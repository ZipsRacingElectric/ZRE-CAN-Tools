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
#include "can_widget_internal.h"
#include "can_database/can_database.h"
#include "can_node/fault_signal.h"

// GTK
#include <gtk/gtk.h>

// Datatypes ------------------------------------------------------------------------------------------------------------------

typedef struct
{
	// Nothing here right now.
} canLabelFaultStyle_t;

typedef struct
{
	/// @brief The array of fault signals to display. Required. Specified as "configPath" in JSON.
	faultSignals_t faults;

	/// @brief The format string to use. Must contain the '%s' specifier exactly once. Required.
	char* format;

	/// @brief The style of the widget.
	canLabelFaultStyle_t style;
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
 * @brief Creates and initializes a fault CAN label from a JSON configuration.
 * @param database The CAN database to bind to.
 * @param config The JSON configuration to load from.
 * @param parentStyle The parent style to inherit from, or @c NULL .
 * @return The created widget, if successful, @c NULL otherwise.
 */
canWidget_t* canLabelFaultLoad (canDatabase_t* database, cJSON* config, canLabelFaultStyle_t* parentStyle);

/**
 * @brief Loads a fault CAN label's style from a JSON configuration.
 * @param config JSON configuration to use.
 * @param style The style to load into.
 * @param parent The parent style to inherit from, or @c NULL .
 */
void canLabelFaultLoadStyle (cJSON* config, canLabelFaultStyle_t* style, canLabelFaultStyle_t* parent);

#endif // CAN_LABEL_FAULT_H