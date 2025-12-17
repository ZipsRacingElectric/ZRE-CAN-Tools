// Header
#include "gtk_util.h"

void gtkLabelSetFont (GtkLabel* label, const char* pangoFontDescriptor)
{
	// Get the label's pango attributes, create a new list if none exist.
	PangoAttrList* list = gtk_label_get_attributes (label);
	if (list == NULL)
		list = pango_attr_list_new ();

	PangoFontDescription* fontDesc = pango_font_description_from_string (pangoFontDescriptor);
	PangoAttribute* attr = pango_attr_font_desc_new (fontDesc);
	pango_attr_list_insert (list, attr);

	gtk_label_set_attributes (label, list);
}