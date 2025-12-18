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

static gboolean gtkDestroyHandler (GtkWidget* self, gpointer data)
{
	(void) self;

	// Remove the event loop timer
	guint* timeout = data;
	g_source_remove (*timeout);
	free (timeout);

	return TRUE;
}

static void gtkActivate (GtkApplication* app, activateArg_t* arg)
{
	// Create a new window and set its title
	GtkWidget* window = gtk_application_window_new (app);
	gtk_window_set_title (GTK_WINDOW (window), arg->applicationName);
	gtk_window_set_default_size (GTK_WINDOW (window), 800, 480);

	pageAutox_t* page = malloc (sizeof (pageAutox_t));
	pageAutoxInit (page, arg->database);
	gtk_window_set_child (GTK_WINDOW (window), PAGE_AUTOX_TO_WIDGET (page));

	GtkEventController* controller = gtk_event_controller_key_new ();

  	g_signal_connect_object (controller, "key-pressed", G_CALLBACK (eventKeyPress), window, G_CONNECT_SWAPPED);
	gtk_widget_add_controller (window, controller);

	gtk_window_present (GTK_WINDOW (window));

	// Create the event loop timer
	guint* timeout = malloc (sizeof (guint));
	*timeout = g_timeout_add (1, G_SOURCE_FUNC (updateLoop), page);

	// Bind the destroy signal to a handler
	g_signal_connect (GTK_WINDOW (window), "destroy", G_CALLBACK (gtkDestroyHandler), timeout);
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