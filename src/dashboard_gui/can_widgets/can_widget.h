#ifndef CAN_WIDGET_H
#define CAN_WIDGET_H

// Base GTK CAN Database Widget -----------------------------------------------------------------------------------------------
//
// Author: Cole Barach
// Date Created: 2025.12.16
//
// Description: Base polymorphic object describing a widget that sources information from a CAN database. This family of
//   widgets all require a routine 'update' at a fixed rate.

// Includes -------------------------------------------------------------------------------------------------------------------

// Includes
#include "can_database/can_database.h"
#include "cjson/cjson.h"

// GTK
#include <gtk/gtk.h>

// Datatypes ------------------------------------------------------------------------------------------------------------------

/**
 * @brief Virtual method table entry for implementations of the @c canWidgetUpdate function.
 * @param widget Pointer to the widget to update.
 */
typedef void (canWidgetUpdate_t) (void* widget);

/// @brief Virtual method (and member) table of the polymorphic @c canWidget_t structure.
typedef struct
{
	canWidgetUpdate_t* update;
	GtkWidget* widget;
} canWidgetVmt_t;

/// @brief Base polymorphic object for all CAN database widgets.
typedef struct
{
	/// @brief The object's virtual method table. Must be the first element in derived structures.
	canWidgetVmt_t vmt;
} canWidget_t;

/// @brief Converts a CAN widget into a GTK widget.
#define CAN_WIDGET_TO_WIDGET(wdgt) ((wdgt)->vmt.widget)

// Functions ------------------------------------------------------------------------------------------------------------------

/**
 * @brief Loads any type of CAN widget from a JSON configuration. This function parses the "type" field of the provided JSON to
 * determine which type of CAN widget the configuration belongs to, then loads the widget via its constructor.
 * @param database The CAN database to bind the widget to.
 * @param config The JSON configuration to use. The remainder of the config is parsed as the widget-specific config of the
 * widget specified by "type".
 * @return The created widget, if successful, @c NULL otherwise.
 */
canWidget_t* canWidgetLoad (canDatabase_t* database, cJSON* config);

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

#endif // CAN_WIDGET_H