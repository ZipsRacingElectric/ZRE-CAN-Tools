#include <gtk/gtk.h>
#include "debug.h"
#include "can_device/can_device.h"
#include "can_database/can_database.h"

int fprintCanDatabaseFloat (FILE* stream, canDatabase_t* database, ssize_t index, const char* prefix,
	const char* formatValue, const char* formatInvalid)
{
	// Get signal reference, also check existence
	canSignal_t* signal = canDatabaseGetSignal (database, index);
	if (signal == NULL)
		return CAN_DATABASE_MISSING;

	// Prefix first
	int result = fprintf (stream, "%s", prefix);
	if (result < 0)
		return result;
	int count = result;

	// Get the value and print
	float value;
	if (canDatabaseGetFloat (database, index, &value) == CAN_DATABASE_VALID)
	{
		// Print the value as a float using the user-provided format string. The unit is ignored if not '%s' specifier is added.
		result = fprintf (stream, formatValue, value, signal->unit);
		if (result < 0)
			return result;
		count += result;
	}
	else
	{
		// Print '--' as a string using the user-provided format string. The unit is ignored if not '%s' specifier is added.
		result = fprintf (stream, formatInvalid, "--", signal->unit);
		if (result < 0)
			return result;
		count += result;
	}

	return count;
}

int snprintCanDatabaseFloat (char* str, size_t n, canDatabase_t* database, ssize_t index, const char* prefix,
	const char* formatValue, const char* formatInvalid)
{
	// Get signal reference, also check existence
	canSignal_t* signal = canDatabaseGetSignal (database, index);
	if (signal == NULL)
		return CAN_DATABASE_MISSING;

	// Prefix first
	int result = snprintf (str, n, "%s", prefix);
	if (result < 0 || (size_t) result >= n)
		return -1;
	int count = result;
	str += count;
	n -= count;

	// Get the value and print
	float value;
	if (canDatabaseGetFloat (database, index, &value) == CAN_DATABASE_VALID)
	{
		// Print the value as a float using the user-provided format string. The unit is ignored if not '%s' specifier is added.
		result = snprintf (str, n, formatValue, value, signal->unit);
		if (result < 0 || (size_t) result >= n)
			return -1;
		count += result;
	}
	else
	{
		// Print '--' as a string using the user-provided format string. The unit is ignored if not '%s' specifier is added.
		result = snprintf (str, n, formatInvalid, "--", signal->unit);
		if (result < 0 || (size_t) result >= n)
			return -1;
		count += result;
	}

	return count;
}

typedef struct
{
	GtkWidget* widget;
	const char* signalName;
	const char* prefix;
	const char* formatValue;
	const char* formatInvalid;
	canDatabase_t* database;
	ssize_t index;
	size_t specifierIndex;
} canLabel_t;

typedef struct
{
	const char* applicationName;
	const char* applicationId;
	canDatabase_t* database;
	canLabel_t* labels;
	size_t labelCount;
} updateLoopArg_t;

void canLabelUpdate (canLabel_t* label)
{
	char text [16] = "";
	snprintCanDatabaseFloat (text, sizeof (text), label->database, label->index, label->prefix, label->formatValue,
		label->formatInvalid);
	gtk_label_set_text (GTK_LABEL (label->widget), text);
}

void canLabelInit (canLabel_t* label, canDatabase_t* database)
{
	label->database = database;
	label->widget = gtk_label_new ("HELLO");
	label->index = canDatabaseFindSignal (database, label->signalName);
	canLabelUpdate (label);
}

static gboolean updateLoop (updateLoopArg_t* arg)
{
	for (size_t index = 0; index < arg->labelCount; ++index)
		canLabelUpdate (&arg->labels [index]);

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

	// arg->indices = malloc (sizeof (ssize_t) * 5);
	// arg->labels  = malloc (sizeof (GtkWidget*) * 5);

	// Create the top-level grid
	GtkWidget* grid = gtk_grid_new ();
	gtk_window_set_child (GTK_WINDOW (window), grid);

	GtkWidget* frame = gtk_frame_new ("B");
	gtk_grid_attach (GTK_GRID (grid), frame, 0, 0, 1, 2);

	GtkWidget* subGrid = gtk_grid_new ();
	gtk_grid_attach (GTK_GRID (grid), subGrid, 1, 0, 1, 1);
	frame = gtk_frame_new ("");
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
		gtk_grid_attach (GTK_GRID (subSubGrid), arg->labels [index].widget, 0, index, 1, 1);
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
	gtk_widget_set_size_request (button, 0, 90);

	frame = gtk_frame_new ("A");
	gtk_grid_attach (GTK_GRID (grid), frame, 4, 0, 1, 2);

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
			.prefix			= "APPS-1: ",
			.formatValue	= "%.2f",
			.formatInvalid	= "%s"
		},
		{
			.signalName		= "APPS_2_PERCENT",
			.prefix			= "APPS-2: ",
			.formatValue	= "%.2f",
			.formatInvalid	= "%s"
		},
		{
			.signalName		= "BSE_FRONT_PERCENT",
			.prefix			= "BSE-F: ",
			.formatValue	= "%.2f",
			.formatInvalid	= "%s"
		},
		{
			.signalName		= "BSE_REAR_PERCENT",
			.prefix			= "BSE-R: ",
			.formatValue	= "%.2f",
			.formatInvalid	= "%s"
		},
		{
			.signalName		= "STEERING_ANGLE",
			.prefix			= "SAS: ",
			.formatValue	= "%.2f",
			.formatInvalid	= "%s"
		}
	};

	updateLoopArg_t arg =
	{
		.applicationName	= applicationName,
		.applicationId		= applicationId,
		.database			= &database,
		.labels				= labels,
		.labelCount			= sizeof (labels) / sizeof (labels [0])
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