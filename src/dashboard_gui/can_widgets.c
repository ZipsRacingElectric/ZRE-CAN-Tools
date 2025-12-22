// Header
#include "can_widgets.h"

// TODO(Barach): Move these to can_database_stdio.h

int fprintCanDatabaseFloat (FILE* stream, canDatabase_t* database, ssize_t index, const char* formatValue,
	const char* formatInvalid)
{
	// Get signal reference, also check existence
	canSignal_t* signal = canDatabaseGetSignal (database, index);
	if (signal == NULL)
		return fprintf (stream, formatInvalid, "--", "-");

	float value;
	if (canDatabaseGetFloat (database, index, &value) != CAN_DATABASE_VALID)
	{
		// Print '--' as a string using the user-provided format string. The unit is ignored if not '%s' specifier is added.
		return fprintf (stream, formatInvalid, "--", signal->unit);
	}

	// Print the value as a float using the user-provided format string. The unit is ignored if not '%s' specifier is added.
	return fprintf (stream, formatValue, value, signal->unit);
}

int snprintCanDatabaseFloat (char* str, size_t n, canDatabase_t* database, ssize_t index, const char* formatValue,
	const char* formatInvalid)
{
	// Get signal reference, also check existence
	canSignal_t* signal = canDatabaseGetSignal (database, index);
	if (signal == NULL)
		return snprintf (str, n, formatInvalid, "--", "-");

	float value;
	if (canDatabaseGetFloat (database, index, &value) != CAN_DATABASE_VALID)
	{
		// Print '--' as a string using the user-provided format string. The unit is ignored if not '%s' specifier is added.
		return snprintf (str, n, formatInvalid, "--", signal->unit);
	}

	// Print the value as a float using the user-provided format string. The unit is ignored if not '%s' specifier is added.
	return snprintf (str, n, formatValue, value, signal->unit);
}

void canLabelFloatInit (canLabelFloat_t* label, canDatabase_t* database)
{
	label->database = database;
	label->widget = gtk_label_new ("");
	label->index = canDatabaseFindSignal (database, label->signalName);
	canLabelFloatUpdate (label);
}

void canLabelFloatUpdate (canLabelFloat_t* label)
{
	char text [16] = "";
	snprintCanDatabaseFloat (text, sizeof (text), label->database, label->index, label->formatValue,
		label->formatInvalid);
	gtk_label_set_text (CAN_LABEL_FLOAT_TO_LABEL (label), text);
}

void canLabelBoolInit (canLabelBool_t* label, canDatabase_t* database)
{
	label->database = database;
	label->widget = gtk_label_new (label->invalidValue);
	label->index = canDatabaseFindSignal (database, label->signalName);
	canLabelBoolUpdate (label);
}

void canLabelBoolUpdate (canLabelBool_t* label)
{
	canLabelBoolState_t state;

	float value;
	if (canDatabaseGetFloat (label->database, label->index, &value) == CAN_DATABASE_VALID)
	{
		if ((value >= label->threshold) != label->inverted)
			state = CAN_LABEL_BOOL_ACTIVE;
		else
			state = CAN_LABEL_BOOL_INACTIVE;
	}
	else
		state = CAN_LABEL_BOOL_INVALID;

	if (state != label->state)
	{
		label->state = state;

		switch (state)
		{
		case CAN_LABEL_BOOL_ACTIVE:
			gtk_label_set_text (CAN_LABEL_BOOL_TO_LABEL (label), label->activeValue);
			break;

		case CAN_LABEL_BOOL_INACTIVE:
			gtk_label_set_text (CAN_LABEL_BOOL_TO_LABEL (label), label->inactiveValue);
			break;

		case CAN_LABEL_BOOL_INVALID:
			gtk_label_set_text (CAN_LABEL_BOOL_TO_LABEL (label), label->invalidValue);
			break;
		}

	}
}

void canProgessBarInit (canProgressBar_t* bar, canDatabase_t* database)
{
	bar->database = database;
	bar->widget = gtk_progress_bar_new ();
	bar->index = canDatabaseFindSignal (database, bar->signalName);
	canProgressBarUpdate (bar);
}

void canProgressBarUpdate (canProgressBar_t* bar)
{
	float value;
	if (canDatabaseGetFloat (bar->database, bar->index, &value) != CAN_DATABASE_VALID)
		value = bar->min;

	// Inverse linear interpolation
	value = (value - bar->min) / (bar->max - bar->min);

	gtk_progress_bar_set_fraction (CAN_PROGRESS_BAR_TO_PROGRESS_BAR (bar), value);
}

static void draw (GtkDrawingArea* area, cairo_t* cr, int width, int height, gpointer arg)
{
	(void) area;
	canIndicator_t* indicator = arg;

	switch (indicator->state)
	{
	case CAN_INDICATOR_ACTIVE:
		gdk_cairo_set_source_rgba (cr, &indicator->activeColor);
		break;

	case CAN_INDICATOR_INACTIVE:
		gdk_cairo_set_source_rgba (cr, &indicator->inactiveColor);
		break;

	case CAN_INDICATOR_INVALID:
		gdk_cairo_set_source_rgba (cr, &indicator->invalidColor);
		break;
	}

	if (indicator->pointCount == 0)
	{
		cairo_arc (cr, width / 2.0, height / 2.0, MIN (width, height) / 2.0, 0, 2 * G_PI);
	}
	else
	{
		cairo_move_to (cr, width * indicator->points [0][0], height * indicator->points [0][1]);

		for (size_t index = 1; index < indicator->pointCount; ++index)
			cairo_line_to (cr, roundf (width * indicator->points [index][0]), roundf (height * indicator->points [index][1]));

		cairo_close_path (cr);
	}

	cairo_fill (cr);
}

void canIndicatorInit (canIndicator_t* indicator, canDatabase_t* database, int width, int height)
{
	indicator->widget = gtk_drawing_area_new ();
	indicator->database = database;
	indicator->index = canDatabaseFindSignal (database, indicator->signalName);

	gtk_drawing_area_set_content_width (CAN_INDICATOR_TO_DRAWING_AREA (indicator), width);
	gtk_drawing_area_set_content_height (CAN_INDICATOR_TO_DRAWING_AREA (indicator), height);
	gtk_drawing_area_set_draw_func (CAN_INDICATOR_TO_DRAWING_AREA (indicator), draw, indicator, NULL);

	// For some reason, drawing area appears to stretch to fit area by default. Disable this so it defaults to the minimum size.
	gtk_widget_set_halign (CAN_INDICATOR_TO_WIDGET (indicator), GTK_ALIGN_CENTER);
	gtk_widget_set_valign (CAN_INDICATOR_TO_WIDGET (indicator), GTK_ALIGN_CENTER);
}

void canIndicatorUpdate (canIndicator_t* indicator)
{
	canIndicatorState_t state;

	float value;
	if (canDatabaseGetFloat (indicator->database, indicator->index, &value) == CAN_DATABASE_VALID)
	{
		if ((value >= indicator->threshold) != indicator->inverted)
			state = CAN_INDICATOR_ACTIVE;
		else
			state = CAN_INDICATOR_INACTIVE;
	}
	else
		state = CAN_INDICATOR_INVALID;

	if (state != indicator->state)
	{
		indicator->state = state;
		gtk_widget_queue_draw (CAN_INDICATOR_TO_WIDGET (indicator));
	}
}