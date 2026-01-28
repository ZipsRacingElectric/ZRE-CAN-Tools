// Header
#include "stylized_frame.h"

static void draw (GtkDrawingArea* area, cairo_t* cr, int width, int height, gpointer arg)
{
	(void) area;
	stylizedFrame_t* frame = arg;

	float x0 = frame->config.borderThickness / 2.0f;
	float y0 = frame->config.borderThickness / 2.0f;
	float x3 = width - frame->config.borderThickness;
	float y3 = height - frame->config.borderThickness;
	float x1 = x0 + frame->config.cornerRadius;
	float y1 = y0 + frame->config.cornerRadius;
	float x2 = x3 - frame->config.cornerRadius;
	float y2 = y3 - frame->config.cornerRadius;

	gdk_cairo_set_source_rgba (cr, &frame->config.backgroundColor);
	cairo_move_to (cr, x1, y0);
	cairo_line_to (cr, x2, y0);
	cairo_arc (cr, x2, y1, frame->config.cornerRadius, -G_PI_2, 0);
	cairo_line_to (cr, x3, y2);
	cairo_arc (cr, x2, y2, frame->config.cornerRadius, 0, G_PI_2);
	cairo_line_to (cr, x1, y3);
	cairo_arc (cr, x1, y2, frame->config.cornerRadius, G_PI_2, G_PI);
	cairo_line_to (cr, x0, y1);
	cairo_arc (cr, x1, y1, frame->config.cornerRadius, G_PI, 3 * G_PI_2);
	cairo_fill_preserve (cr);

	gdk_cairo_set_source_rgba (cr, &frame->config.borderColor);
	cairo_set_line_width (cr, frame->config.borderThickness);
	cairo_stroke (cr);
}

stylizedFrame_t* stylizedFrameInit (stylizedFrameConfig_t* config)
{
	// Allocate the widget.
	stylizedFrame_t* frame = malloc (sizeof (stylizedFrame_t));
	if (frame == NULL)
		return NULL;

	// Initialize the widget
	*frame = (stylizedFrame_t)
	{
		.widget = gtk_overlay_new (),
		.config = *config,
	};

	GtkWidget* drawingArea = gtk_drawing_area_new ();
	gtk_overlay_set_child (GTK_OVERLAY (frame->widget), drawingArea);

	// Set the drawing area size and drawing function
	gtk_drawing_area_set_draw_func (GTK_DRAWING_AREA (drawingArea), draw, frame, NULL);

	return frame;
}

void stylizedFrameSetChild (stylizedFrame_t* frame, GtkWidget* child)
{
	gtk_overlay_add_overlay (GTK_OVERLAY (frame->widget), child);
	gtk_overlay_set_measure_overlay (GTK_OVERLAY (frame->widget), child, true);
}