// Header
#include "page_can_bus.h"

// Includes
#include "../stylized_widgets/stylized_button.h"
#include "../stylized_widgets/stylized_frame.h"
#include "../gtk_util.h"
#include "can_device/can_device_stdio.h"
#include "can_database/can_database_stdio.h"
#include "cjson/cjson_util.h"
#include "debug.h"

static void drawBg (GtkDrawingArea* area, cairo_t* cr, int width, int height, gpointer arg)
{
	pageCanBus_t* page = arg;
	(void) area;
	(void) arg;

	gdk_cairo_set_source_rgba (cr, &page->style.baseStyle->backgroundColor);
	cairo_rectangle (cr, 0, 0, width, height);
	cairo_fill (cr);
}

static void styleLoad (pageCanBusStyle_t* style, pageStyle_t* baseStyle, cJSON* config)
{
	*style = (pageCanBusStyle_t)
	{
		.baseStyle = baseStyle
	};

	if (config == NULL)
		return;

	style->baseStyle = pageStyleLoad (jsonGetObjectV2 (config, "baseStyle"), baseStyle);

	char* color;
	if (jsonGetString (config, "terminalBackgroundColor", &color) == 0)
		style->terminalBackgroundColor = gdkHexToColor (color);
}

static void appendButton (void* pageArg, const char* label, pageButtonCallback_t* callback, void* arg, bool currentPage, pageStyle_t* style)
{
	pageCanBus_t* page = pageArg;

	stylizedButton_t* button = stylizedButtonInit (callback, arg, &(stylizedButtonConfig_t)
	{
		.width				= 100,
		.height				= style->buttonHeight,
		.label				= label,
		.borderThickness	= style->borderThickness,
		.backgroundColor	= style->backgroundColor,
		.borderColor		= style->borderColor,
		.selectedColor		= style->fontColor,
		.indicatorColor		= currentPage ?
			page->style.baseStyle->indicatorActiveColor : page->style.baseStyle->indicatorInactiveColor
	});
	gtkLabelSetFont (STYLIZED_BUTTON_TO_LABEL (button), style->buttonFont);
	gtk_widget_set_margin_top (STYLIZED_BUTTON_TO_WIDGET (button), 8);
	gtk_widget_set_margin_start (STYLIZED_BUTTON_TO_WIDGET (button), 4);
	gtk_widget_set_margin_end (STYLIZED_BUTTON_TO_WIDGET (button), 4);
	gtk_widget_set_hexpand (STYLIZED_BUTTON_TO_WIDGET (button), true);
	gtk_grid_attach (GTK_GRID (page->buttonPanel), STYLIZED_BUTTON_TO_WIDGET (button), page->buttonCount, 0, 1, 1);
	++page->buttonCount;
}

static void update (void* pageArg)
{
	pageCanBus_t* page = pageArg;

	size_t bufferSize;
	char* buffer = stylizedTerminalGetBuffer (page->term, &bufferSize);
	if (buffer == NULL)
		return;

	if (stylizedTerminalSnprintf (page->term, &buffer, &bufferSize, "%-36s | %-16s | Unit", "Message / Signal Name", "Value") != 0)
		return;

	if (stylizedTerminalPrintNewline (page->term, &buffer, &bufferSize) != 0)
		return;

	for (size_t index = 0; index < bufferSize - 1; ++index)
		buffer [index] = '-';
	buffer [bufferSize - 1] = '\0';

	if (stylizedTerminalPrintNewline (page->term, &buffer, &bufferSize) != 0)
		return;

	int scroll = stylizedTerminalGetScrollPosition (page->term);
	size_t messageCount = canDatabaseGetMessageCount (page->database);
	for (size_t messageIndex = 0; messageIndex < messageCount; ++messageIndex)
	{
		canMessage_t* message = canDatabaseGetMessage (page->database, messageIndex);

		if (scroll <= 0)
		{
			if (stylizedTerminalSnprintf (page->term, &buffer, &bufferSize, "%s - ", message->name) != 0)
				return;

			if (stylizedTerminalSnprintCallback (page->term, buffer, bufferSize, snprintCanId, message->id, message->ide, false) != 0)
				return;

			if (stylizedTerminalSnprintf (page->term, &buffer, &bufferSize, ":") != 0)
				return;

			if (stylizedTerminalPrintNewline (page->term, &buffer, &bufferSize) != 0)
				return;
		}
		else
			--scroll;

		for (size_t signalIndex = 0; signalIndex < message->signalCount; ++signalIndex)
		{
			canSignal_t* signal = &message->signals [signalIndex];

			if (scroll <= 0)
			{
				size_t globalIndex = canDatabaseGetGlobalIndex (page->database, messageIndex, signalIndex);
				if (stylizedTerminalSnprintf (page->term, &buffer, &bufferSize, "    %-32s | ", signal->name) != 0)
					return;

				if (stylizedTerminalSnprintCallback (page->term, buffer, bufferSize, snprintCanDatabaseFloat, "%16f | %s", "%16s | %s", page->database, globalIndex) != 0)
					return;

				if (stylizedTerminalPrintNewline (page->term, &buffer, &bufferSize) != 0)
					return;
			}
			else
				--scroll;
		}
	}

	stylizedTerminalWriteBuffer (page->term);
}

page_t* pageCanBusLoad (cJSON* config, canDatabase_t* database, pageStyle_t* style)
{
	if (config == NULL)
		return NULL;

	char* pageName;
	if (jsonGetString (config, "name", &pageName) != 0)
		return NULL;

	// Allocate the page
	pageCanBus_t* page = malloc (sizeof (pageCanBus_t));
	if (page == NULL)
		return NULL;

	// Setup the VMT
	page->vmt = (pageVmt_t)
	{
		.update			= update,
		.appendButton	= appendButton,
		.widget			= gtk_overlay_new (),
		.name			= pageName,
		.parent			= NULL
	};

	cJSON* styleConfig = jsonGetObjectV2 (config, "style");
	styleLoad (&page->style, style, styleConfig);

	page->database = database;

	GtkWidget* bg = gtk_drawing_area_new ();
	gtk_drawing_area_set_draw_func (GTK_DRAWING_AREA (bg), drawBg, page, NULL);
	gtk_overlay_set_child (GTK_OVERLAY (page->vmt.widget), bg);

	page->grid = gtk_grid_new ();
	gtk_overlay_add_overlay (GTK_OVERLAY (page->vmt.widget), page->grid);
	gtk_overlay_set_measure_overlay (GTK_OVERLAY (page->vmt.widget), page->grid, true);

	stylizedFrame_t* frame = stylizedFrameInit (&(stylizedFrameConfig_t)
	{
		.backgroundColor	= page->style.terminalBackgroundColor,
		.borderColor		= page->style.baseStyle->borderColor,
		.borderThickness	= page->style.baseStyle->borderThickness,
		.cornerRadius		= 0
	});
	gtk_widget_set_hexpand (STYLIZED_FRAME_TO_WIDGET (frame), true);
	gtk_widget_set_vexpand (STYLIZED_FRAME_TO_WIDGET (frame), true);
	gtk_grid_attach (GTK_GRID (page->grid), STYLIZED_FRAME_TO_WIDGET (frame), 0, 0, 1, 1);

	page->term = stylizedTerminalInit (&(stylizedTerminalConfig_t)
	{
		.lineLengthMax			= 128,
		.lineCountMax			= 64,
		.fontSize				= 14,
		.fontSpacing			= 2,
		.scrollEnabled			= true,
		.scrollMin				= 0,
		.scrollMax				= canDatabaseGetMessageCount (database) + canDatabaseGetSignalCount (database),
		.backgroundColor		= page->style.terminalBackgroundColor,
		.fontColor				= page->style.baseStyle->fontColor
	});
	gtk_widget_set_margin_top (STYLIZED_TERMINAL_TO_WIDGET (page->term), 4);
	gtk_widget_set_margin_bottom (STYLIZED_TERMINAL_TO_WIDGET (page->term), 4);
	gtk_widget_set_margin_start (STYLIZED_TERMINAL_TO_WIDGET (page->term), 4);
	gtk_widget_set_margin_end (STYLIZED_TERMINAL_TO_WIDGET (page->term), 4);
	stylizedFrameSetChild (frame, STYLIZED_TERMINAL_TO_WIDGET (page->term));

	page->buttonCount = 0;
	page->buttonPanel = gtk_grid_new ();
	gtk_widget_set_margin_bottom (GTK_WIDGET (page->buttonPanel), 10);
	gtk_widget_set_margin_start (GTK_WIDGET (page->buttonPanel), 100);
	gtk_widget_set_margin_end (GTK_WIDGET (page->buttonPanel), 20);
	gtk_grid_attach (GTK_GRID (page->grid), page->buttonPanel, 0, 1, 1, 1);

	// Return the created page (cast to the base type).
	return (page_t*) page;
}