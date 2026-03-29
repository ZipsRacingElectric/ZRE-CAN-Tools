// Header
#include "page_can_bus.h"

// Includes
#include "../stylized_widgets/stylized_button.h"
#include "../stylized_widgets/stylized_frame.h"
#include "../gtk_util.h"
#include "can_device/can_device_stdio.h"
#include "can_database/can_database_stdio.h"
#include "cjson/cjson_util.h"

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
		.baseStyle					= baseStyle,
		.terminalBackgroundColor	= gdkHexToColor ("#000000"),
		.animationTime				= 0.5f
	};

	if (config == NULL)
		return;

	style->baseStyle = pageStyleLoad (jsonGetObjectV2 (config, "baseStyle"), baseStyle);

	char* color;
	if (jsonGetString (config, "terminalBackgroundColor", &color) == 0)
		style->terminalBackgroundColor = gdkHexToColor (color);

	jsonGetFloat (config, "animationTime", &style->animationTime);
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
			page->style.baseStyle->buttonActiveColor : page->style.baseStyle->buttonInactiveColor,
		.useIndicator		= true
	});
	if (style->buttonFont != NULL)
		gtkLabelSetFont (STYLIZED_BUTTON_TO_LABEL (button), style->buttonFont);
	gtk_widget_set_margin_top (STYLIZED_BUTTON_TO_WIDGET (button), 8);
	gtk_widget_set_margin_start (STYLIZED_BUTTON_TO_WIDGET (button), 4);
	gtk_widget_set_margin_end (STYLIZED_BUTTON_TO_WIDGET (button), 4);
	gtk_widget_set_hexpand (STYLIZED_BUTTON_TO_WIDGET (button), true);
	gtk_grid_attach (GTK_GRID (page->buttonPanel), STYLIZED_BUTTON_TO_WIDGET (button), page->buttonCount, 0, 1, 1);
	++page->buttonCount;
}

static inline canDatabase_t* getDatabase (pageCanBus_t* page)
{
	return &page->databases [page->dbcIndex];
}

static void update (void* pageArg)
{
	pageCanBus_t* page = pageArg;
	canDatabase_t* database = getDatabase (page);

	size_t bufferSize;
	char* buffer = stylizedTerminalGetBuffer (page->term, &bufferSize);
	if (buffer == NULL)
		return;

	// If the animation timer hasn't expired, display the DBC name
	if (page->animationTimer < page->style.animationTime)
	{
		// Rougly 30 FPS
		page->animationTimer += 1 / 30.0f;

		int lineCount = stylizedTerminalGetLineCount (page->term);

		for (int index = 0; index < lineCount / 2 - 1; ++index)
		{
			if (bufferSize == 0)
				return;

			buffer [0] = '\0';

			if (stylizedTerminalPrintNewline (page->term, &buffer, &bufferSize) != 0)
				return;
		}

		for (size_t index = 0; index < bufferSize - 1; ++index)
			buffer [index] = '-';
		buffer [bufferSize - 1] = '\0';

		if (stylizedTerminalPrintNewline (page->term, &buffer, &bufferSize) != 0)
			return;

		char* name = canDatabaseGetName (database);

		int padding = (bufferSize - strlen (name)) / 2;
		for (int index = 0; index < padding; ++index)
		{
			*buffer = ' ';
			++buffer;
			--bufferSize;
		}

		if (stylizedTerminalSnprintf (page->term, &buffer, &bufferSize, "%s", name) != 0)
			return;

		if (stylizedTerminalPrintNewline (page->term, &buffer, &bufferSize) != 0)
			return;

		for (size_t index = 0; index < bufferSize - 1; ++index)
			buffer [index] = '-';
		buffer [bufferSize - 1] = '\0';

		if (stylizedTerminalPrintNewline (page->term, &buffer, &bufferSize) != 0)
			return;

		stylizedTerminalWriteBuffer (page->term);

		return;
	}

	// Otherwise, display the DBC contents.

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
	size_t messageCount = canDatabaseGetMessageCount (database);
	for (size_t messageIndex = 0; messageIndex < messageCount; ++messageIndex)
	{
		canMessage_t* message = canDatabaseGetMessage (database, messageIndex);

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
				size_t globalIndex = canDatabaseGetGlobalIndex (database, messageIndex, signalIndex);
				if (stylizedTerminalSnprintf (page->term, &buffer, &bufferSize, "    %-32s | ", signal->name) != 0)
					return;

				if (stylizedTerminalSnprintCallback (page->term, buffer, bufferSize, snprintCanDatabaseFloat, "%16f | %s", "%16s | %s", page->databases, globalIndex) != 0)
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

static void dbcButtonCallback (GtkWidget* widget, gpointer data)
{
	(void) widget;
	pageCanBus_t* page = data;

	// Increment the DBC to use
	page->dbcIndex = (page->dbcIndex + 1) % page->databaseCount;

	// Update the terminal bounds
	canDatabase_t* database = getDatabase (page);
	stylizedTerminalSetScrollMax (page->term, canDatabaseGetMessageCount (database) + canDatabaseGetSignalCount (database));

	// Reset the animation timer
	page->animationTimer = 0.0f;
}

page_t* pageCanBusLoad (cJSON* config, canDatabase_t* databases, size_t databaseCount, pageStyle_t* style)
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

	page->databases			= databases;
	page->databaseCount		= databaseCount;
	page->dbcIndex			= 0;
	page->animationTimer	= page->style.animationTime;

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
	gtk_grid_attach (GTK_GRID (page->grid), STYLIZED_FRAME_TO_WIDGET (frame), 0, 0, 2, 1);

	page->term = stylizedTerminalInit (&(stylizedTerminalConfig_t)
	{
		.lineLengthMax			= 128,
		.lineCountMax			= 64,
		.fontSize				= 14,
		.fontSpacing			= 2,
		.scrollEnabled			= true,
		.scrollMin				= 0,
		.scrollMax				= canDatabaseGetMessageCount (databases) + canDatabaseGetSignalCount (databases),
		.backgroundColor		= page->style.terminalBackgroundColor,
		.fontColor				= page->style.baseStyle->fontColor
	});
	gtk_widget_set_margin_top (STYLIZED_TERMINAL_TO_WIDGET (page->term), 4);
	gtk_widget_set_margin_bottom (STYLIZED_TERMINAL_TO_WIDGET (page->term), 4);
	gtk_widget_set_margin_start (STYLIZED_TERMINAL_TO_WIDGET (page->term), 4);
	gtk_widget_set_margin_end (STYLIZED_TERMINAL_TO_WIDGET (page->term), 4);
	stylizedFrameSetChild (frame, STYLIZED_TERMINAL_TO_WIDGET (page->term));

	stylizedButton_t* dbcButton = stylizedButtonInit (dbcButtonCallback, page, &(stylizedButtonConfig_t)
	{
		.width				= 80,
		.height				= 0,
		.borderThickness	= page->style.baseStyle->borderThickness,
		.label				= "DBC",
		.borderColor		= page->style.baseStyle->borderColor,
		.backgroundColor	= page->style.baseStyle->backgroundColor,
		.selectedColor		= page->style.baseStyle->borderColor,
		.indicatorColor		= page->style.baseStyle->borderColor,
		.useIndicator		= false
	});
	gtkLabelSetFont (STYLIZED_BUTTON_TO_LABEL (dbcButton), page->style.baseStyle->buttonFont);
	gtk_widget_set_margin_top (STYLIZED_BUTTON_TO_WIDGET (dbcButton), 10);
	gtk_widget_set_margin_bottom (STYLIZED_BUTTON_TO_WIDGET (dbcButton), 10);
	gtk_widget_set_margin_start (STYLIZED_BUTTON_TO_WIDGET (dbcButton), 10);
	gtk_grid_attach (GTK_GRID (page->grid), STYLIZED_BUTTON_TO_WIDGET (dbcButton), 0, 1, 1, 1);

	page->buttonCount = 0;
	page->buttonPanel = gtk_grid_new ();
	gtk_widget_set_margin_bottom (GTK_WIDGET (page->buttonPanel), 10);
	gtk_widget_set_margin_start (GTK_WIDGET (page->buttonPanel), 10);
	gtk_widget_set_margin_end (GTK_WIDGET (page->buttonPanel), 20);
	gtk_grid_attach (GTK_GRID (page->grid), page->buttonPanel, 1, 1, 1, 1);

	// Return the created page (cast to the base type).
	return (page_t*) page;
}