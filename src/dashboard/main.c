#include <gtk/gtk.h>
#include "debug.h"
#include "can_device/can_device.h"
#include "can_database/can_database.h"

int fprintCanDatabaseFloat (FILE* stream, canDatabase_t* database, ssize_t index, const char* formatValue,
	const char* formatInvalid)
{
	// Get signal reference, also check existence
	canSignal_t* signal = canDatabaseGetSignal (database, index);
	if (signal == NULL)
		return CAN_DATABASE_MISSING;

	float value;
	if (canDatabaseGetFloat (database, index, &value) != CAN_DATABASE_VALID)
	{
		// Print '--' as a string using the user-provided format string. The unit is ignored if not '%s' specifier is added.
		return fprintf (stream, formatInvalid, "--", signal->unit);
	}

	// Print the value as a float using the user-provided format string. The unit is ignored if not '%s' specifier is added.
	return fprintf (stream, formatValue, value, signal->unit);
}

int snprintCanDatabaseFloat (char* str, size_t n, canDatabase_t* database, ssize_t index, const char* formatValue,
	const char* formatInvalid)
{
	// Get signal reference, also check existence
	canSignal_t* signal = canDatabaseGetSignal (database, index);
	if (signal == NULL)
		return CAN_DATABASE_MISSING;

	float value;
	if (canDatabaseGetFloat (database, index, &value) != CAN_DATABASE_VALID)
	{
		// Print '--' as a string using the user-provided format string. The unit is ignored if not '%s' specifier is added.
		return snprintf (str, n, formatInvalid, "--", signal->unit);
	}

	// Print the value as a float using the user-provided format string. The unit is ignored if not '%s' specifier is added.
	return snprintf (str, n, formatValue, value, signal->unit);
}

typedef struct
{
	GtkWidget* widget;
	const char* signalName;
	const char* formatValue;
	const char* formatInvalid;
	canDatabase_t* database;
	ssize_t index;
} canLabel_t;

#define CAN_LABEL_TO_LABEL(label) GTK_LABEL ((label)->widget)
#define CAN_LABEL_TO_WIDGET(label) ((label)->widget)

void canLabelUpdate (canLabel_t* label)
{
	char text [16] = "";
	snprintCanDatabaseFloat (text, sizeof (text), label->database, label->index, label->formatValue,
		label->formatInvalid);
	gtk_label_set_text (CAN_LABEL_TO_LABEL (label), text);
}

void canLabelInit (canLabel_t* label, canDatabase_t* database)
{
	label->database = database;
	label->widget = gtk_label_new ("");
	label->index = canDatabaseFindSignal (database, label->signalName);
	canLabelUpdate (label);
}

typedef struct
{
	GtkWidget* widget;
	const char* signalName;
	float min;
	float max;
	canDatabase_t* database;
	ssize_t index;
} canProgressBar_t;

#define CAN_PROGRESS_BAR_TO_PROGRESS_BAR(label) GTK_PROGRESS_BAR ((label)->widget)
#define CAN_PROGRESS_BAR_TO_WIDGET(label) ((label)->widget)

void canProgressBarUpdate (canProgressBar_t* bar)
{
	float value;
	if (canDatabaseGetFloat (bar->database, bar->index, &value) != CAN_DATABASE_VALID)
		value = bar->min;

	// Inverse linear interpolation
	value = (value - bar->min) / (bar->max - bar->min);

	gtk_progress_bar_set_fraction (CAN_PROGRESS_BAR_TO_PROGRESS_BAR (bar), value);
}

void canProgessBarInit (canProgressBar_t* bar, canDatabase_t* database)
{
	bar->database = database;
	bar->widget = gtk_progress_bar_new ();
	bar->index = canDatabaseFindSignal (database, bar->signalName);
	canProgressBarUpdate (bar);
}

typedef struct
{
	const char* applicationName;
	const char* applicationId;
	canDatabase_t* database;
	canLabel_t* labels;
	size_t labelCount;
	canProgressBar_t* bars;
	size_t barCount;
} updateLoopArg_t;

static gboolean updateLoop (updateLoopArg_t* arg)
{
	for (size_t index = 0; index < arg->labelCount; ++index)
		canLabelUpdate (&arg->labels [index]);

	for (size_t index = 0; index < arg->barCount; ++index)
		canProgressBarUpdate (&arg->bars [index]);

	// Continue calling this function
	return TRUE;
}

static gboolean eventKeyPress (GtkWidget* window, guint keyValue, guint keyCode, GdkModifierType state,
	GtkEventControllerKey* eventController)
{
	(void) keyCode;
	(void) state;
	(void) eventController;

	switch (keyValue)
	{
	case GDK_KEY_F1:
		if (gtk_window_is_fullscreen (GTK_WINDOW (window)))
			gtk_window_unfullscreen (GTK_WINDOW (window));
		else
			gtk_window_fullscreen (GTK_WINDOW (window));
		return TRUE;

	case GDK_KEY_F4:
		gtk_window_destroy (GTK_WINDOW (window));
		return TRUE;
	}

	return FALSE;
}

// static void drawFunction (GtkDrawingArea *area, cairo_t* cr, int width, int height, gpointer data)
// {
// 	GdkRGBA color =
// 	{
// 		.red	= 0,
// 		.green	= 0,
// 		.blue	= 0,
// 		.alpha	= 1
// 	};

// 	// cairo_arc (cr, width / 2.0, height / 2.0, MIN (width, height) / 2.0, 0, 2 * G_PI);

// 	// gtk_widget_get_color (GTK_WIDGET (area), &color);
// 	gdk_cairo_set_source_rgba (cr, &color);

// 	cairo_rectangle (cr, width / 4.0f, height / 4.0f, width / 2.0f, height / 2.0f);
// 	cairo_fill (cr);
// }

static void gtkActivate (GtkApplication* app, updateLoopArg_t* arg)
{
	// Create a new window and set its title
	GtkWidget* window = gtk_application_window_new (app);
	gtk_window_set_title (GTK_WINDOW (window), arg->applicationName);
	gtk_window_set_default_size (GTK_WINDOW (window), 800, 480);

	// Create the top-level grid
	GtkWidget* grid = gtk_grid_new ();
	gtk_window_set_child (GTK_WINDOW (window), grid);

	canProgessBarInit(&arg->bars [0], arg->database);
	gtk_orientable_set_orientation (GTK_ORIENTABLE (CAN_PROGRESS_BAR_TO_WIDGET (&arg->bars [0])), GTK_ORIENTATION_VERTICAL);
	gtk_progress_bar_set_inverted (CAN_PROGRESS_BAR_TO_PROGRESS_BAR (&arg->bars [0]), true);
	gtk_grid_attach (GTK_GRID (grid), CAN_PROGRESS_BAR_TO_WIDGET (&arg->bars [0]), 0, 0, 1, 2);

	GtkWidget* subGrid = gtk_grid_new ();
	gtk_grid_attach (GTK_GRID (grid), subGrid, 1, 0, 1, 1);
	GtkWidget* frame = gtk_frame_new ("");
	gtk_grid_attach (GTK_GRID (subGrid), frame, 0, 1, 1, 1);
	gtk_widget_set_vexpand (frame, true);
	gtk_widget_set_size_request (frame, 50, 0);
	frame = gtk_frame_new ("Data Logger");
	gtk_grid_attach (GTK_GRID (subGrid), frame, 0, 0, 2, 1);
	gtk_widget_set_size_request (frame, 200, 150);
	frame = gtk_frame_new ("Torque Config");
	gtk_grid_attach (GTK_GRID (subGrid), frame, 1, 1, 1, 1);
	gtk_widget_set_vexpand (frame, true);
	gtk_widget_set_size_request (frame, 200, 0);

	subGrid = gtk_grid_new ();
	gtk_grid_attach (GTK_GRID (grid), subGrid, 2, 0, 1, 1);
	frame = gtk_frame_new ("Center Label");
	gtk_grid_attach (GTK_GRID (subGrid), frame, 0, 0, 1, 1);
	gtk_widget_set_size_request (frame, 0, 100);
	frame = gtk_frame_new ("Center Value");
	gtk_grid_attach (GTK_GRID (subGrid), frame, 0, 1, 1, 1);
	gtk_widget_set_hexpand (frame, true);
	gtk_widget_set_vexpand (frame, true);

	subGrid = gtk_grid_new ();
	gtk_grid_attach (GTK_GRID (grid), subGrid, 3, 0, 1, 1);
	frame = gtk_frame_new ("");
	gtk_grid_attach (GTK_GRID (subGrid), frame, 0, 0, 1, 1);
	gtk_widget_set_size_request (frame, 0, 150);
	GtkWidget* subSubGrid = gtk_grid_new ();
	gtk_grid_attach (GTK_GRID (subGrid), subSubGrid, 0, 1, 1, 1);
	gtk_widget_set_size_request (subSubGrid, 200, 0);
	gtk_widget_set_vexpand (subSubGrid, true);

	for (size_t index = 0; index < arg->labelCount; ++index)
	{
		canLabelInit (&arg->labels [index], arg->database);
		gtk_grid_attach (GTK_GRID (subSubGrid), CAN_LABEL_TO_WIDGET (&arg->labels [index]), 0, index, 1, 1);
	}

	subGrid = gtk_grid_new ();
	gtk_grid_attach (GTK_GRID (grid), subGrid, 1, 1, 3, 1);
	frame = gtk_frame_new ("");
	gtk_grid_attach (GTK_GRID (subGrid), frame, 0, 0, 1, 1);
	gtk_widget_set_size_request (frame, 50, 0);
	GtkWidget* button = gtk_button_new ();
	gtk_grid_attach (GTK_GRID (subGrid), button, 1, 0, 1, 1);
	gtk_widget_set_hexpand (button, true);
	button = gtk_button_new ();
	gtk_grid_attach (GTK_GRID (subGrid), button, 2, 0, 1, 1);
	gtk_widget_set_hexpand (button, true);
	button = gtk_button_new ();
	gtk_grid_attach (GTK_GRID (subGrid), button, 3, 0, 1, 1);
	gtk_widget_set_hexpand (button, true);
	button = gtk_button_new ();
	gtk_grid_attach (GTK_GRID (subGrid), button, 4, 0, 1, 1);
	gtk_widget_set_hexpand (button, true);
	button = gtk_button_new ();
	gtk_grid_attach (GTK_GRID (subGrid), button, 5, 0, 1, 1);
	gtk_widget_set_hexpand (button, true);
	gtk_widget_set_size_request (button, 0, 90);

	canProgessBarInit(&arg->bars [1], arg->database);
	gtk_orientable_set_orientation (GTK_ORIENTABLE (CAN_PROGRESS_BAR_TO_WIDGET (&arg->bars [1])), GTK_ORIENTATION_VERTICAL);
	gtk_progress_bar_set_inverted (CAN_PROGRESS_BAR_TO_PROGRESS_BAR (&arg->bars [1]), true);
	gtk_grid_attach (GTK_GRID (grid), CAN_PROGRESS_BAR_TO_WIDGET (&arg->bars [1]), 4, 0, 1, 2);

	// GtkWidget* da = gtk_drawing_area_new ();
	// gtk_drawing_area_set_content_width (GTK_DRAWING_AREA (da), 800);
	// gtk_drawing_area_set_content_height (GTK_DRAWING_AREA (da), 480);
	// gtk_drawing_area_set_draw_func (GTK_DRAWING_AREA (da), drawFunction, NULL, NULL);
	// gtk_grid_attach (GTK_GRID (grid), da, 0, 0, 1, 1);
	// gtk_widget_set_hexpand (da, true);
	// gtk_widget_set_vexpand (da, true);

	GtkEventController* controller = gtk_event_controller_key_new ();

  	g_signal_connect_object (controller, "key-pressed", G_CALLBACK (eventKeyPress), window, G_CONNECT_SWAPPED);
	gtk_widget_add_controller (window, controller);

	// GtkWidget* button = gtk_button_new_with_label ("Button 1");
	// g_signal_connect (button, "clicked", G_CALLBACK (print_hello), NULL);

	// gtk_grid_attach (GTK_GRID (grid), button, 0, 0, 1, 1);

	// button = gtk_button_new_with_label ("Unfullscreen");
	// g_signal_connect (button, "clicked", G_CALLBACK (winscreen), window);

	// gtk_grid_attach (GTK_GRID (grid), button, 1, 0, 1, 1);

	// button = gtk_button_new_with_label ("Quit");
	// g_signal_connect_swapped (button, "clicked", G_CALLBACK (gtk_window_destroy), window);

	// gtk_grid_attach (GTK_GRID (grid), button, 0, 1, 2, 1);

	g_timeout_add (30, G_SOURCE_FUNC (updateLoop), arg);

	gtk_window_present (GTK_WINDOW (window));
}

static char* getApplicationId (const char* applicationName)
{
	const char* APPLICATION_DOMAIN = "org.zre";
	size_t length = strlen (APPLICATION_DOMAIN) + 1 + strlen (applicationName) + 1;

	char* id = malloc (length);
	if (id == NULL)
		return NULL;

	if (snprintf (id, length, "%s.%s", APPLICATION_DOMAIN, applicationName) < 0)
		return NULL;

	return id;
}

int main (int argc, char** argv)
{
	// TODO(Barach): Instancing multiple apps is weird? Consider passing args into app name?

	GtkApplication* app;
	int status;

	if (argc != 4)
	{
		fprintf (stderr, "Invalid usage: dashboard <Application Name> <Device Name> <DBC File Path>\n");
		return -1;
	}

	debugInit ();

	// Initialize the CAN device
	char* deviceName = argv [2];
	canDevice_t* device = canInit (deviceName);
	if (device == NULL)
		return errorPrintf ("Failed to initialize CAN device '%s'", deviceName);

	// Initialize the CAN database
	canDatabase_t database;
	char* dbcPath = argv [3];
	if (canDatabaseInit (&database, device, dbcPath) != 0)
		return errorPrintf ("Failed to initialize CAN database");

	// Create application ID from application name
	char* applicationName = argv [1];
	char* applicationId = getApplicationId (applicationName);
	if (applicationId == NULL)
		return errorPrintf ("Failed to create application name");

	canLabel_t labels [] =
	{
		{
			.signalName		= "APPS_1_PERCENT",
			.formatValue	= "APPS-1: %.2f%s",
			.formatInvalid	= "APPS-1: %s%s"
		},
		{
			.signalName		= "APPS_2_PERCENT",
			.formatValue	= "APPS-2: %.2f%s",
			.formatInvalid	= "APPS-2: %s%s"
		},
		{
			.signalName		= "BSE_FRONT_PERCENT",
			.formatValue	= "BSE-F: %.2f%s",
			.formatInvalid	= "BSE-F: %s%s"
		},
		{
			.signalName		= "BSE_REAR_PERCENT",
			.formatValue	= "BSE-R: %.2f%s",
			.formatInvalid	= "BSE-R: %s%s"
		},
		{
			.signalName		= "STEERING_ANGLE",
			.formatValue	= "SAS: %.2f%s",
			.formatInvalid	= "SAS: %s%s"
		}
	};

	canProgressBar_t bars [] =
	{
		{
			.signalName		= "STEERING_ANGLE",
			.min			= -180,
			.max			= 180
		},
		{
			.signalName		= "STEERING_ANGLE",
			.min			= -90,
			.max			= 90
		}
	};

	updateLoopArg_t arg =
	{
		.applicationName	= applicationName,
		.applicationId		= applicationId,
		.database			= &database,
		.labels				= labels,
		.labelCount			= sizeof (labels) / sizeof (labels [0]),
		.bars				= bars,
		.barCount			= sizeof (bars) / sizeof (bars [0])
	};

	// Create the GTK application bound to the activation signal
	app = gtk_application_new (applicationId, G_APPLICATION_DEFAULT_FLAGS);
	g_signal_connect (app, "activate", G_CALLBACK (gtkActivate), &arg);
	status = g_application_run (G_APPLICATION (app), argc - 3, argv);
	g_object_unref (app);

	// Deallocate the application ID.
	free (applicationId);

	// Exit
	return status;
}