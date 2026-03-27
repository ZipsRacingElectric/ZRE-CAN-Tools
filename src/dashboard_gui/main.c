// Dashboard GUI --------------------------------------------------------------------------------------------------------------
//
// Author: Cole Barach
// Date Created: 2025.12.22
//
// Description:
// - For application description, see help page.
// - For technical documentation, see doc/dashboard_gui.md.
// - For help with the GTK library, see doc/gtk_introduction.md.

// Includes -------------------------------------------------------------------------------------------------------------------

// Includes
#include "page_stack.h"
#include "cjson/cjson_util.h"
#include "can_database/can_database_stdio.h"
#include "can_device/can_device_stdio.h"
#include "options.h"
#include "debug.h"
#include "misc_port.h"

// GTK
#include <gtk/gtk.h>

// Globals --------------------------------------------------------------------------------------------------------------------

bool fullscreen = false;

// Functions ------------------------------------------------------------------------------------------------------------------

static void fprintUsage (FILE* stream)
{
	fprintf (stream, "Usage: dashboard-gui <Options> <Config JSON> <Device 0 Name> <Device 1 Name> ...\n");
}

static void fprintHelp (FILE* stream)
{
	fprintf (stream, ""
		"dashboard-gui - Graphical user interface for the dashboard of Zips Racing's\n"
		"                vehicles.\n\n");


	fprintUsage (stream);

	fprintf (stream, "\n"
		"Parameters:\n"
		"\n"
		"    <Config JSON>         - The dashboard's configuration file. This defines the\n"
		"                            pages and style of the GUI.\n\n");

	fprintCanDeviceNameHelp (stream, "    ");
	fprintCanDbcFileHelp (stream, "    ");

	fprintf (stream, ""
		"Options:\n\n"
		"    --fullscreen          - Start the application in fullscreen mode.\n\n");
	fprintOptionHelp (stream, "    ");
}

static void handleOptionFullscreen (char* option, char* value)
{
	(void) option;
	(void) value;
	fullscreen = true;
}

typedef struct
{
	const char* applicationTitle;
	const char* applicationId;
	cJSON* config;
	canDatabase_t* databases;
	size_t databaseCount;
} activateArg_t;

static gboolean updateLoop (pageStack_t* stack)
{
	pageStackUpdate (stack);

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

	// If defaulting to fullscreen, set it.
	if (fullscreen)
		gtk_window_fullscreen (GTK_WINDOW (window));

	pageStack_t* stack = pageStackInit ();
	gtk_window_set_child (GTK_WINDOW (window), PAGE_STACK_TO_WIDGET (stack));

	pageStyle_t* style = pageStyleLoad (jsonGetObjectV2 (arg->config, "baseStyle"), NULL);

	cJSON* pageConfigs = jsonGetObjectV2 (arg->config, "pages");
	if (pageConfigs == NULL)
	{
		errorPrintf ("Failed to load pages");
		gtk_window_destroy (GTK_WINDOW (window));
		return;
	}

	size_t pageCount = cJSON_GetArraySize (pageConfigs);
	page_t** pages = malloc (sizeof (page_t*) * pageCount);
	if (pages == NULL)
		return;

	for (size_t index = 0; index < pageCount; ++index)
	{
		cJSON* pageConfig = cJSON_GetArrayItem (pageConfigs, index);
		pages [index] = pageLoad (pageConfig, arg->databases, arg->databaseCount, style);
		if (pages [index] == NULL)
			continue;

		pageStackAppend (stack, pages [index]);
	}

	for (size_t index = 0; index < pageCount; ++index)
	{
		if (pages [index] == NULL)
			continue;

		for (size_t buttonIndex = 0; buttonIndex < pageCount; ++buttonIndex)
		{
			if (pages [buttonIndex] != NULL)
			{
				if (index != buttonIndex)
					pageAppendButton (pages [index], pageGetName (pages [buttonIndex]), pageStackSelectCallback, pages [buttonIndex], false, style);
				else
					pageAppendButton (pages [index], pageGetName (pages [buttonIndex]), NULL, NULL, true, style);
			}
			else
			{
				pageAppendButton (pages [index], "", NULL, NULL, false, style);
			}
		}
	}

	GtkEventController* controller = gtk_event_controller_key_new ();
	g_signal_connect_object (controller, "key-pressed", G_CALLBACK (eventKeyPress), window, G_CONNECT_SWAPPED);
	gtk_widget_add_controller (window, controller);

	gtk_window_present (GTK_WINDOW (window));

	// Create the event loop timer
	guint* timeout = malloc (sizeof (guint));
	*timeout = g_timeout_add (40, G_SOURCE_FUNC (updateLoop), stack);

	// Bind the destroy signal to a handler
	g_signal_connect (GTK_WINDOW (window), "destroy", G_CALLBACK (gtkDestroyHandler), timeout);
}

static char* getApplicationTitle (const char* applicationName)
{
	size_t length = snprintf (NULL, 0, "dashboard-gui - %s - %s", applicationName, ZRE_CANTOOLS_VERSION_FULL) + 1;

	char* title = malloc (length);
	if (title == NULL)
		return NULL;

	if (snprintf (title, length, "dashboard-gui - %s - %s", applicationName, ZRE_CANTOOLS_VERSION_FULL) < 0)
		return NULL;

	return title;
}

static char* getApplicationId (const char* applicationName)
{
	const char* APPLICATION_DOMAIN = "org.zre";
	size_t length = snprintf (NULL, 0, "%s.dashboard-gui-%s", APPLICATION_DOMAIN, applicationName) + 1;

	char* id = malloc (length);
	if (id == NULL)
		return NULL;

	if (snprintf (id, length, "%s.dashboard-gui-%s", APPLICATION_DOMAIN, applicationName) < 0)
		return NULL;

	return id;
}

int main (int argc, char** argv)
{
	// TODO(Barach): Instancing multiple apps is weird? Consider passing args into app name?

	// The argc and argv to pass GTK (just the application name).
	char** gtkArgv = argv;
	int gtkArgc = 1;

	// Debug initialization
	debugInit ();

	// Check standard arguments
	if (handleOptions (&argc, &argv, &(handleOptionsParams_t)
	{
		.fprintHelp		= fprintHelp,
		.charHandlers	= NULL,
		.chars			= NULL,
		.charCount		= 0,
		.stringHandlers	= (optionStringCallback_t* [])
		{
			handleOptionFullscreen
		},
		.strings		= (char* [])
		{
			"fullscreen"
		},
		.stringCount	= 1
	}) != 0)
		return errorPrintf ("Failed to handle options");

	// Validate usage
	if (argc < 1)
	{
		fprintUsage (stderr);
		return -1;
	}

	// Load the config file
	char* configPath = argv [0];
	cJSON* config = jsonLoad (configPath);
	if (config == NULL)
		return errorPrintf ("Failed to load config JSON '%s'", configPath);

	cJSON* dbcConfigArray = jsonGetObjectV2 (config, "dbcFiles");
	if (dbcConfigArray == NULL)
		return errorPrintf ("Failed to load DBC file array");

	// Get the number of CAN devices from the config file

	size_t deviceCount = cJSON_GetArraySize (dbcConfigArray);

	if ((size_t) argc != deviceCount + 1)
	{
		fprintf (stderr, "Missing CAN device name, expected %lu device(s).\n", (long unsigned) deviceCount);
		fprintUsage (stderr);
		return -1;
	}

	// Allocate arrays

	canDevice_t** devices = malloc (sizeof (canDevice_t*) * deviceCount);
	if (devices == NULL)
		return errorPrintf ("Failed to allocate CAN device array");

	canDatabase_t* databases = malloc (sizeof (canDatabase_t) * deviceCount);
	if (databases == NULL)
		return errorPrintf ("Failed to allocate CAN database array");

	// CAN Device / Database initialization

	for (size_t index = 0; index < deviceCount; ++index)
	{
		// Initialize the CAN device
		char* deviceName = argv [index + 1];
		devices [index] = canInit (deviceName);
		if (devices [index] == NULL)
			return errorPrintf ("Failed to initialize CAN device '%s'", deviceName);

		// Get the DBC file path and expand any environment variables.
		cJSON* dbcConfig = cJSON_GetArrayItem (dbcConfigArray, index);
		char* dbcPath = cJSON_GetStringValue (dbcConfig);
		char* dbcPathExpanded = expandEnv (dbcPath);
		if (dbcPathExpanded == NULL)
			return errorPrintf ("Failed to expand environment variable");

		// Initialize the CAN database
		if (canDatabaseInit (&databases [index], devices [index], dbcPathExpanded) != 0)
			return errorPrintf ("Failed to initialize CAN database '%s'", dbcPathExpanded);

		free (dbcPathExpanded);
	}

	// Create application ID from application name

	char* applicationName;
	if (jsonGetString (config, "name", &applicationName) != 0)
		return errorPrintf ("Failed to load application name");

	char* applicationTitle = getApplicationTitle (applicationName);
	if (applicationTitle == NULL)
		return errorPrintf ("Failed to create application title");

	char* applicationId = getApplicationId (applicationName);
	if (applicationId == NULL)
		return errorPrintf ("Failed to create application name");

	debugPrintf ("Application ID: '%s'\n", applicationId);
	debugPrintf ("Application Title: '%s'\n", applicationTitle);

	// Arguments to pass activation function

	activateArg_t arg =
	{
		.applicationTitle	= applicationTitle,
		.applicationId		= applicationId,
		.config				= config,
		.databases			= databases,
		.databaseCount		= deviceCount
	};

	// Create the GTK application bound to the activation signal
	GtkApplication* app = gtk_application_new (applicationId, G_APPLICATION_DEFAULT_FLAGS);
	g_signal_connect (app, "activate", G_CALLBACK (gtkActivate), &arg);
	int status = g_application_run (G_APPLICATION (app), gtkArgc, gtkArgv);
	g_object_unref (app);

	// Deallocate the application ID.
	free (applicationTitle);
	free (applicationId);

	// Deallocate the CAN Devices /Databases.
	for (size_t index = 0; index < deviceCount; ++index)
	{
		canDatabaseDealloc (&databases [index]);
		canDealloc (devices [index]);
	}

	// Exit
	return status;
}