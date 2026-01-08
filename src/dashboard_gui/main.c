// Includes
#include "debug.h"
#include "page_autox.h"
#include "page_bms_overview.h"
#include "page_can_bus.h"
#include "page_stack.h"
#include "cjson/cjson_util.h"
#include "options.h"
#include "bms/bms.h"
#include "gtk_util.h"

// GTK
#include <gtk/gtk.h>

static void fprintUsage (FILE* stream)
{
	fprintf (stream, "Usage: dashboard-gui <Options> <Application Name> <Device Name> <DBC File Path> <BMS Config Path>\n");
}

static void fprintHelp (FILE* stream)
{
	// TODO(Barach)
	fprintf (stream, "dashboard-gui\n\n");
	fprintUsage (stream);
	fprintf (stream, "\n");
}

typedef struct
{
	const char* applicationTitle;
	const char* applicationId;
	canDatabase_t* database;
	bms_t* bms;
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

	// TODO(Barach): JSON
	pageStyle_t* style = malloc (sizeof (pageStyle_t));
	*style = (pageStyle_t)
	{
		.backgroundColor		= gdkHexToColor ("#000000"),
		.fontColor				= gdkHexToColor ("#F4931E"),
		.borderColor			= gdkHexToColor ("#D3792C"),
		.borderThickness		= 1.5,
		.indicatorActiveColor	= gdkHexToColor ("#FF0000"),
		.indicatorInactiveColor	= gdkHexToColor ("#580000"),
		.buttonHeight			= 80,
		.buttonFont				= "Futura Std Bold Condensed 34px"
	};

	pageStack_t* stack = pageStackInit ();
	gtk_window_set_child (GTK_WINDOW (window), PAGE_STACK_TO_WIDGET (stack));

	// TODO(Barach): Path
	cJSON* pageAutoxConfig = jsonLoad ("config/zr25_glory/temp.json");
	page_t* pageAutox = pageAutoxInit (arg->database, style, pageAutoxConfig);
	pageStackAppend (stack, pageAutox);
	pageStackPair_t* pageAutoxPair = pageStackPairInit (stack, pageAutox);

	page_t* pageBms = pageBmsOverviewInit (arg->bms, style);
	pageStackAppend (stack, pageBms);
	pageStackPair_t* pageBmsPair = pageStackPairInit (stack, pageBms);

	page_t* pageCanBus = pageCanBusInit (arg->database, style);
	pageStackAppend (stack, pageCanBus);
	pageStackPair_t* pageCanBusPair = pageStackPairInit (stack, pageCanBus);

	pageAppendButton (pageAutox, "AUTO-X", NULL, NULL, true);
	pageAppendButton (pageAutox, "", NULL, NULL, false);
	pageAppendButton (pageAutox, "", NULL, NULL, false);
	pageAppendButton (pageAutox, "BMS", pageStackSelectCallback, pageBmsPair, false);
	pageAppendButton (pageAutox, "CAN", pageStackSelectCallback, pageCanBusPair, false);

	pageAppendButton (pageBms, "AUTO-X", pageStackSelectCallback, pageAutoxPair, false);
	pageAppendButton (pageBms, "", NULL, NULL, false);
	pageAppendButton (pageBms, "", NULL, NULL, false);
	pageAppendButton (pageBms, "BMS", NULL, NULL, true);
	pageAppendButton (pageBms, "CAN", pageStackSelectCallback, pageCanBusPair, false);

	pageAppendButton (pageCanBus, "AUTO-X", pageStackSelectCallback, pageAutoxPair, false);
	pageAppendButton (pageCanBus, "", NULL, NULL, false);
	pageAppendButton (pageCanBus, "", NULL, NULL, false);
	pageAppendButton (pageCanBus, "BMS", pageStackSelectCallback, pageBmsPair, false);
	pageAppendButton (pageCanBus, "CAN", NULL, NULL, true);

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
	if (argc < 5)
	{
		fprintUsage (stderr);
		return -1;
	}

	// Initialize the CAN device
	char* deviceName = argv [argc - 3];
	canDevice_t* device = canInit (deviceName);
	if (device == NULL)
		return errorPrintf ("Failed to initialize CAN device '%s'", deviceName);

	// Initialize the CAN database
	canDatabase_t database;
	char* dbcPath = argv [argc - 2];
	if (canDatabaseInit (&database, device, dbcPath) != 0)
		return errorPrintf ("Failed to initialize CAN database");

	// Initialize the BMS
	bms_t bms;
	cJSON* bmsConfig = jsonLoad (argv [argc - 1]);
	if (bmsConfig == NULL)
		return errorPrintf ("Failed to load BMS config");
	if (bmsInit (&bms, bmsConfig, &database) != 0)
		return errorPrintf ("Failed to initialize BMS");

	// Create application ID from application name
	char* applicationName = argv [argc - 4];
	char* applicationTitle = getApplicationTitle (applicationName);
	if (applicationTitle == NULL)
		return errorPrintf ("Failed to create application title");

	char* applicationId = getApplicationId (applicationName);
	if (applicationId == NULL)
		return errorPrintf ("Failed to create application name");

	debugPrintf ("Application ID: '%s'\n", applicationId);
	debugPrintf ("Application Title: '%s'\n", applicationTitle);

	activateArg_t arg =
	{
		.applicationTitle	= applicationTitle,
		.applicationId		= applicationId,
		.database			= &database,
		.bms				= &bms
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