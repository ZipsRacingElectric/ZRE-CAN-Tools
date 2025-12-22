// Includes
#include "debug.h"
#include "page_autox.h"
#include "options.h"

// GTK
#include <gtk/gtk.h>

static void fprintHelp (FILE* stream)
{
	fprintf (stream, "TODO(Barach)\n");
}

static void fprintUsage (FILE* stream)
{
	fprintf (stream, "Invalid usage: dashboard <Application Name> <Device Name> <DBC File Path>\n");
}

typedef struct
{
	const char* applicationTitle;
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
	gtk_window_set_title (GTK_WINDOW (window), arg->applicationTitle);
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

static char* getApplicationTitle (const char* applicationName)
{
	size_t length = snprintf (NULL, 0, "dashboard - %s - %s", applicationName, ZRE_CANTOOLS_VERSION_FULL) + 1;

	char* title = malloc (length);
	if (title == NULL)
		return NULL;

	if (snprintf (title, length, "dashboard - %s - %s", applicationName, ZRE_CANTOOLS_VERSION_FULL) < 0)
		return NULL;

	return title;
}

static char* getApplicationId (const char* applicationName)
{
	const char* APPLICATION_DOMAIN = "org.zre";
	size_t length = snprintf (NULL, 0, "%s.dashboard-%s", APPLICATION_DOMAIN, applicationName) + 1;

	char* id = malloc (length);
	if (id == NULL)
		return NULL;

	if (snprintf (id, length, "%s.dashboard-%s", APPLICATION_DOMAIN, applicationName) < 0)
		return NULL;

	return id;
}

int main (int argc, char** argv)
{
	// TODO(Barach): Instancing multiple apps is weird? Consider passing args into app name?

	// Debug initialization
	debugInit ();

	// Check standard arguments
	for (int index = 1; index < argc; ++index)
	{
		switch (handleOption (argv [index], NULL, fprintHelp))
		{
		case OPTION_CHAR:
		case OPTION_STRING:
			fprintf (stderr, "Unknown argument '%s'.\n", argv [index]);
			return -1;

		case OPTION_QUIT:
			return 0;

		default:
			break;
		}
	}

	// Validate usage
	if (argc < 4)
	{
		fprintUsage (stderr);
		return -1;
	}

	// Initialize the CAN device
	char* deviceName = argv [argc - 2];
	canDevice_t* device = canInit (deviceName);
	if (device == NULL)
		return errorPrintf ("Failed to initialize CAN device '%s'", deviceName);

	// Initialize the CAN database
	canDatabase_t database;
	char* dbcPath = argv [argc - 1];
	if (canDatabaseInit (&database, device, dbcPath) != 0)
		return errorPrintf ("Failed to initialize CAN database");

	// Create application ID from application name
	char* applicationName = argv [argc - 3];
	char* applicationTitle = getApplicationTitle (applicationName);
	if (applicationTitle == NULL)
		return errorPrintf ("Failed to create application title");

	char* applicationId = getApplicationId (applicationName);
	if (applicationId == NULL)
		return errorPrintf ("Failed to create application name");

	debugPrintf ("Application ID: '%s'\n", applicationId);
	debugPrintf ("Application Title '%s'\n", applicationTitle);

	activateArg_t arg =
	{
		.applicationTitle	= applicationTitle,
		.applicationId		= applicationId,
		.database			= &database
	};

	// Create the GTK application bound to the activation signal
	GtkApplication* app = gtk_application_new (applicationId, G_APPLICATION_DEFAULT_FLAGS);
	g_signal_connect (app, "activate", G_CALLBACK (gtkActivate), &arg);
	int status = g_application_run (G_APPLICATION (app), 1, argv);
	g_object_unref (app);

	// Deallocate the application ID.
	free (applicationTitle);
	free (applicationId);

	// Exit
	return status;
}