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
#include "can_widget_internal.h"
#include "can_database/can_database.h"
#include "cjson/cjson.h"

// GTK
#include <gtk/gtk.h>

// Datatypes ------------------------------------------------------------------------------------------------------------------

typedef struct
{
	// Nothing here right now.
} canLabelBoolStyle_t;

/// @brief Configuration for the @c canLabelBool_t widget.
typedef struct
{
	/// @brief The name of the CAN database signal to bind to. Required.
	char* signalName;

	/// @brief The string to display when the value is active. Required.
	char* activeValue;

	/// @brief The string to display when the value is inactive. Required.
	char* inactiveValue;

	/// @brief The string to display when the value is invalid. Required.
	char* invalidValue;

	/// @brief The threshold to compare the signal against. Required.
	float threshold;

	/// @brief False for non-inverted, true for inverted. Default false.
	bool inverted;

	/// @brief The style of the widget.
	canLabelBoolStyle_t style;
} canLabelBoolConfig_t;

// Functions ------------------------------------------------------------------------------------------------------------------

/**
 * @brief Creates and initializes a boolean CAN label.
 * @param database The CAN database to bind to.
 * @param config The configuration to use. Note a copy is made, so temporary pointers are acceptable.
 * @return The created widget, if successful, @c NULL otherwise.
 */
canWidget_t* canLabelBoolInit (canDatabase_t* database, canLabelBoolConfig_t* config);

/**
 * @brief Creates and initializes a boolean CAN label from a JSON configuration.
 * @param database The CAN database to bind to.
 * @param config The JSON configuration to load from.
 * @param parentStyle The parent style to inherit from, or @c NULL .
 * @return The created widget, if successful, @c NULL otherwise.
 */
canWidget_t* canLabelBoolLoad (canDatabase_t* database, cJSON* config, canLabelBoolStyle_t* parentStyle);

/**
 * @brief Loads a boolean CAN label's style from a JSON configuration.
 * @param config JSON configuration to use.
 * @param style The style to load into.
 * @param parent The parent style to inherit from, or @c NULL .
 */
void canLabelBoolLoadStyle (cJSON* config, canLabelBoolStyle_t* style, canLabelBoolStyle_t* parent);

#endif // CAN_LABEL_BOOL_H