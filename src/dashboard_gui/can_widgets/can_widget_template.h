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
#include "can_widget_internal.h"
#include "can_database/can_database.h"
#include "cjson/cjson.h"

// GTK
#include <gtk/gtk.h>

// Datatypes ------------------------------------------------------------------------------------------------------------------

typedef struct
{
	// TODO: Style elements here...
} canWidgetTemplateStyle_t;

typedef struct
{
	// TODO: Config elements here...

	/// @brief The style of the widget.
	canWidgetTemplateStyle_t style;
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
 * @brief Creates and initializes a template CAN widget from a JSON configuration.
 * @param database The CAN database to bind to.
 * @param config The JSON configuration to load from.
 * @param parentStyle The parent style to inherit from, or @c NULL .
 * @return The created widget, if successful, @c NULL otherwise.
 */
canWidget_t* canWidgetTemplateLoad (canDatabase_t* database, cJSON* config, canWidgetTemplateStyle_t* parentStyle);

/**
 * @brief Loads a template CAN widget's style from a JSON configuration.
 * @param config JSON configuration to use.
 * @param style The style to load into.
 * @param parent The parent style to inherit from, or @c NULL .
 */
void canWidgetTemplateLoadStyle (cJSON* config, canWidgetTemplateStyle_t* style, canWidgetTemplateStyle_t* parent);

#endif // CAN_WIDGET_TEMPLATE_H