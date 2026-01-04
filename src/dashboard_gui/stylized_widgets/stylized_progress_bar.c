// Header
#include "stylized_progress_bar.h"

static void draw (GtkDrawingArea* area, cairo_t* cr, int width, int height, gpointer arg)
{
	(void) area;
	stylizedProgressBar_t* bar = arg;

	// Draw the background
	gdk_cairo_set_source_rgba (cr, &bar->config.backgroundColor);
	cairo_rectangle (cr, 0, 0, width, height);
	cairo_fill (cr);

	// Draw the bar's fill
	gdk_cairo_set_source_rgba (cr, &bar->config.fillColor);
	if (bar->config.orientation == GTK_ORIENTATION_HORIZONTAL)
	{
		if (!bar->config.inverted)
			cairo_rectangle (cr, 0, 0, bar->value * width, height);
		else
			cairo_rectangle (cr, width, 0, -bar->value * width, height);
	}
	else
	{
		if (!bar->config.inverted)
			cairo_rectangle (cr, 0, height, width, -bar->value * height);
		else
			cairo_rectangle (cr, 0, 0, width, bar->value * height);
	}
	cairo_fill (cr);

	// Draw the border
	gdk_cairo_set_source_rgba (cr, &bar->config.borderColor);
	cairo_rectangle (cr,
		bar->config.borderThickness / 2, bar->config.borderThickness / 2,
		width - bar->config.borderThickness, height - bar->config.borderThickness);
	cairo_stroke (cr);
}

stylizedProgressBar_t* stylizedProgressBarInit (stylizedProgressBarConfig_t* config)
{
	// Allocate the widget.
	stylizedProgressBar_t* bar = malloc (sizeof (stylizedProgressBar_t));
	if (bar == NULL)
		return NULL;

	// Initialize the widget
	*bar = (stylizedProgressBar_t)
	{
		.config			= *config,
		.widget			= gtk_drawing_area_new (),
	};

	// Set the drawing area size and drawing function
	gtk_drawing_area_set_content_width (GTK_DRAWING_AREA (bar->widget), config->width);
	gtk_drawing_area_set_content_height (GTK_DRAWING_AREA (bar->widget), config->height);
	gtk_drawing_area_set_draw_func (GTK_DRAWING_AREA (bar->widget), draw, bar, NULL);

	return bar;
}

void stylizedProgressBarSetValue (stylizedProgressBar_t* bar, float value)
{
	// Store the value and queue a redraw
	bar->value = value;
	gtk_widget_queue_draw (STYLIZED_PROGRESS_BAR_TO_WIDGET (bar));
}