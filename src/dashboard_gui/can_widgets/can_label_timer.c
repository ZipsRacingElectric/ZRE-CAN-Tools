#include "can_label_timer.h"
#include "../gtk_util.h"
#include "cjson/cjson_util.h"
#include "debug.h"

static void draw (GtkDrawingArea* area, cairo_t* cr, int width, int height, gpointer arg)
{
	(void) area;
	(void) arg;

	canLabelTimer_t* timer = arg;

	// Creates the background of the timer
	gdk_cairo_set_source_rgba (cr, &timer->config.backgroundColor);
	cairo_rectangle (cr, 0, 0, width, height);
	cairo_fill_preserve (cr);

	// Creates the border of the timer
	cairo_set_line_width(cr, timer->config.borderThickness);
    gdk_cairo_set_source_rgba (cr, &timer->config.borderColor);
    cairo_stroke(cr);
}

static void update (void* widget)
{
	float value = 0;
	canLabelTimer_t* timer = widget;

	// REVIEW(Barach): canDatabaseGetFloat is one of the few functions that doesn't write to errno on failure, instead the
	//   return value indicates why it failed. In general, this only fails if a signal is timed out, in which case the timer
	//   should display "--:--:---" instead of writing to the debug stream. See return value enum docs for how to check.

	if (canDatabaseGetFloat (timer->database, timer->config.signalIndex, &value))
		debugPrintf (errorCodeToMessage (errno));

	// REVIEW(Barach): Eventually we'll want this 0.5 pulled out into a "threshold" value and an "inverted" option added. Can use:
	//
	// if ((value >= threshold) != inverted)
	//
	// to check if the button is pressed.
	//
	if (value > 0.5 && !timer->buttonPressed)
	{
		if (timer->running)
		{
			// REVIEW(Barach): I'd replace this with CLOCK_MONOTONIC, as realtime clock is subject to resynchronizations (can
			//   go backwards).
			clock_gettime (CLOCK_REALTIME, &timer->currentTime);
	 		struct timespec delta = timespecSub (&timer->currentTime, &timer->startTime);

			// REVIEW(Barach): Memory allocation can be pretty slow, I'd replace this with either a statically allocated buffer
			//   (in the structure) or one that is allocated during initialization.
			size_t n = strlen ("00:00:000") + 1;
				char* time = malloc (n);
				if (time == NULL)
					return;

			if (timer->mode == LAST_TIME)
			{
				timer->lastTime.tv_sec = delta.tv_sec;
				timer->lastTime.tv_nsec = delta.tv_nsec;

				// REVIEW(Barach): Nothing wrong with this, but I'd probably pull this out into a function just to clean things
				//   up. Ex:
				//
				//     void setText (canLabelTimer_t* timer, struct timespec* timestamp)
				//
				snprintf (time, n, "%02lu:%02lu:%03lu",
					(unsigned long) (timer->lastTime.tv_sec / 60),
					(unsigned long) (timer->lastTime.tv_sec % 60),
					(unsigned long) (timer->lastTime.tv_nsec / 1000000)
				);

				gtk_label_set_text (GTK_LABEL (timer->timer), time);
			}
			else if (timer->mode == BEST_TIME)
			{
				if (timer->bestTime.tv_nsec == 0 && timer->bestTime.tv_sec == 0) {
					timer->bestTime.tv_sec = delta.tv_sec;
					timer->bestTime.tv_nsec = delta.tv_nsec;
				}
				else
				{
					// REVIEW(Barach): This comparison isn't correct. Timespec A is only less than timespec B if:
					//   A.tv_sec < B.tv_sec || (A.tv_sec == B.tv_sec && A.tv_nsec < B.tv_nsec)
					//
					//   You can use the timespecCompare macro to do this:
					//
					//     timespecCompare(a, b, <) // for A < B.
					//
					if (delta.tv_sec <= timer->bestTime.tv_sec && delta.tv_nsec <= timer->bestTime.tv_nsec)
					{
						timer->bestTime.tv_sec = delta.tv_sec;
						timer->bestTime.tv_nsec = delta.tv_nsec;
					}
				}

				snprintf (time, n, "%02lu:%02lu:%03lu",
					(unsigned long) (timer->bestTime.tv_sec / 60),
					(unsigned long) (timer->bestTime.tv_sec % 60),
					(unsigned long) (timer->bestTime.tv_nsec / 1000000)
				);

				gtk_label_set_text (GTK_LABEL (timer->timer), time);
			}

			free (time);
			timer->buttonPressed = true;
			clock_gettime (CLOCK_REALTIME, &timer->startTime);
		}
		else
		{
			timer->running = true;
			timer->buttonPressed = true;
			clock_gettime (CLOCK_REALTIME, &timer->startTime);
		}
	}

	// REVIEW(Barach): Same threshold/inverted as above.
	if (value < 0.5 && timer->buttonPressed)
	{
		timer->buttonPressed = false;
	}

	if (timer->running)
	{
		if (timer->mode == CURRENT_TIME)
		{

			clock_gettime (CLOCK_REALTIME, &timer->currentTime);
			struct timespec delta = timespecSub (&timer->currentTime, &timer->startTime);

			size_t n = strlen ("00:00:000") + 1;
			char* time = malloc (n);
			if (time == NULL)
				return;

			snprintf (time, n, "%02lu:%02lu:%03lu",
				(unsigned long) (delta.tv_sec / 60),
				(unsigned long) (delta.tv_sec % 60),
				(unsigned long) (delta.tv_nsec / 1000000)
			);

			gtk_label_set_text (GTK_LABEL (timer->timer), time);

			free (time);
		}
	}
}

canWidget_t* canLabelTimerInit (canDatabase_t* database, canLabelTimerConfig_t* config)
{
    // Allocate the object
	canLabelTimer_t* timer = malloc (sizeof (canLabelTimer_t));
	if (timer == NULL)
		return NULL;

    // Init the object
	*timer = (canLabelTimer_t)
	{
		.vmt =
		{
			.update = update,
			.widget	= gtk_grid_new (),
		},
		.config		= *config,
		.database	= database,
	};

	// Timer Initialization
	timer->currentTime.tv_sec = 0;
	timer->currentTime.tv_nsec = 0;

	timer->startTime.tv_sec = 0;
	timer->startTime.tv_nsec = 0;

	timer->lastTime.tv_nsec = 0;
	timer->lastTime.tv_sec = 0;

	timer->bestTime.tv_nsec = 0;
	timer->bestTime.tv_sec = 0;

	// REVIEW(Barach): In general, '--' is used to indicate a timeout, so I'd default to "--:--:---"
	timer->overlay = gtk_overlay_new ();
	timer->timer = gtk_label_new ("00:00:000");

	// TODO(DiBacco): change font size configuration to work via gtk_utils method and with the page's font size specification
	PangoAttrList* attrList = pango_attr_list_new ();
	PangoAttribute* fontSize = pango_attr_size_new (20 * PANGO_SCALE);
	pango_attr_list_insert (attrList, fontSize);

	gtk_label_set_attributes (GTK_LABEL (timer->timer), attrList);
	pango_attr_list_unref (attrList);

	gtkLabelSetColor (GTK_LABEL (timer->timer), &timer->config.borderColor);
	gtk_widget_set_halign(timer->timer, GTK_ALIGN_START);

	timer->area = gtk_drawing_area_new ();

	gtk_overlay_set_child (GTK_OVERLAY (timer->overlay), timer->area);
	gtk_overlay_add_overlay (GTK_OVERLAY (timer->overlay), timer->timer);
	gtk_grid_attach (GTK_GRID (CAN_WIDGET_TO_WIDGET (timer)), timer->overlay, 0, 0, 1, 1);

	gtk_drawing_area_set_content_width (GTK_DRAWING_AREA (timer->area), config->width);
	gtk_drawing_area_set_content_height (GTK_DRAWING_AREA (timer->area), config->height);
	gtk_drawing_area_set_draw_func (GTK_DRAWING_AREA (timer->area), draw, timer, NULL);

	timer->running = false;
	timer->buttonPressed = false;

	timer->config.signalIndex = canDatabaseFindSignal (database, "WHEEL_BUTTON_TOP_LEFT");

    // Update initial value
	update (timer);

	// Cast into the base type
	return (canWidget_t*) timer;
}

canWidget_t* canLabelTimerLoad (canDatabase_t* database, cJSON* config)
{
	// REVIEW(Barach): The timer's mode should be loaded in from the config file.

	uint16_t width;
	if (jsonGetUint16_t (config, "width", &width) != 0)
		return NULL;

	uint16_t height;
	if (jsonGetUint16_t (config, "height", &height) != 0)
		return NULL;

	char* backgroundColor;
	if (jsonGetString (config, "backgroundColor", &backgroundColor) != 0)
		return NULL;

	float boarderThickness;
	if (jsonGetFloat (config, "boarderThickness", &boarderThickness) != 0)
		return NULL;

	char* borderColor;
	if (jsonGetString (config, "borderColor", &borderColor) != 0)
		return NULL;

	char* fontColor;
	if (jsonGetString (config, "fontColor", &fontColor) != 0)
		return NULL;

	return canLabelTimerInit(database, &(canLabelTimerConfig_t)
	{
		.width  			= width,
		.height 			= height,
		.backgroundColor 	= gdkHexToColor (backgroundColor),
		.borderColor 		= gdkHexToColor (borderColor),
		.fontColor 			= gdkHexToColor (fontColor),
		.borderThickness 	= boarderThickness
	});
}

void setMode (void* widget, char* mode)
{
	canLabelTimer_t* timer = widget;

	if (strcmp (mode, "current") == 0)
        timer->mode = CURRENT_TIME;

	else if (strcmp (mode, "last") == 0)
        timer->mode = LAST_TIME;

	else if (strcmp (mode, "best") == 0)
        timer->mode = BEST_TIME;

	else debugPrintf ("Failed to set mode of unknown type: %s\n", mode);
}

