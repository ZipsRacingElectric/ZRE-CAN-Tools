// Header
#include "page.h"

// Include
#include "page_drive.h"
#include "page_bms.h"
#include "page_can_bus.h"
#include "../gtk_util.h"
#include "cjson/cjson_util.h"
#include "debug.h"

page_t* pageLoad (cJSON* config, canDatabase_t* database, pageStyle_t* style)
{
	char* pageType;
	if (jsonGetString (config, "type", &pageType) != 0)
	{
		fprintf (stderr, "Failed to load page: Page type key is missing.\n");
		return NULL;
	}

	if (strcmp (pageType, "pageDrive_t") == 0)
	{
		page_t* page = pageDriveLoad (config, database, style);
		if (page == NULL)
		{
			errorPrintf ("Failed to load page of type 'pageDrive_t'");
			return NULL;
		}

		return page;
	}

	if (strcmp (pageType, "pageBms_t") == 0)
	{
		page_t* page = pageBmsLoad (config, database, style);
		if (page == NULL)
		{
			errorPrintf ("Failed to load page of type 'pageBms_t'");
			return NULL;
		}

		return page;
	}

	if (strcmp (pageType, "pageCanBus_t") == 0)
	{
		page_t* page = pageCanBusLoad (config, database, style);
		if (page == NULL)
		{
			errorPrintf ("Failed to load page of type 'pageCanBus_t'");
			return NULL;
		}

		return page;
	}

	if (strcmp (pageType, "null") == 0)
		return NULL;

	fprintf (stderr, "Failed to load page of unknown type '%s'.\n", pageType);

	return NULL;
}

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