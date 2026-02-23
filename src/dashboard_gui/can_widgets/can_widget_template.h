#ifndef CAN_WIDGET_TEMPLATE_H
#define CAN_WIDGET_TEMPLATE_H

// Template CAN Widget --------------------------------------------------------------------------------------------------------
//
// Author: Cole Barach
// Date Created: 2026.02.23
//
// Description: Template header/source files for creating new CAN widgets.

// Includes -------------------------------------------------------------------------------------------------------------------

// Includes
#include "can_widget.h"
#include "can_database/can_database.h"

// GTK
#include <gtk/gtk.h>

// Datatypes ------------------------------------------------------------------------------------------------------------------

typedef struct
{
	// Config elements here...
} canWidgetTemplateConfig_t;

// Functions ------------------------------------------------------------------------------------------------------------------

/**
 * @brief Creates and initializes a template CAN widget.
 * @param database The CAN database to bind to.
 * @param config The configuration to use. Note a copy is made, so temporary pointers are acceptable.
 * @return The created widget, if successful, @c NULL otherwise.
 */
canWidget_t* canWidgetTemplateInit (canDatabase_t* database, canWidgetTemplateConfig_t* config);

/**
 * @brief Creates and initializes a template widget from a JSON configuration.
 * @param database The CAN database to bind to.
 * @param config The JSON configuration to load from.
 * @return The created widget, if successful, @c NULL otherwise.
 */
canWidget_t* canWidgetTemplateLoad (canDatabase_t* database, cJSON* config);

#endif // CAN_WIDGET_TEMPLATE_H