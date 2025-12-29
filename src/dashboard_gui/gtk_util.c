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

void gtkLabelSetColor (GtkLabel* label, const char* color)
{
	// Get the label's pango attributes, create a new list if none exist.
	PangoAttrList* list = gtk_label_get_attributes (label);
	if (list == NULL)
		list = pango_attr_list_new ();

	PangoColor pangoColor;
	guint16 pangoAlpha;
	if (!pango_color_parse_with_alpha(&pangoColor, &pangoAlpha, color))
		pangoColor = (PangoColor) { 0, 0, 0 };

	PangoAttribute* attr = pango_attr_foreground_new (pangoColor.red, pangoColor.green, pangoColor.blue);
	pango_attr_list_insert (list, attr);

	// TODO(Barach): Does alpha ever vary?
	gtk_label_set_attributes (label, list);
	gtk_widget_set_opacity (GTK_WIDGET (label), pangoAlpha / 65536.0f);
}

GdkRGBA gdkHexToColor (const char* color)
{
	PangoColor pangoColor;
	guint16 pangoAlpha;
	if (!pango_color_parse_with_alpha(&pangoColor, &pangoAlpha, color))
		pangoColor = (PangoColor) { 0, 0, 0 };

	return (GdkRGBA)
	{
		.red	= pangoColor.red	/ 65536.0f,
		.green	= pangoColor.green	/ 65536.0f,
		.blue	= pangoColor.blue	/ 65536.0f,
		.alpha	= pangoAlpha		/ 65536.0f
	};
}