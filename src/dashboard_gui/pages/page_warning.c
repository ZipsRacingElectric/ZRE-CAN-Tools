// Header
#include "page_warning.h"

// Includes
#include "../page_stack.h"
#include "cjson/cjson_util.h"
#include "misc_port.h"
#include "debug.h"

#define PAGE_WARNING_BUTTON_COUNT 7

static const char* sibButtonSignals[PAGE_WARNING_BUTTON_COUNT] =
{
	"SIB_BUTTON_MID_LEFT",
	"SIB_BUTTON_BOTTOM_LEFT",
	"SIB_BUTTON_MID_RIGHT",
	"SIB_BUTTON_BOTTOM_RIGHT",
	"SIB_BUTTON_TOP_LEFT",
	"SIB_SWITCH_TOP_RIGHT_DOWN",
	"SIB_SWITCH_TOP_RIGHT_UP"
};

static void drawBg (GtkDrawingArea* area, cairo_t* cr, int width, int height, gpointer arg)
{
	pageWarning_t* page = arg;
	(void) area;
	(void) arg;

	gdk_cairo_set_source_rgba (cr, &page->style.baseStyle->backgroundColor);
	cairo_rectangle (cr, 0, 0, width, height);
	cairo_fill (cr);
}

static void styleLoad (pageWarningStyle_t* style, pageStyle_t* baseStyle, cJSON* config)
{
	*style = (pageWarningStyle_t)
	{
		.baseStyle = baseStyle
	};

	if (config == NULL)
		return;

	style->baseStyle = pageStyleLoad (jsonGetObjectV2 (config, "baseStyle"), baseStyle);
}

static void appendButton (void* pageArg, const char* label, pageButtonCallback_t* callback, void* arg, bool currentPage, pageStyle_t* style)
{
	(void) pageArg;
	(void) label;
	(void) callback;
	(void) arg;
	(void) currentPage;
	(void) style;
}

static void update (void* page)
{
	pageWarning_t* warning = page;

	canWidgetUpdateArray (warning->warnings, warning->warningCount);

	bool buttonPressed = false;

	for (size_t index = 0; index < PAGE_WARNING_BUTTON_COUNT; ++index)
	{
		float value;

		if (canDatabaseGetFloat(warning->sibDatabase, warning->sibButtonIndices[index], &value ) == CAN_DATABASE_VALID)
		{
			if (value != 0.0f)
			{
				buttonPressed = true;
				break;
			}
		}
	}

	// Establish the initial button state when the warning page is shown.
	// This prevents a button already being held from immediately dismissing the warning.
	if (!warning->buttonStateInitialized)
	{
		warning->buttonPressedLast = buttonPressed;
		warning->buttonStateInitialized = true;
		return;
	}

	// Dismiss warning on a new button pressed
	if (buttonPressed && !warning->buttonPressedLast)
	{
		pageStack_t* stack = pageGetParent ((page_t*) warning);

		if (stack != NULL)	
			pageStackReturnFromWarning(stack);
	}

	warning->buttonPressedLast = buttonPressed;
	 
}

page_t* pageWarningLoad (cJSON* config, canDatabase_t* databases, size_t databaseCount, pageStyle_t* style)
{

	if (config == NULL)
		return NULL;

	char* pageName;
	if (jsonGetString (config, "name", &pageName) != 0)
		return NULL;

	pageWarning_t* page = malloc (sizeof (pageWarning_t));
	if (page == NULL)
		return NULL;

	// Load SIB CAN button signals

cJSON* sibConfig = jsonGetObjectV2 (config, "sibInput");

if (sibConfig == NULL)
{
	errorPrintf ("Warning page is missing sibInput configuration.\n");
	free (page);
	return NULL;
}

unsigned deviceIndex;

if (jsonGetUnsigned (sibConfig, "deviceIndex", &deviceIndex) != 0)
{
	errorPrintf ("Warning page SIB input is missing deviceIndex.\n");
	free (page);
	return NULL;
}

if (deviceIndex >= databaseCount)
{
	errorPrintf ("Warning page SIB deviceIndex is invalid.\n");
	free (page);
	return NULL;
}

page->sibDatabase = &databases[deviceIndex];

for (size_t index = 0; index < PAGE_WARNING_BUTTON_COUNT; ++index)
{
	page->sibButtonIndices[index] = canDatabaseFindSignal (page->sibDatabase, sibButtonSignals[index]);

	if (page->sibButtonIndices[index] < 0)
	{
		errorPrintf ("Warning page failed to find SIB signal '%s'.\n", sibButtonSignals[index]);
		free (page);
		return NULL;
	}
}

page->buttonPressedLast = false;
page->buttonStateInitialized = false;

	page->warnings = NULL;
	page->warningCount = 0;

	page->vmt = (pageVmt_t)
	{
		.update			= update,
		.appendButton	= appendButton,
		.widget			= gtk_overlay_new (),
		.name			= pageName,
		.parent			= NULL
	};

	// Draw a blank black background
	GtkWidget* bg = gtk_drawing_area_new ();
	gtk_drawing_area_set_draw_func (GTK_DRAWING_AREA (bg), drawBg, page, NULL);
	gtk_overlay_set_child (GTK_OVERLAY (page->vmt.widget), bg);

	cJSON* styleConfig = jsonGetObjectV2 (config, "style");
	styleLoad (&page->style, style, styleConfig);

	// Background image
	char* backgroundPath;
	if (jsonGetString (config, "warningBackground", &backgroundPath) != 0)
		return NULL;

	char* backgroundPathExp = expandEnv (backgroundPath);
	free (backgroundPath);

	if (backgroundPathExp == NULL)
		return NULL;

	GtkWidget* warningBackground = gtk_picture_new_for_filename (backgroundPathExp);

	free (backgroundPathExp);

	gtk_picture_set_can_shrink (GTK_PICTURE (warningBackground), false);
	//gtk_picture_set_content_fit (GTK_PICTURE (warningBackground), GTK_CONTENT_FIT_FILL);

	gtk_widget_set_halign (warningBackground, GTK_ALIGN_CENTER);
	gtk_widget_set_valign (warningBackground, GTK_ALIGN_CENTER);
	gtk_widget_set_hexpand (warningBackground, true);
	gtk_widget_set_vexpand (warningBackground, true);

	gtk_overlay_set_child (GTK_OVERLAY (page->vmt.widget), warningBackground);

	// Warning widget overlay
	GtkWidget* warningWidgets = gtk_fixed_new ();

	gtk_widget_set_halign (warningWidgets, GTK_ALIGN_START);
	gtk_widget_set_valign (warningWidgets, GTK_ALIGN_START);
	gtk_widget_set_hexpand (warningWidgets, true);
	gtk_widget_set_vexpand (warningWidgets, true);

	gtk_overlay_add_overlay (GTK_OVERLAY (page->vmt.widget), warningWidgets);

	// Load warning configurations
	cJSON* warningConfigs = jsonGetObjectV2 (config, "warnings");

	if (warningConfigs == NULL)
		return (page_t*) page;

	page->warningCount = cJSON_GetArraySize (warningConfigs);

	if (page->warningCount == 0)
		return (page_t*) page;

	page->warnings = calloc (page->warningCount, sizeof (canWidget_t*));

	if (page->warnings == NULL)
		return NULL;

	page->triggerDatabase = calloc (page->warningCount, sizeof (canDatabase_t*));
	page->triggerIndices = calloc (page->warningCount, sizeof (ssize_t));

	if (page->triggerDatabase == NULL || page->triggerIndices == NULL)
		return NULL;

	// Load CAN warning widgets
	for (size_t index = 0; index < page->warningCount; ++index)
	{
		cJSON* warningConfig = cJSON_GetArrayItem (warningConfigs, index);

		if (warningConfig == NULL)
			continue;

		uint16_t x = 0;
		jsonGetUint16_t (warningConfig, "xPosition", &x);

		uint16_t y = 0;
		jsonGetUint16_t (warningConfig, "yPosition", &y);

		cJSON* widgetConfig = jsonGetObjectV2 (warningConfig, "widget");

		if (widgetConfig == NULL)
			continue;

		page->triggerIndices [index] = -1;

		unsigned deviceIndex;
		char* signalName;
		if (jsonGetUnsigned(widgetConfig, "deviceIndex", &deviceIndex) == 0 &&
			jsonGetString (widgetConfig, "signalName", &signalName) == 0 &&
			deviceIndex < databaseCount)
		{
			page->triggerDatabase [index] = &databases [deviceIndex];
			page->triggerIndices [index] = canDatabaseFindSignal(&databases [deviceIndex], signalName);
		}

		page->warnings [index] = canWidgetLoad (databases, databaseCount, widgetConfig, PAGE_WIDGET_STYLE (page));

		if (page->warnings [index] == NULL)
			continue;

		gtk_fixed_put (GTK_FIXED (warningWidgets), CAN_WIDGET_TO_WIDGET (page->warnings [index]), x, y);
	}

	return (page_t*) page;
}

bool pageWarningIsActive(page_t* pageBase) 
{
	pageWarning_t* page = (pageWarning_t*) pageBase;

	for (size_t index = 0; index < page->warningCount; ++index) 
	{
		if (page->triggerIndices [index] < 0)
			continue;

		float value;
		if (canDatabaseGetFloat (page->triggerDatabase [index], page->triggerIndices [index], &value) != CAN_DATABASE_VALID)
			continue;

		if ((uint8_t) value != 0x00)
			return true;
	}
	return false;
}

void pageWarningResetButtonState(page_t *pageBase)
{
	pageWarning_t* warning = (pageWarning_t*) pageBase;

	warning->buttonPressedLast = false;
	warning->buttonStateInitialized = false;
}