#include "debug.h"
#include "page_autox.h"

// GTK
#include <gtk/gtk.h>

typedef struct
{
	const char* applicationName;
	const char* applicationId;
	canDatabase_t* database;
} activateArg_t;

static gboolean updateLoop (pageAutox_t* page)
{
	pageAutoxUpdate (page);

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

static void gtkActivate (GtkApplication* app, activateArg_t* arg)
{
	// Create a new window and set its title
	GtkWidget* window = gtk_application_window_new (app);
	gtk_window_set_title (GTK_WINDOW (window), arg->applicationName);
	gtk_window_set_default_size (GTK_WINDOW (window), 800, 480);

	// GtkWidget* da = gtk_drawing_area_new ();
	// gtk_drawing_area_set_content_width (GTK_DRAWING_AREA (da), 800);
	// gtk_drawing_area_set_content_height (GTK_DRAWING_AREA (da), 480);
	// gtk_drawing_area_set_draw_func (GTK_DRAWING_AREA (da), drawFunction, NULL, NULL);
	// gtk_grid_attach (GTK_GRID (grid), da, 0, 0, 1, 1);
	// gtk_widget_set_hexpand (da, true);
	// gtk_widget_set_vexpand (da, true);

	pageAutox_t* page = malloc (sizeof (pageAutox_t));
	pageAutoxInit (page, arg->database);
	gtk_window_set_child (GTK_WINDOW (window), PAGE_AUTOX_TO_WIDGET (page));

	// GtkWidget* grid = gtk_grid_new ();
	// gtk_window_set_child (GTK_WINDOW (window), grid);

	// GtkWidget* left = gtk_label_new ("- LEFT -");
	// gtk_widget_set_size_request (left, 150, 0);
	// // gtk_widget_set_halign (left, GTK_ALIGN_FILL);
	// gtk_label_set_xalign (GTK_LABEL (left), 0.5);
	// gtk_grid_attach (GTK_GRID (grid), left, 0, 0, 1, 1);

	// GtkWidget* middle = gtk_label_new ("- MIDDLE -");
	// gtk_widget_set_hexpand (middle, true);
	// gtk_grid_attach (GTK_GRID (grid), middle, 1, 0, 1, 1);

	// GtkWidget* right = gtk_label_new ("- RIGHT -");
	// gtk_widget_set_size_request (right, 150, 0);
	// gtk_grid_attach (GTK_GRID (grid), right, 2, 0, 1, 1);

	GtkEventController* controller = gtk_event_controller_key_new ();

  	g_signal_connect_object (controller, "key-pressed", G_CALLBACK (eventKeyPress), window, G_CONNECT_SWAPPED);
	gtk_widget_add_controller (window, controller);

	g_timeout_add (30, G_SOURCE_FUNC (updateLoop), page);

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

	activateArg_t arg =
	{
		.applicationName	= applicationName,
		.applicationId		= applicationId,
		.database			= &database
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