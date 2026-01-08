// Header
#include "stylized_terminal.h"

static void draw (GtkDrawingArea* area, cairo_t* cr, int width, int height, gpointer arg)
{
	(void) area;
	stylizedTerminal_t* term = arg;

	// Draw the background
	gdk_cairo_set_source_rgba (cr, &term->config.backgroundColor);
	cairo_rectangle (cr, 0, 0, width, height);
	cairo_fill (cr);

	cairo_set_font_size (cr, term->config.fontSize);
	cairo_select_font_face (cr, "Monospace", CAIRO_FONT_SLANT_NORMAL, CAIRO_FONT_WEIGHT_NORMAL);
	gdk_cairo_set_source_rgba (cr, &term->config.fontColor);

	float fontSpacing = term->config.fontSize + term->config.fontSpacing;
	for (size_t index = 0; index < term->linesWritten; ++index)
	{
		char* line = term->buffer + index * (term->config.lineLengthMax + 1);
		cairo_move_to (cr, 0, (index + 1) * fontSpacing);
		cairo_show_text (cr, line);
	}
}

static gboolean keyPress (GtkEventControllerKey* controller, guint keyValue, guint keyCode, GdkModifierType state, stylizedTerminal_t* term)
{
	(void) keyCode;
	(void) state;
	(void) controller;

	switch (keyValue)
	{
	case GDK_KEY_Up:
		if (term->scrollPosition != term->config.scrollMin)
			--term->scrollPosition;
		return TRUE;

	case GDK_KEY_Down:
		if (term->scrollPosition != term->config.scrollMax - 1)
			++term->scrollPosition;
		return TRUE;
	}

	return FALSE;
}

static gboolean scroll (GtkEventControllerScroll* controller, gdouble dx, gdouble dy, stylizedTerminal_t* term)
{
	(void) controller;
	(void) dx;

	term->scrollPosition += (int) dy;

	if (term->scrollPosition < term->config.scrollMin)
		term->scrollPosition = term->config.scrollMin;
	if (term->scrollPosition > term->config.scrollMax - 1)
		term->scrollPosition = term->config.scrollMax - 1;

	return TRUE;
}

static void motion (GtkEventController* gesture, gdouble x, gdouble y, stylizedTerminal_t* term)
{
	(void) gesture;
	(void) x;

	if (!term->pressed)
		return;

	term->scrollPosition = term->scrollPressed - (y - term->yPressed) / (term->config.fontSize + term->config.fontSpacing);
	if (term->scrollPosition < term->config.scrollMin)
		term->scrollPosition = term->config.scrollMin;
	if (term->scrollPosition > term->config.scrollMax - 1)
		term->scrollPosition = term->config.scrollMax - 1;
}

static void pressed (GtkGestureClick* gesture, gint nPress, gdouble x, gdouble y, stylizedTerminal_t* term)
{
	(void) gesture;
	(void) nPress;
	(void) x;

	term->scrollPressed = term->scrollPosition;
	term->yPressed = y;
	term->pressed = true;
}

static void released (GtkGestureClick* gesture, gint nPress, gdouble x, gdouble y, stylizedTerminal_t* term)
{
	(void) gesture;
	(void) nPress;
	(void) x;

	if (!term->pressed)
		return;

	term->pressed = false;
	term->scrollPosition = term->scrollPressed - (y - term->yPressed) / (term->config.fontSize + term->config.fontSpacing);
	if (term->scrollPosition < term->config.scrollMin)
		term->scrollPosition = term->config.scrollMin;
	if (term->scrollPosition > term->config.scrollMax - 1)
		term->scrollPosition = term->config.scrollMax - 1;
}

stylizedTerminal_t* stylizedTerminalInit (stylizedTerminalConfig_t* config)
{
	// Allocate the widget.
	stylizedTerminal_t* term = malloc (sizeof (stylizedTerminal_t));
	if (term == NULL)
		return NULL;

	// Initialize the widget
	*term = (stylizedTerminal_t)
	{
		.widget			= gtk_drawing_area_new (),
		.config			= *config,
		.buffer			= malloc (sizeof (char) * config->lineCountMax * (config->lineLengthMax + 1)),
		.linesWritten	= 0,
		.lineCount		= 0,
		.scrollPosition	= 0,
		.scrollPressed	= 0,
		.yPressed		= 0,
		.pressed		= false
	};

	// If scrolling is enabled, create and bind the appropriate event controllers.
	if (config->scrollEnabled)
	{
		// Controller for up/down keys
		GtkEventController* keyController = gtk_event_controller_key_new ();
		g_signal_connect (keyController, "key-pressed", G_CALLBACK (keyPress), term);
		gtk_widget_add_controller (term->widget, keyController);

		// Controller for mouse scrolling
		GtkEventController* scrollController = gtk_event_controller_scroll_new (GTK_EVENT_CONTROLLER_SCROLL_VERTICAL | GTK_EVENT_CONTROLLER_SCROLL_DISCRETE);
		g_signal_connect (scrollController, "scroll", G_CALLBACK (scroll), term);
		gtk_widget_add_controller (term->widget, scrollController);

		// Controller for click and drag
		GtkGesture* clickController = gtk_gesture_click_new ();
		g_signal_connect (clickController, "pressed", G_CALLBACK (pressed), term);
		g_signal_connect (clickController, "released", G_CALLBACK (released), term);
		gtk_widget_add_controller (term->widget, GTK_EVENT_CONTROLLER (clickController));

		// Controller for click and drag
		GtkEventController* motionController = gtk_event_controller_motion_new ();
		gtk_event_controller_set_propagation_phase (GTK_EVENT_CONTROLLER (motionController), GTK_PHASE_CAPTURE);
		g_signal_connect (motionController, "motion", G_CALLBACK (motion), term);
		gtk_widget_add_controller (term->widget, GTK_EVENT_CONTROLLER (motionController));

		// Make the widget focusable to enable these controllers
		gtk_widget_set_focusable (term->widget, true);
	}

	// Set the drawing area function
	gtk_drawing_area_set_draw_func (GTK_DRAWING_AREA (term->widget), draw, term, NULL);

	return term;
}

char* stylizedTerminalGetBuffer (stylizedTerminal_t* term)
{
	// Reset the number of written lines
	term->linesWritten = 0;

	// Get the size of the widget and calculate the number of writable lines

	int height = gtk_widget_get_size (term->widget, GTK_ORIENTATION_VERTICAL);
	if (height < 0)
		height = 0;

	int lineCount = height / (term->config.fontSize + term->config.fontSpacing);
	if (lineCount < 0)
		lineCount = 0;
	if ((size_t) lineCount > term->config.lineCountMax)
		lineCount = term->config.lineCountMax;
	term->lineCount = lineCount;

	int width = gtk_widget_get_size (term->widget, GTK_ORIENTATION_HORIZONTAL);
	if (width < 0)
		width = 0;

	// TODO(Barach): Terrible constant, won't work with other fonts.
	int lineLength = width / (term->config.fontSize * 0.575f);
	if (lineLength < 0)
		lineLength = 0;
	if ((size_t) lineLength > term->config.lineLengthMax)
		lineLength = term->config.lineLengthMax;
	term->lineLength = lineLength;

	// If no lines can be written, return no buffer
	if (term->lineCount == 0)
		return NULL;

	// Otherwise, return the first line buffer
	return term->buffer;
}

char* stylizedTerminalNextLine (stylizedTerminal_t* term)
{
	++term->linesWritten;
	if (term->linesWritten >= term->lineCount)
		return NULL;

	return term->buffer + term->linesWritten * (term->config.lineLengthMax + 1);
}

void stylizedTerminalWriteBuffer (stylizedTerminal_t* term)
{
	gtk_widget_queue_draw (STYLIZED_TERMINAL_TO_WIDGET (term));
}