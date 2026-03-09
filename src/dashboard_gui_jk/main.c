#include <gtk/gtk.h>
#include "debug.h"
#include "error_codes.h"
#include "can_device/can_device.h"
#include "can_database/can_database.h"

canDatabase_t database;

static gboolean speed_can(GtkLabel* label) {

    ssize_t index = canDatabaseFindSignal(&database, "BSE_FRONT_PERCENT");
    if (index < 0) {
        gtk_label_set_text(GTK_LABEL(label), "Error");
        return TRUE;
    }

    float value = 0.0;
    char text[16] = "--";  // Default text

    if (canDatabaseGetFloat(&database, index, &value) == CAN_DATABASE_VALID) {
        snprintf(text, sizeof(text), "%.2f", value); 
    }

    gtk_label_set_text(GTK_LABEL(label), text);
    return TRUE;
}

static gboolean logger_can(GtkLabel* label) {

    ssize_t index = canDatabaseFindSignal(&database, "BSE_FRONT_PERCENT");
    if (index < 0) {
        gtk_label_set_text(GTK_LABEL(label), "Error");
        return TRUE;
    }

    float value = 0.0;
    char text[16] = "--";  // Default text

    if (canDatabaseGetFloat(&database, index, &value) == CAN_DATABASE_VALID) {
        snprintf(text, sizeof(text), "%.2f", value); 
    }

    gtk_label_set_text(GTK_LABEL(label), text);
    return TRUE;
}

static gboolean bms_max_can(GtkLabel* label) {

    ssize_t index = canDatabaseFindSignal(&database, "BSE_FRONT_PERCENT");
    if (index < 0) {
        gtk_label_set_text(GTK_LABEL(label), "Error");
        return TRUE;
    }

    float value = 0.0;
    char text[16] = "--";  // Default text

    if (canDatabaseGetFloat(&database, index, &value) == CAN_DATABASE_VALID) {
        snprintf(text, sizeof(text), "%.2f", value); 
    }

    gtk_label_set_text(GTK_LABEL(label), text);
    return TRUE;
}

static gboolean bms_avg_can(GtkLabel* label) {

    ssize_t index = canDatabaseFindSignal(&database, "BSE_FRONT_PERCENT");
    if (index < 0) {
        gtk_label_set_text(GTK_LABEL(label), "Error");
        return TRUE;
    }

    float value = 0.0;
    char text[16] = "--";  // Default text

    if (canDatabaseGetFloat(&database, index, &value) == CAN_DATABASE_VALID) {
        snprintf(text, sizeof(text), "%.2f", value); 
    }

    gtk_label_set_text(GTK_LABEL(label), text);
   
	return TRUE;
}

static gboolean vcu_faults_can(GtkLabel* label) {

    ssize_t index = canDatabaseFindSignal(&database, "BSE_FRONT_PERCENT");
    if (index < 0) {
        gtk_label_set_text(GTK_LABEL(label), "Error");
        return TRUE;
    }

    float value = 0.0;
    char text[16] = "--";  // Default text

    if (canDatabaseGetFloat(&database, index, &value) == CAN_DATABASE_VALID) {
        snprintf(text, sizeof(text), "%.2f", value); 
    }

    gtk_label_set_text(GTK_LABEL(label), text);
    return TRUE;
}

static gboolean mtr_temp_can(GtkLabel* label) {

    ssize_t index = canDatabaseFindSignal(&database, "BSE_FRONT_PERCENT");
    if (index < 0) {
        gtk_label_set_text(GTK_LABEL(label), "Error");
        return TRUE;
    }

    float value = 0.0;
    char text[16] = "--";  // Default text

    if (canDatabaseGetFloat(&database, index, &value) == CAN_DATABASE_VALID) {
        snprintf(text, sizeof(text), "%.2f", value); 
    }

    gtk_label_set_text(GTK_LABEL(label), text);
    return TRUE;
}

static gboolean inv_temp_can(GtkLabel* label) {

    ssize_t index = canDatabaseFindSignal(&database, "BSE_FRONT_PERCENT");
    if (index < 0) {
        gtk_label_set_text(GTK_LABEL(label), "Error");
        return TRUE;
    }

    float value = 0.0;
    char text[16] = "--";  // Default text

    if (canDatabaseGetFloat(&database, index, &value) == CAN_DATABASE_VALID) {
        snprintf(text, sizeof(text), "%.2f", value); 
    }

    gtk_label_set_text(GTK_LABEL(label), text);
    return TRUE;
}

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
    GtkWidget* speed;
	GtkWidget* data_logger;
	GtkWidget* speed_btn;
	GtkWidget* lap_btn;
	GtkWidget* endr_btn;
	GtkWidget* bms_btn;
	GtkWidget* Bms_VCU_AMK_label;
	GtkWidget* bms_max;
	GtkWidget* bms_avg;
	GtkWidget* vcu_faults;
	GtkWidget* mtr_temp;
	GtkWidget* inv_temp;

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
	gtk_grid_attach(GTK_GRID(grid), speed, 0, 0, 1, 7); 

	gtk_widget_set_halign(speed, GTK_ALIGN_CENTER);  
    gtk_widget_set_valign(speed, GTK_ALIGN_CENTER);  
	

	data_logger = gtk_label_new("Logger");
	gtk_grid_attach(GTK_GRID(grid), data_logger, -1, -1, 1, 1);

	Bms_VCU_AMK_label = gtk_label_new("BMS");
	gtk_grid_attach(GTK_GRID(grid), Bms_VCU_AMK_label, 2, 1, 2, 1);
	Bms_VCU_AMK_label = gtk_label_new("Max:");
	gtk_grid_attach(GTK_GRID(grid), Bms_VCU_AMK_label, 2, 2, 1, 1);
	bms_max = gtk_label_new("23C");
	gtk_grid_attach(GTK_GRID(grid), bms_max, 3, 2, 1, 1);
	Bms_VCU_AMK_label = gtk_label_new("AVG:");
	gtk_grid_attach(GTK_GRID(grid), Bms_VCU_AMK_label, 2, 3, 1, 1);
	bms_avg = gtk_label_new("23C");
	gtk_grid_attach(GTK_GRID(grid), bms_avg, 3, 3, 1, 1);
	Bms_VCU_AMK_label = gtk_label_new("VCU:");
	gtk_grid_attach(GTK_GRID(grid), Bms_VCU_AMK_label, 2, 4, 1, 1);
	vcu_faults = gtk_label_new("No Faults");
	gtk_grid_attach(GTK_GRID(grid), vcu_faults, 4, 4, 1, 1);
	Bms_VCU_AMK_label = gtk_label_new("AMK");
	gtk_grid_attach(GTK_GRID(grid), Bms_VCU_AMK_label, 2, 5, 2, 1);
	Bms_VCU_AMK_label = gtk_label_new("MTR:");
	gtk_grid_attach(GTK_GRID(grid), Bms_VCU_AMK_label, 2, 6, 1, 1);
	mtr_temp = gtk_label_new("91C");
	gtk_grid_attach(GTK_GRID(grid), mtr_temp, 3, 6, 1, 1);
	Bms_VCU_AMK_label = gtk_label_new("INV:");
	gtk_grid_attach(GTK_GRID(grid), Bms_VCU_AMK_label, 2, 7, 1, 1);
	inv_temp = gtk_label_new("45C");
	gtk_grid_attach(GTK_GRID(grid), inv_temp, 3, 7, 1, 1);

	page_sl_box = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 10);
	gtk_grid_attach(GTK_GRID(grid), page_sl_box, -1, 8, 4, 1);
	gtk_box_set_homogeneous(GTK_BOX(page_sl_box), TRUE);

	speed_btn = gtk_button_new_with_label ("Speed");
	lap_btn = gtk_button_new_with_label ("LAP");
	endr_btn = gtk_button_new_with_label ("ENDR");
	bms_btn = gtk_button_new_with_label ("BMS");

	gtk_box_append (GTK_BOX (page_sl_box), speed_btn);
  	gtk_box_append (GTK_BOX (page_sl_box), lap_btn);
	gtk_box_append (GTK_BOX (page_sl_box), endr_btn);
  	gtk_box_append (GTK_BOX (page_sl_box), bms_btn);
	
	//Don't worry about it 
	g_timeout_add(33, (GSourceFunc)speed_can, speed);
	g_timeout_add(33, (GSourceFunc)logger_can, data_logger);
	g_timeout_add(33, (GSourceFunc)bms_max_can, bms_max);
	g_timeout_add(33, (GSourceFunc)bms_avg_can, bms_avg);
	g_timeout_add(33, (GSourceFunc)vcu_faults_can, vcu_faults);
	g_timeout_add(33, (GSourceFunc)mtr_temp_can, mtr_temp);
	g_timeout_add(33, (GSourceFunc)inv_temp_can, inv_temp);

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
