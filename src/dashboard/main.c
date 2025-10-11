#include <gtk/gtk.h>

static void print_hello (GtkWidget* widget, gpointer data)
{
	g_print ("Hello, World!\n");
}

static void activate (GtkApplication* app, gpointer title)
{
	GtkWidget* window = gtk_application_window_new (app);
	gtk_window_set_title (GTK_WINDOW (window), title);

	GtkWidget* button = gtk_button_new_with_label ("Hello?");
	g_signal_connect (button, "clicked", G_CALLBACK (print_hello), NULL);
	gtk_window_set_child (GTK_WINDOW (window), button);

	gtk_window_set_default_size (GTK_WINDOW (window), 200, 200);
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