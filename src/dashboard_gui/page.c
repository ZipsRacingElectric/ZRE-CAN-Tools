// Header
#include "page.h"

void pageAppendButton (page_t* page, const char* label, pageButtonCallback_t* callback, void* arg)
{
	GtkWidget* button = gtk_button_new ();

	if (callback != NULL)
		g_signal_connect (button, "clicked", G_CALLBACK (callback), arg);

	gtk_button_set_label (GTK_BUTTON (button), label);
	gtk_widget_set_hexpand (button, true);
	gtk_widget_set_size_request (button, 0, 90);
	gtk_grid_attach (GTK_GRID (page->vmt.buttonPanel), button, page->vmt.buttonCount, 0, 1, 1);
	++page->vmt.buttonCount;
}