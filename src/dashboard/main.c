#include <gtk/gtk.h>
#include "debug.h"

static void print_hello (GtkWidget* widget, gpointer data)
{
	(void) widget;
	(void) data;
	g_print ("Hello, World!\n");
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

	/* Create a new window and set it's title*/
	window = gtk_application_window_new (app);
	gtk_window_set_title (GTK_WINDOW (window), title);

	/* Contruct the Container that will hold buttons */
	grid = gtk_grid_new ();

	/* Pack the container in the window */
	gtk_window_set_child (GTK_WINDOW(window), grid);

	button = gtk_button_new_with_label ("Button 1");
	g_signal_connect (button, "clicked", G_CALLBACK (print_hello), NULL);

	gtk_grid_attach (GTK_GRID(grid), button, 0, 0, 1, 1);

	button = gtk_button_new_with_label ("Button 2");
	g_signal_connect (button, "clicked", G_CALLBACK (print_hello), NULL);

	gtk_grid_attach (GTK_GRID(grid), button, 1, 0, 1, 1);

	button = gtk_button_new_with_label ("Quit");
	g_signal_connect (button, "clicked", G_CALLBACK (quit), NULL);
	g_signal_connect_swapped (button, "clicked", G_CALLBACK (gtk_window_destroy), window);

	gtk_grid_attach (GTK_GRID(grid), button, 0, 1, 2, 1);

	gtk_window_present (GTK_WINDOW (window));
}

int main (int argc, char** argv)
{
	
	GtkApplication* app;
	int status;

	if (argc != 2)
	{
		fprintf (stderr, "Invalid usage: dashboard <Application Name>\n");
		return -1;
	}

	debugInit ();

	// Create application ID from application name
	char* applicationName = argv [1];
	const char* APPLICATION_DOMAIN = "org.zre";
	size_t applicationIdSize = strlen (APPLICATION_DOMAIN) + 1 + strlen (applicationName) + 1;
	char* applicationId = malloc (applicationIdSize);
	snprintf (applicationId, applicationIdSize, "%s.%s", APPLICATION_DOMAIN, applicationName);

	app = gtk_application_new (applicationId, G_APPLICATION_DEFAULT_FLAGS);
	g_signal_connect (app, "activate", G_CALLBACK (activate), applicationName);
	status = g_application_run (G_APPLICATION (app), argc - 1, argv);
	g_object_unref (app);

	free (applicationId);
	return status;
}