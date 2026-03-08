// Header
#include "can_widget.h"

// TODO(Barach): This form is deprecated
// CAN Widgets
#include "can_label_float.h"
#include "can_label_bool.h"
#include "can_label_fault.h"
#include "can_float_popup.h"
#include "can_widget_template.h"

// Includes
#include "cjson/cjson_util.h"
#include "debug.h"

#define STRINGIFY(str) #str
#define CONCETENATE(a, b) a ## b

#define LOAD_WIDGET_STATEMENTS(type)																						\
	if (strcmp (widgetType, STRINGIFY (type)) == 0)																			\
	{																														\
		canWidget_t* widget = CONCETENATE (type, Load) (database, config, &style->type);									\
		if (widget == NULL)																									\
		{																													\
			errorPrintf ("Failed to load widget of type '%s'", STRINGIFY (type));											\
			return NULL;																									\
		}																													\
																															\
		return widget;																										\
	}

canWidget_t* canWidgetLoad (canDatabase_t* database, cJSON* config, canWidgetStyle_t* style)
{
	if (config == NULL)
		return NULL;

	char* widgetType;
	if (jsonGetString (config, "type", &widgetType) != 0)
	{
		debugPrintf ("Warning, CAN widget is missing type.\n");
		return NULL;
	}

	LOAD_WIDGET_STATEMENTS (canIndicator);

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
		canWidget_t* widget = canShutdownLoopIndicatorLoad (database, config, &style->canShutdownLoopIndicator);
		if (widget == NULL)
		{
			errorPrintf ("Failed to load widget of type 'canShutdownLoopIndicator_t'");
			return NULL;
		}

		return widget;
	}

	if (strcmp (widgetType, "canFaultPopup_t") == 0)
	{
		canWidget_t* widget = canFaultPopupLoad (database, config, &style->canFaultPopup);
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

#define LOAD_STYLE_STATEMENTS(type)																							\
	styleConfig = config != NULL ? jsonGetObjectV2 (config, STRINGIFY (type)) : NULL;										\
	CONCETENATE (type, LoadStyle) (styleConfig, &style->type, parent != NULL ? &parent->type : NULL);

void canWidgetLoadStyle (canWidgetStyle_t* style, cJSON* config, canWidgetStyle_t* parent)
{
	cJSON* styleConfig;

	LOAD_STYLE_STATEMENTS (canIndicator);
	LOAD_STYLE_STATEMENTS (canShutdownLoopIndicator);
	LOAD_STYLE_STATEMENTS (canFaultPopup);
}