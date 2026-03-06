#include <gtk/gtk.h>
#include "debug.h"
#include "error_codes.h"
#include "can_device/can_device.h"
#include "can_database/can_database.h"

canDatabase_t database;

static gboolean update_counter(GtkLabel* label[]) {
	g_print("Label address: %p\n", label[0]);
	// Check if label array is valid
    if (label == NULL || label[0] == NULL) {
        g_warning("Invalid label array");
        return FALSE;
    }

    ssize_t index = canDatabaseFindSignal(&database, "BSE_FRONT_PERCENT");
    if (index < 0) {
        // Handle signal not found
        gtk_label_set_text(GTK_LABEL(label[0]), "Error");
        return TRUE;
    }

    float value = 0.0;
    char text[16] = "--";  // Default text

    if (canDatabaseGetFloat(&database, index, &value) == CAN_DATABASE_VALID) {
        snprintf(text, sizeof(text), "%.2f", value);  // Format with 2 decimal places
    }
	GtkLabel* label_p = label[0];
	g_print("Setting Label");
    gtk_label_set_text(label_p, "Word");
    return TRUE; // Continue calling this function
}


/* static void print_hello (GtkWidget* widget, gpointer data)
{
        (void) widget;
        (void) data;
        g_print ("Hello, World!\n");
        for (int i = 1; i <= 10; i++)
        {
                g_print("%d'\n",i);
        }
}
static void winscreen (GtkWidget* widget, GtkWindow* window)
{
        //Default is Fullscreen

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
        (void) widget;
        (void) data;
        g_print ("Quit!\n");
}*/

static void set_window_css(GtkWidget *window) {
    GtkCssProvider *css_provider = gtk_css_provider_new();

    gtk_css_provider_load_from_data(css_provider,
        "window { "
        "  background: linear-gradient(90deg, #e4a089, #67a56f); "
        "}", -1);
    gtk_style_context_add_provider_for_display( gtk_widget_get_display(window), GTK_STYLE_PROVIDER(css_provider), GTK_STYLE_PROVIDER_PRIORITY_USER);
    g_object_unref(css_provider);
}
static void activate(GtkApplication* app, gpointer title) {
    GtkWidget* window;
    GtkWidget* grid;
	GtkWidget* page_sl_box;
	GtkWidget* info_box;
    GtkWidget* speed;
	GtkWidget* data_logger;
	GtkWidget* speed_btn;
	GtkWidget* lap_btn;
	GtkWidget* endr_btn;
	GtkWidget* bms_btn;


    /* Create a new window and set its title */
    window = gtk_application_window_new(app);
    gtk_window_set_title(GTK_WINDOW(window), title);
    gtk_window_set_default_size(GTK_WINDOW(window), 800, 480);

    // Apply CSS to set a gradient background
    set_window_css(window);

	// Setup Grid
    grid = gtk_grid_new();
    
    gtk_window_set_child(GTK_WINDOW(window), grid);

    gtk_widget_set_halign(grid, GTK_ALIGN_CENTER);  
    gtk_widget_set_valign(grid, GTK_ALIGN_CENTER);  
	
	gtk_grid_set_row_spacing(GTK_GRID(grid), 10);
    gtk_grid_set_column_spacing(GTK_GRID(grid), 10);

	speed = gtk_label_new("42");
	gtk_grid_attach(GTK_GRID(grid), speed, 0, 0, 1, 1); 

	gtk_widget_set_halign(speed, GTK_ALIGN_CENTER);  
    gtk_widget_set_valign(speed, GTK_ALIGN_CENTER);  
	

	data_logger = gtk_label_new("Logger");
	gtk_grid_attach(GTK_GRID(grid), data_logger, -1, -1, 1, 1);

	page_sl_box = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 10);
	gtk_grid_attach(GTK_GRID(grid), page_sl_box, -1, 5, 4, 1);
	gtk_box_set_homogeneous(GTK_BOX(page_sl_box), TRUE);

	speed_btn = gtk_button_new_with_label ("Speed");
	lap_btn = gtk_button_new_with_label ("LAP");
	endr_btn = gtk_button_new_with_label ("ENDR");
	bms_btn = gtk_button_new_with_label ("BMS");

	gtk_box_append (GTK_BOX (page_sl_box), speed_btn);
  	gtk_box_append (GTK_BOX (page_sl_box), lap_btn);
	gtk_box_append (GTK_BOX (page_sl_box), endr_btn);
  	gtk_box_append (GTK_BOX (page_sl_box), bms_btn);

    //g_print("Label address: %p\n", can_label[0]);
	//g_timeout_add(33, (GSourceFunc)update_counter, can_label[0]);

    gtk_window_present(GTK_WINDOW(window));
}

int main (int argc, char** argv)
{

        int status;

        if (argc != 4)
        {
                fprintf (stderr, "Invalid usage: dashboard <Application Name> <Device Name> <DBC File Path> \n");
                return -1;
        }

        debugInit ();

        canDevice_t* device = canInit(argv [2]);
        if (device == NULL){
                fprintf (stderr , "Invalid Device Name, %s \n" , errorCodeToMessage(errno));
                return errno;
        }

        if (canDatabaseInit(&database, device, argv [3]) != 0){
                //argv [3] = DBC File Path
                fprintf (stderr , "Invalid DBC File Path, %s \n" , errorCodeToMessage(errno));
                return errno;
        }

        // Create application ID from application name
        char* applicationName = argv [1];
        const char* APPLICATION_DOMAIN = "org.zre";
        size_t applicationIdSize = strlen (APPLICATION_DOMAIN) + 1 + strlen (applicationName) + 1;
        char* applicationId = malloc (applicationIdSize);
        snprintf (applicationId, applicationIdSize, "%s.%s", APPLICATION_DOMAIN, applicationName);

        GtkApplication* app = gtk_application_new (applicationId, 0);
        g_signal_connect (app, "activate", G_CALLBACK (activate), applicationName);
        status = g_application_run (G_APPLICATION (app), argc - 3, argv);
        g_object_unref (app);

        free (applicationId);
        return status;
}
