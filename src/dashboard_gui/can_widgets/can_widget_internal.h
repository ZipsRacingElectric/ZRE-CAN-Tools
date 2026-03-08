#ifndef CAN_WIDGET_INTERNAL_H
#define CAN_WIDGET_INTERNAL_H

// GTK CAN Database Widget Internal Header ------------------------------------------------------------------------------------
//
// Author: Cole Barach
// Date Created: 2026.03.08
//
// Description: Internal datatypes used by implementations of CAN widgets.

// Includes -------------------------------------------------------------------------------------------------------------------

// GTK
#include <gtk/gtk.h>

// Macros ---------------------------------------------------------------------------------------------------------------------

/// @brief Converts a CAN widget into a GTK widget.
#define CAN_WIDGET_TO_WIDGET(wdgt) ((wdgt)->vmt.widget)

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

#endif // CAN_WIDGET_INTERNAL_H