// Header
#include "page_can_bus.h"

// Includes
#include "stylized_widgets/stylized_button.h"
#include "stylized_widgets/stylized_frame.h"
#include "gtk_util.h"
#include "can_device/can_device_stdio.h"
#include "can_database/can_database_stdio.h"
#include "debug.h"

page_t* pageCanBusInit (canDatabase_t* database, pageStyle_t* style)
{
	// Allocate the page
	pageCanBus_t* page = malloc (sizeof (pageCanBus_t));
	if (page == NULL)
		return NULL;

	// Setup the VMT
	page->vmt = (pageVmt_t)
	{
		.update			= pageCanBusUpdate,
		.appendButton	= pageCanBusAppendButton,
		.widget			= gtk_overlay_new ()
	};

	page->style = (pageCanBusStyle_t)
	{
		.pageStyle = style,
	};

	page->database = database;

	page->grid = gtk_grid_new ();
	gtk_overlay_add_overlay (GTK_OVERLAY (page->vmt.widget), page->grid);
	gtk_overlay_set_measure_overlay (GTK_OVERLAY (page->vmt.widget), page->grid, true);

	stylizedFrame_t* frame = stylizedFrameInit (&(stylizedFrameConfig_t)
	{
		.backgroundColor	= gdkHexToColor ("#000000"),
		.borderColor		= gdkHexToColor ("#00FFAA"),
		.borderThickness	= 1.5f,
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
		.backgroundColor		= gdkHexToColor ("#00000000"),
		.fontColor				= gdkHexToColor ("#00FFAA"),
		.fontOutlineColor		= gdkHexToColor ("#006600"),
		.fontOutlineThickness	= 0.75f,
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

void pageCanBusAppendButton (void* pageArg, const char* label, pageButtonCallback_t* callback, void* arg, bool currentPage)
{
	pageCanBus_t* page = pageArg;

	stylizedButton_t* button = stylizedButtonInit (callback, arg, &(stylizedButtonConfig_t)
	{
		.width				= 100,
		.height				= page->style.pageStyle->buttonHeight,
		.label				= label,
		.borderThickness	= page->style.pageStyle->borderThickness,
		.backgroundColor	= page->style.pageStyle->backgroundColor,
		.borderColor		= page->style.pageStyle->borderColor,
		.selectedColor		= page->style.pageStyle->fontColor,
		.indicatorColor		= currentPage ?
			page->style.pageStyle->indicatorActiveColor : page->style.pageStyle->indicatorInactiveColor
	});
	gtkLabelSetFont (STYLIZED_BUTTON_TO_LABEL (button), page->style.pageStyle->buttonFont);
	gtk_widget_set_margin_top (STYLIZED_BUTTON_TO_WIDGET (button), 8);
	gtk_widget_set_margin_start (STYLIZED_BUTTON_TO_WIDGET (button), 4);
	gtk_widget_set_margin_end (STYLIZED_BUTTON_TO_WIDGET (button), 4);
	gtk_widget_set_hexpand (STYLIZED_BUTTON_TO_WIDGET (button), true);
	gtk_grid_attach (GTK_GRID (page->buttonPanel), STYLIZED_BUTTON_TO_WIDGET (button), page->buttonCount, 0, 1, 1);
	++page->buttonCount;
}

void pageCanBusUpdate (void* pageArg)
{
	pageCanBus_t* page = pageArg;

	char* buffer = stylizedTerminalGetBuffer (page->term);
	size_t bufferSize = stylizedTerminalGetBufferSize (page->term);
	if (buffer == NULL)
	{
		stylizedTerminalWriteBuffer (page->term);
		return;
	}

	int scroll = stylizedTerminalGetScrollPosition (page->term);
	size_t messageCount = canDatabaseGetMessageCount (page->database);
	for (size_t messageIndex = 0; messageIndex < messageCount; ++messageIndex)
	{
		canMessage_t* message = canDatabaseGetMessage (page->database, messageIndex);

		if (scroll <= 0)
		{
			int code = snprintf (buffer, bufferSize, "%s - ", message->name);
			if (code < 0 || (size_t) code >= bufferSize)
			{
				debugPrintf ("Failed to print CAN database, buffer overflow.\n");
				stylizedTerminalWriteBuffer (page->term);
				return;
			}
			buffer += code;
			bufferSize -= code;

			code = snprintCanId (buffer, bufferSize, message->id, message->ide, false);
			if (code < 0 || (size_t) code >= bufferSize)
			{
				debugPrintf ("Failed to print CAN database, buffer overflow.\n");
				stylizedTerminalWriteBuffer (page->term);
				return;
			}
			buffer += code;
			bufferSize -= code;

			code = snprintf (buffer, bufferSize, ":");
			if (code < 0 || (size_t) code >= bufferSize)
			{
				debugPrintf ("Failed to print CAN database, buffer overflow.\n");
				stylizedTerminalWriteBuffer (page->term);
				return;
			}
			buffer = stylizedTerminalNextLine (page->term);
			if (buffer == NULL)
			{
				stylizedTerminalWriteBuffer (page->term);
				return;
			}
			bufferSize = stylizedTerminalGetBufferSize (page->term);
		}
		else
			--scroll;

		for (size_t signalIndex = 0; signalIndex < message->signalCount; ++signalIndex)
		{
			canSignal_t* signal = &message->signals [signalIndex];

			if (scroll <= 0)
			{
				size_t globalIndex = canDatabaseGetGlobalIndex (page->database, messageIndex, signalIndex);
				int code = snprintf (buffer, bufferSize, "    %-32s | ", signal->name);
				if (code < 0 || (size_t) code >= bufferSize)
				{
					debugPrintf ("Failed to print CAN database, buffer overflow.\n");
					stylizedTerminalWriteBuffer (page->term);
					return;
				}
				buffer += code;
				bufferSize -= code;

				code = snprintCanDatabaseFloat (buffer, bufferSize, "%16f | %s", "%16s | %s", page->database, globalIndex);
				if (code < 0 || (size_t) code >= bufferSize)
				{
					debugPrintf ("Failed to print CAN database, buffer overflow.\n");
					stylizedTerminalWriteBuffer (page->term);
					return;
				}
				buffer = stylizedTerminalNextLine (page->term);
				if (buffer == NULL)
				{
					stylizedTerminalWriteBuffer (page->term);
					return;
				}
				bufferSize = stylizedTerminalGetBufferSize (page->term);
			}
			else
				--scroll;
		}
	}

	stylizedTerminalWriteBuffer (page->term);
}