// Header
#include "page.h"

// Include
#include "gtk_util.h"
#include "cjson/cjson_util.h"

pageStyle_t* pageStyleLoad (cJSON* config, pageStyle_t* parent)
{
	pageStyle_t* style = malloc (sizeof (pageStyle_t));
	if (style == NULL)
		return NULL;

	// Set to default values
	if (parent != NULL)
		*style = *parent;
	else
		*style = (pageStyle_t) {0};

	if (config == NULL)
		return style;

	char* backgroundColor;
	if (jsonGetString (config, "backgroundColor", &backgroundColor) == 0)
		style->backgroundColor = gdkHexToColor (backgroundColor);

	char* fontColor;
	if (jsonGetString (config, "fontColor", &fontColor) == 0)
		style->fontColor = gdkHexToColor (fontColor);

	char* borderColor;
	if (jsonGetString (config, "borderColor", &borderColor) == 0)
		style->borderColor = gdkHexToColor (borderColor);

	jsonGetFloat (config, "borderThickness", &style->borderThickness);

	char* indicatorActiveColor;
	if (jsonGetString (config, "indicatorActiveColor", &indicatorActiveColor) == 0)
		style->indicatorActiveColor = gdkHexToColor (indicatorActiveColor);

	char* indicatorInactiveColor;
	if (jsonGetString (config, "indicatorInactiveColor", &indicatorInactiveColor) == 0)
		style->indicatorInactiveColor = gdkHexToColor (indicatorInactiveColor);

	uint16_t buttonHeight;
	if (jsonGetUint16_t (config, "buttonHeight", &buttonHeight) == 0)
		style->buttonHeight = buttonHeight;

	jsonGetString (config, "buttonFont", &style->buttonFont);

	return style;
}