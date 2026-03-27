// Header
#include "can_widget.h"

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

canWidget_t* canWidgetLoad (canDatabase_t* databases, size_t databaseCount, cJSON* config, canWidgetStyle_t* style)
{
	if (config == NULL)
		return NULL;

	char* widgetType;
	if (jsonGetString (config, "type", &widgetType) != 0)
	{
		debugPrintf ("Warning, CAN widget is missing type.\n");
		return NULL;
	}

	unsigned deviceIndex;
	if (jsonGetUnsigned (config, "deviceIndex", &deviceIndex) != 0)
	{
		debugPrintf ("Warning, CAN widget is missing device index.\n");
		return NULL;
	}

	if (deviceIndex >= databaseCount)
	{
		debugPrintf ("Warning, CAN widget specifies invalid device index %u. Device count is %lu.\n",
			deviceIndex, (long unsigned) databaseCount);
		return NULL;
	}

	canDatabase_t* database = &databases [deviceIndex];

	LOAD_WIDGET_STATEMENTS (canFaultPopup);
	LOAD_WIDGET_STATEMENTS (canFloatPopup);
	LOAD_WIDGET_STATEMENTS (canIndicator);
	LOAD_WIDGET_STATEMENTS (canLabelBool);
	LOAD_WIDGET_STATEMENTS (canLabelFault);
	LOAD_WIDGET_STATEMENTS (canLabelFloat);
	LOAD_WIDGET_STATEMENTS (canProgressBar);
	LOAD_WIDGET_STATEMENTS (canShutdownLoopIndicator);
	LOAD_WIDGET_STATEMENTS (canWidgetTemplate);

	if (strcmp (widgetType, "null") == 0)
		return NULL;

	fprintf (stderr, "Failed to load widget of unknown type '%s'.\n", widgetType);
	return NULL;
}

canWidget_t** canWidgetLoadArray (canDatabase_t* databases, size_t databaseCount, cJSON* config, canWidgetStyle_t* style, size_t* count)
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
		widgetArray [index] = canWidgetLoad (databases, databaseCount, widgetConfig, style);
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
	LOAD_STYLE_STATEMENTS (canLabelBool);
	LOAD_STYLE_STATEMENTS (canLabelFault);
	LOAD_STYLE_STATEMENTS (canLabelFloat);
	LOAD_STYLE_STATEMENTS (canProgressBar);
	LOAD_STYLE_STATEMENTS (canShutdownLoopIndicator);
	LOAD_STYLE_STATEMENTS (canWidgetTemplate);
}