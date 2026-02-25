// Header
#include "can_widget.h"

// Includes
#include "can_label_float.h"
#include "can_label_bool.h"
#include "can_indicator.h"
#include "can_label_fault.h"
#include "can_shutdown_loop_indicator.h"
#include "can_fault_popup.h"
#include "can_float_popup.h"
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

	if (strcmp (widgetType, "canLabelFault_t") == 0)
	{
		canWidget_t* widget = canLabelFaultLoad (database, config);
		if (widget == NULL)
		{
			errorPrintf ("Failed to load widget of type 'canLabelFault_t'");
			return NULL;
		}

		return widget;
	}

	if (strcmp (widgetType, "canShutdownLoopIndicator_t") == 0)
	{
		canWidget_t* widget = canShutdownLoopIndicatorLoad (database, config);
		if (widget == NULL)
		{
			errorPrintf ("Failed to load widget of type 'canShutdownLoopIndicator_t'");
			return NULL;
		}

		return widget;
	}

	if (strcmp (widgetType, "canFaultPopup_t") == 0)
	{
		canWidget_t* widget = canFaultPopupLoad (database, config);
		if (widget == NULL)
		{
			errorPrintf ("Failed to load widget of type 'canFaultPopup_t'");
			return NULL;
		}

		return widget;
	}

	if (strcmp (widgetType, "canFloatPopup_t") == 0)
	{
		canWidget_t* widget = canFloatPopupLoad (database, config);
		if (widget == NULL)
		{
			errorPrintf ("Failed to load widget of type 'canFloatPopup_t'");
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