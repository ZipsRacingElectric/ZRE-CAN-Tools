#ifndef CAN_WIDGET_H
#define CAN_WIDGET_H

// Base GTK CAN Database Widget -----------------------------------------------------------------------------------------------
//
// Author: Cole Barach
// Date Created: 2025.12.16
//
// Description: Base polymorphic object describing a widget that sources information from a CAN database. This family of
//   widgets all require a routine 'update' at a fixed rate.
//
// Example:
//
//   // Note: Error handling must be performed.
//   cJSON* config = jsonGetObjectV2 (json, "widgetConfig");
//   canWidget_t* widget = canWidgetLoad (database, config, NULL);
//   gtk_grid_attach (grid, CAN_WIDGET_TO_WIDGET (widget), 0, 0, 1, 1);

// Includes -------------------------------------------------------------------------------------------------------------------

// CAN Widgets
#include "can_fault_popup.h"
#include "can_indicator.h"
#include "can_progress_bar.h"
#include "can_shutdown_loop_indicator.h"

// GTK
#include <gtk/gtk.h>

// Datatypes ------------------------------------------------------------------------------------------------------------------

/// @brief Object representing the base style any CAN widget can inherit from.
typedef struct
{
	canFaultPopupStyle_t canFaultPopup;
	canIndicatorStyle_t canIndicator;
	canProgressBarStyle_t canProgressBar;
	canShutdownLoopIndicatorStyle_t canShutdownLoopIndicator;
} canWidgetStyle_t;

// Functions ------------------------------------------------------------------------------------------------------------------

/**
 * @brief Loads any type of CAN widget from a JSON configuration. This function parses the "type" field of the provided JSON to
 * determine which type of CAN widget the configuration belongs to, then loads the widget via its constructor.
 * @param database The CAN database to bind the widget to.
 * @param config The JSON configuration to use. The remainder of the config is parsed as the widget-specific config of the
 * widget specified by "type".
 * @param style The base CAN widget style to inherit from.
 * @return The created widget, if successful, @c NULL otherwise.
 */
canWidget_t* canWidgetLoad (canDatabase_t* database, cJSON* config, canWidgetStyle_t* style);

/**
 * @brief Loads an array of CAN widgets from a JSON configuration. See @c canWidgetLoad for how the individual widgets are loaded.
 * @param database The CAN database to bind the widget to.
 * @param config The JSON configuration to use. This should be an array of CAN widget configurations. If not an array (or
 * @c NULL ), @c errno is set to @c EINVAL and this fails.
 * @param style The base CAN widget style to inherit from.
 * @param count Buffer to write the number of elements into.
 * @return A dynamically allocated array of CAN widgets, if successful, @c NULL otherwise. Note both the array and every
 * individual element within it must be deallocated.
 */
canWidget_t** canWidgetLoadArray (canDatabase_t* database, cJSON* config, canWidgetStyle_t* style, size_t* count);

/**
 * @brief Prompts a CAN widget to update its information.
 * @param widget The widget to update. This may be @c NULL , in which case nothing happens.
 */
static inline void canWidgetUpdate (canWidget_t* widget)
{
	// Invoke the overridden update function
	if (widget != NULL)
		widget->vmt.update (widget);
}

/**
 * @brief Updates an entire array of CAN widgets.
 * @param widgets The array to update. Allowed to have @c NULL elements.
 * @param count The number of elements in the array.
 */
static inline void canWidgetUpdateArray (canWidget_t** widgets, size_t count)
{
	if (widgets == NULL || count == 0)
		return;

	// Update each individual widget
	for (size_t index = 0; index < count; ++index)
		canWidgetUpdate (widgets [index]);
}

/**
 * @brief Loads a base CAN widget style from a JSON configuration.
 * @param style The style to load into.
 * @param config The JSON config to use.
 * @param parent The base CAN widget style to inherit from.
 */
void canWidgetLoadStyle (canWidgetStyle_t* style, cJSON* config, canWidgetStyle_t* parent);

#endif // CAN_WIDGET_H