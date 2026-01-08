// Header
#include "can_widget.h"

// Includes
#include "can_label_float.h"
#include "can_label_bool.h"
#include "cjson/cjson_util.h"
#include "debug.h"

canWidget_t* canWidgetLoad (canDatabase_t* database, cJSON* config)
{
	if (config == NULL)
		return NULL;

	char* widgetType;
	if (jsonGetString (config, "type", &widgetType) != 0)
		return NULL;

	canWidget_t* widget = canLabelFloatLoad (database, config);
	if (widget != NULL)
		return widget;

	widget = canLabelBoolLoad (database, config);
	if (widget != NULL)
		return widget;

	debugPrintf ("Warning, unknown CAN widget type '%s'.\n", widgetType);
	return NULL;
}