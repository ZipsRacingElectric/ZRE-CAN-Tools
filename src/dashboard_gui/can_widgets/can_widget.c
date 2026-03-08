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

	LOAD_WIDGET_STATEMENTS (canFaultPopup);
	LOAD_WIDGET_STATEMENTS (canFloatPopup);
	LOAD_WIDGET_STATEMENTS (canIndicator);
	LOAD_WIDGET_STATEMENTS (canProgressBar);
	LOAD_WIDGET_STATEMENTS (canShutdownLoopIndicator);

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

canWidget_t** canWidgetLoadArray (canDatabase_t* database, cJSON* config, canWidgetStyle_t* style, size_t* count)
{
	// Check the config is a valid array
	if (config == NULL || !cJSON_IsArray (config))
	{
		errno = EINVAL;
		*count = 0;
		return NULL;
	}

	// Get the size of the config array and allocate the widget array.
	*count = cJSON_GetArraySize (config);
	canWidget_t** widgetArray = malloc (sizeof (canWidget_t*) * *count);
	if (widgetArray == NULL)
		return NULL;

	// Load each individual widget.
	for (size_t index = 0; index < *count; ++index)
	{
		cJSON* widgetConfig = cJSON_GetArrayItem (config, index);
		widgetArray [index] = canWidgetLoad (database, widgetConfig, style);
	}

	return widgetArray;
}

#define LOAD_STYLE_STATEMENTS(type)																							\
	styleConfig = config != NULL ? jsonGetObjectV2 (config, STRINGIFY (type)) : NULL;										\
	CONCETENATE (type, LoadStyle) (styleConfig, &style->type, parent != NULL ? &parent->type : NULL);

void canWidgetLoadStyle (canWidgetStyle_t* style, cJSON* config, canWidgetStyle_t* parent)
{
	cJSON* styleConfig;

	LOAD_STYLE_STATEMENTS (canFaultPopup);
	LOAD_STYLE_STATEMENTS (canFloatPopup);
	LOAD_STYLE_STATEMENTS (canIndicator);
	LOAD_STYLE_STATEMENTS (canProgressBar);
	LOAD_STYLE_STATEMENTS (canShutdownLoopIndicator);
}