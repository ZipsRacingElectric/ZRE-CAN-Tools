// Header
#include "can_widget.h"

// Includes
#include "can_label_float.h"
#include "can_label_bool.h"
#include "can_indicator.h"
#include "can_widget_template.h"
#include "cjson/cjson_util.h"
#include "debug.h"

canWidget_t* canWidgetLoad (canDatabase_t* database, cJSON* config)
{
	if (config == NULL)
		return NULL;

	char* widgetType;
	if (jsonGetString (config, "type", &widgetType) != 0)
	{
		debugPrintf ("Warning, CAN widget is missing type.\n");
		return NULL;
	}

	if (strcmp (widgetType, "canLabelFloat_t") == 0)
	{
		canWidget_t* widget = canLabelFloatLoad (database, config);
		if (widget == NULL)
		{
			errorPrintf ("Failed to load widget of type 'canLabelFloat_t'");
			return widget;
		}

		return widget;
	}

	if (strcmp (widgetType, "canLabelBool_t") == 0)
	{
		canWidget_t* widget = canLabelBoolLoad (database, config);
		if (widget == NULL)
		{
			errorPrintf ("Failed to load widget of type 'canLabelBool_t'");
			return NULL;
		}

		return widget;
	}

	if (strcmp (widgetType, "canIndicator_t") == 0)
	{
		canWidget_t* widget = canIndicatorLoad (database, config);
		if (widget == NULL)
		{
			errorPrintf ("Failed to load widget of type 'canIndicator_t'");
			return NULL;
		}

		return widget;
	}

	if (strcmp (widgetType, "canWidgetTemplate_t") == 0)
	{
		canWidget_t* widget = canWidgetTemplateLoad (database, config);
		if (widget == NULL)
		{
			errorPrintf ("Failed to load widget of type 'canWidgetTemplate_t'");
			return NULL;
		}

		return widget;
	}

	if (strcmp (widgetType, "null") == 0)
		return NULL;

	fprintf (stderr, "Failed to load widget of unknown type '%s'.\n", widgetType);
	return NULL;
}