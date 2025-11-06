#include <gtk/gtk.h>
#include "debug.h"
#include "can_device/can_device.h"
#include "can_database/can_database.h"

canDatabase_t database; 

static gboolean update_counter(GtkWidget *label) {
   
	ssize_t index = canDatabaseFindSignal(&database, "BSE_FRONT_PERCENT"); //Check for faliure
	float value;
	canDatabaseGetFloat(&database, index, &value); //Check return code
    char text[16];  // Enough space for numbers 1-30
    snprintf(text, sizeof(text), "%f", value);
    gtk_label_set_text(GTK_LABEL(label), text);
    return TRUE; // Continue calling this function
}

static void print_hello (GtkWidget* widget, gpointer data)
{
	g_print ("Hello, World!\n");
	for (int i = 1; i <= 10; i++)
	{
		g_print("%d'\n",i);
	}
}
static void winscreen (GtkWidget* widget, GtkWindow* window)
{
	//**Default is Fullscreen**//

	//Set Window Unfullscreen
	if (gtk_window_is_fullscreen (GTK_WINDOW(window)) == true)
	{
		g_print ("Unfullscreen\n");
		gtk_button_set_label(GTK_BUTTON(widget), "Fullscreen");
		gtk_window_unfullscreen (GTK_WINDOW(window));
	}

	//Set Window Fullscreen
	else if (gtk_window_is_fullscreen (GTK_WINDOW(window)) == false)
	{
		g_print ("Fullscreen\n");
		gtk_button_set_label(GTK_BUTTON(widget), "Unfullscreen");
		gtk_window_fullscreen (GTK_WINDOW(window));
	}
}

static void quit (GtkWidget* widget, gpointer data)
{
	g_print ("Quit!\n");
}

static void activate (GtkApplication* app, gpointer title)
{
	GtkWidget* window;
	GtkWidget* button;
	GtkWidget* grid;
	GtkWidget* label;

	/* Create a new window and set it's title*/
	window = gtk_application_window_new (app);
	gtk_window_set_title (GTK_WINDOW (window), title);
	gtk_window_fullscreen (GTK_WINDOW(window));

	/* Contruct the Container that will hold buttons */
	grid = gtk_grid_new ();

	/* Pack the container in the window */
	gtk_window_set_child (GTK_WINDOW(window), grid);

	label = gtk_label_new("Hello World");
	gtk_grid_attach (GTK_GRID(grid), label, 0, 2, 5, 1);

	button = gtk_button_new_with_label ("Button 1");
	g_signal_connect (button, "clicked", G_CALLBACK (print_hello), NULL);

	gtk_grid_attach (GTK_GRID(grid), button, 0, 0, 1, 1);

	button = gtk_button_new_with_label ("Unfullscreen");
	g_signal_connect (button, "clicked", G_CALLBACK (winscreen), window);

	gtk_grid_attach (GTK_GRID(grid), button, 1, 0, 1, 1);

	button = gtk_button_new_with_label ("Quit");
	g_signal_connect (button, "clicked", G_CALLBACK (quit), NULL);
	g_signal_connect_swapped (button, "clicked", G_CALLBACK (gtk_window_destroy), window);

	gtk_grid_attach (GTK_GRID(grid), button, 0, 1, 2, 1);
	
	g_timeout_add(33, (GSourceFunc)update_counter, label);

	gtk_window_present (GTK_WINDOW (window));
}

int main (int argc, char** argv)
{
	
	GtkApplication* app;
	int status;

	if (argc != 4)
	{
		fprintf (stderr, "Invalid usage: dashboard <Application Name> <Device Name> <DBC File Path>");
		return -1;
	}

	debugInit ();

	canDevice_t* device = canInit(argv [2]); //TODO Check if Null
	canDatabaseInit(&database, device, argv [3]); //argv [3] = DBC File Path Check return code of the function

	// Create application ID from application name
	char* applicationName = argv [1];
	const char* APPLICATION_DOMAIN = "org.zre";
	size_t applicationIdSize = strlen (APPLICATION_DOMAIN) + 1 + strlen (applicationName) + 1;
	char* applicationId = malloc (applicationIdSize);
	snprintf (applicationId, applicationIdSize, "%s.%s", APPLICATION_DOMAIN, applicationName);

	app = gtk_application_new (applicationId, G_APPLICATION_DEFAULT_FLAGS);
	g_signal_connect (app, "activate", G_CALLBACK (activate), applicationName);
	status = g_application_run (G_APPLICATION (app), argc - 3, argv);
	g_object_unref (app);

	free (applicationId);
	return status;
}