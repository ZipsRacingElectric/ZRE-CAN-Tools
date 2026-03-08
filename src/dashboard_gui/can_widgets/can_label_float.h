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
#include "can_widget_internal.h"
#include "can_database/can_database.h"
#include "cjson/cjson.h"

// GTK
#include <gtk/gtk.h>

// Datatypes ------------------------------------------------------------------------------------------------------------------

typedef struct
{
	// Nothing here right now.
} canLabelFloatStyle_t;

/// @brief Configuration for the @c canLabelFloat_t widget.
typedef struct
{
	/// @brief The name of the CAN database signal to bind to. Required.
	char* signalName;

	/// @brief Format string for valid floating-point values. Must contain the %f format specified exactly once. May contain
	/// the %s after the first specifier for the unit. Required.
	char* formatValue;

	/// @brief Format string for invalid values. Must contain the %s format specified exactly once. May contain the %s after
	/// the first specifier for the unit. Required.
	char* formatInvalid;
} canLabelFloatConfig_t;

// Functions ------------------------------------------------------------------------------------------------------------------

/**
 * @brief Creates and initializes a floating-point CAN label.
 * @param database The CAN database to bind to.
 * @param config The configuration to use. Note a copy is made, so temporary pointers are acceptable.
 * @return The created widget, if successful, @c NULL otherwise.
 */
canWidget_t* canLabelFloatInit (canDatabase_t* database, canLabelFloatConfig_t* config);

/**
 * @brief Creates and initializes a floating-point CAN label from a JSON configuration.
 * @param database The CAN database to bind to.
 * @param config The JSON configuration to load from.
 * @param parentStyle The parent style to inherit from, or @c NULL .
 * @return The created widget, if successful, @c NULL otherwise.
 */
canWidget_t* canLabelFloatLoad (canDatabase_t* database, cJSON* config, canLabelFloatStyle_t* parentStyle);

/**
 * @brief Loads a floating-point CAN label's style from a JSON configuration.
 * @param config JSON configuration to use.
 * @param style The style to load into.
 * @param parent The parent style to inherit from, or @c NULL .
 */
void canLabelFloatLoadStyle (cJSON* config, canLabelFloatStyle_t* style, canLabelFloatStyle_t* parent);

#endif // CAN_LABEL_FLOAT_H