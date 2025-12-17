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

void canLabelInit (canLabel_t* label, canDatabase_t* database)
{
	label->database = database;
	label->widget = gtk_label_new ("");
	label->index = canDatabaseFindSignal (database, label->signalName);
	canLabelUpdate (label);
}

void canLabelUpdate (canLabel_t* label)
{
	char text [16] = "";
	snprintCanDatabaseFloat (text, sizeof (text), label->database, label->index, label->formatValue,
		label->formatInvalid);
	gtk_label_set_text (CAN_LABEL_TO_LABEL (label), text);
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