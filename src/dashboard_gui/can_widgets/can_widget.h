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

// TODO(Barach): Docs
canWidget_t* canWidgetLoad (canDatabase_t* database, cJSON* config);

/**
 * @brief Updates a CAN widget with new information.
 * @param widget The widget to update.
 */
static inline void canWidgetUpdate (canWidget_t* widget)
{
	// Invoke the overridden update function
	if (widget != NULL)
		widget->vmt.update (widget);
}

#endif // CAN_WIDGET_H