// Header
#include "stylized_button.h"

// Includes
#include "../gtk_util.h"

static void draw (GtkDrawingArea* area, cairo_t* cr, int width, int height, gpointer arg)
{
	(void) area;
	stylizedButton_t* button = arg;

	// Draw the background

	GdkRGBA* bg = button->held ? &button->config.selectedColor : &button->config.backgroundColor;
	gdk_cairo_set_source_rgba (cr, bg);
	cairo_rectangle (cr, 0, 0, width, height);
	cairo_fill (cr);

	// Draw the indicator

	float xi = 5;
	float wi = width - 10;
	float yi = height * 0.7f;
	float hi = height * 0.2f;

	gdk_cairo_set_source_rgba (cr, &button->config.indicatorColor);
	cairo_rectangle (cr, xi, yi, wi, hi);
	cairo_fill (cr);

	// Draw the frame

	cairo_set_line_width (cr, button->config.borderThickness);

	gdk_cairo_set_source_rgba (cr, &button->config.borderColor);
	cairo_rectangle (cr,
		button->config.borderThickness / 2.0f, button->config.borderThickness / 2.0f,
		width - button->config.borderThickness, height - button->config.borderThickness);
	cairo_stroke (cr);

	// Draw the indicator frame

	cairo_rectangle (cr, xi, yi, wi, hi);
	cairo_stroke (cr);
}

static void pressed (GtkGestureClick* self, gint nPress, gdouble x, gdouble y, gpointer arg)
{
	(void) self;
	(void) nPress;
	(void) x;
	(void) y;
	stylizedButton_t* button = arg;

	// Select the button and redraw
	button->held = true;
	gtkLabelSetColor (button->label, &button->config.backgroundColor);
	gtk_widget_queue_draw (GTK_WIDGET (button->drawingArea));
}

static void released (GtkGestureClick* self, gint nPress, gdouble x, gdouble y, gpointer arg)
{
	(void) self;
	(void) nPress;
	(void) x;
	(void) y;
	stylizedButton_t* button = arg;

	// Check the mouse was released in the widget's boundaries

	graphene_rect_t bounds;
	if (!gtk_widget_compute_bounds (STYLIZED_BUTTON_TO_WIDGET (button), STYLIZED_BUTTON_TO_WIDGET (button), &bounds))
		bounds = *graphene_rect_zero ();

	if (x >= 0 && x <= bounds.size.width && y >= 0 && y <= bounds.size.height && button->callback != NULL)
		button->callback (STYLIZED_BUTTON_TO_WIDGET (button), button->arg);

	// Unselect the button and redraw
	button->held = false;
	gtkLabelSetColor (button->label, &button->config.selectedColor);
	gtk_widget_queue_draw (GTK_WIDGET (button->drawingArea));
}

stylizedButton_t* stylizedButtonInit (stylizedButtonCallback_t* callback, void* arg, stylizedButtonConfig_t* config)
{
	// Allocate the widget.
	stylizedButton_t* button = malloc (sizeof (stylizedButton_t));
	if (button == NULL)
		return NULL;

	// Initialize the widget
	*button = (stylizedButton_t)
	{
		.config			= *config,
		.callback		= callback,
		.arg			= arg,
		.widget			= gtk_overlay_new (),
		.drawingArea	= GTK_DRAWING_AREA (gtk_drawing_area_new ()),
		.label			= GTK_LABEL (gtk_label_new (config->label)),
		.held			= false
	};

	// Set the label color, size, and alignment.
	gtkLabelSetColor (button->label, &config->selectedColor);
	gtk_widget_set_size_request (GTK_WIDGET (button->label), 0, config->height);
	gtk_label_set_yalign (button->label, 0.25);

	// Set the drawing area size and drawing function
	gtk_drawing_area_set_content_width (button->drawingArea, config->width);
	gtk_drawing_area_set_content_height (button->drawingArea, config->height);
	gtk_drawing_area_set_draw_func (button->drawingArea, draw, button, NULL);

	// Create a click gesture bound to the top-level widget, bind the appropriate signals.
	GtkGesture* gesture = gtk_gesture_click_new ();
	gtk_gesture_single_set_button (GTK_GESTURE_SINGLE (gesture), GDK_BUTTON_PRIMARY);
	g_signal_connect (gesture, "pressed", G_CALLBACK (pressed), button);
	g_signal_connect (gesture, "released", G_CALLBACK (released), button);
	gtk_widget_add_controller (STYLIZED_BUTTON_TO_WIDGET (button), GTK_EVENT_CONTROLLER (gesture));

	// Setup the overlay hierarchy (label overlaid onto drawing area)
	gtk_overlay_set_child (GTK_OVERLAY (button->widget), GTK_WIDGET (button->drawingArea));
	gtk_overlay_add_overlay (GTK_OVERLAY (button->widget), GTK_WIDGET (button->label));

	return button;
}