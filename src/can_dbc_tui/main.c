// CAN Database TUI -----------------------------------------------------------------------------------------------------------
//
// Author: Cole Barach
// Date Created: 2024.10.27
//
// Description: See help page.

// Includes -------------------------------------------------------------------------------------------------------------------

// Includes
#include "can_database/can_database.h"
#include "can_database/can_database_stdio.h"
#include "can_device/can_device.h"
#include "can_device/can_device_stdio.h"
#include "debug.h"
#include "options.h"

// Curses
#include "curses_port.h"

// C Standard Library
#include <inttypes.h>
#include <locale.h>

// Function Prototypes --------------------------------------------------------------------------------------------------------

void printDatabase (canDatabase_t* database, size_t startRow, size_t endRow);

// Functions ------------------------------------------------------------------------------------------------------------------

void fprintUsage (FILE* stream)
{
	fprintf (stream, "Usage: can-dbc-tui <Options> <Device Name> <DBC file path>\n");
}

void fprintHelp (FILE* stream)
{
	fprintf (stream, ""
		"can-dbc-tui - Terminal user interface used to monitor the activity of a CAN bus\n"
		"              in real-time.\n\n");

	fprintUsage (stream);

	fprintf (stream, "\nParameters:\n\n");
	fprintCanDeviceNameHelp (stream, "    ");
	fprintCanDbcFileHelp (stream, "    ");

	fprintf (stream, "Options:\n\n");
	fprintOptionHelp (stream, "    ");
}

// Entrypoint -----------------------------------------------------------------------------------------------------------------

int main (int argc, char** argv)
{
	// Debug initialization
	debugInit ();

	// Check standard arguments
	for (int index = 1; index < argc; ++index)
	{
		switch (handleOption (argv [index], NULL, fprintHelp))
		{
		case OPTION_CHAR:
		case OPTION_STRING:
			fprintf (stderr, "Unknown argument '%s'.\n", argv [index]);
			return -1;

		case OPTION_QUIT:
			return 0;

		default:
			break;
		}
	}

	// Validate the usage
	if (argc < 3)
	{
		fprintUsage (stderr);
		return -1;
	}

	// Initialize the CAN device
	char* deviceName = argv [argc - 2];
	canDevice_t* device = canInit (deviceName);
	if (device == NULL)
		return errorPrintf ("Failed to initialize CAN device '%s'", deviceName);

	// Initialize the CAN database
	char* dbcPath = argv [argc - 1];
	canDatabase_t database;
	if (canDatabaseInit (&database, device, dbcPath) != 0)
		return errorPrintf ("Failed to initialize CAN database");

	// Set the locale for wide character support
	cursesSetWideLocale ();

	initscr ();

	// Non-blocking input
	cbreak ();
	nodelay (stdscr, true);
	keypad (stdscr, true);

	int offset = 0;

	while (true)
	{
		#ifdef ZRE_CANTOOLS_OS_linux
		clear ();
		#endif

		// Get terminal dimensions
		int row, col;
		getmaxyx (stdscr, row, col);
		(void) col;

		// Print controls
		mvprintw (0, 0, " Controls");
		mvprintw (0, 32, "│ Space .... Refresh the screen");
		mvprintw (0, 64, "│ Up/Down .............. Scroll");
		mvprintw (0, 96, "│ Q ...................... Quit │");
		for (int i = 0; i < col; ++i)
		{
			if (i % 32 == 0 && i > 0 && i <= 128)
				mvprintw (1, i, "┴");
			else
				mvprintw (1, i, "─");
		}
		move (2, 0);

		int ret = getch ();
		if (ret != ERR)
		{
			// Space key should refresh the screen
			if (ret == ' ')
			{
				endwin ();
				refresh ();
			}
			// Up to scroll up
			if (ret == KEY_UP)
			{
				offset -= 4;
				if (offset < 0)
					offset = 0;
			}
			// Down to scroll down
			else if (ret == KEY_DOWN)
				offset += 4;
			// Q quits application
			else if (ret == 'q')
				break;
		}

		// Print the database (starts at row 2, ends at the end of screen)
		printDatabase (&database, offset, row + offset - 2);

		refresh ();
		napms (48);
	}

	endwin ();

	canDealloc (device);
	return 0;
}

// Functions ------------------------------------------------------------------------------------------------------------------

void printDatabase (canDatabase_t* database, size_t startRow, size_t endRow)
{
	size_t currentRow = 0;

	for (size_t messageIndex = 0; messageIndex < database->messageCount; ++messageIndex)
	{
		canMessage_t* message = &database->messages [messageIndex];

		// Print message name & ID
		if (startRow <= currentRow && currentRow < endRow)
		{
			char buffer [130];
			int pos = snprintf (buffer, sizeof (buffer), "%s - ID ", message->name);
			if (pos >= 0 && (size_t) pos <= sizeof (buffer) - 1)
				snprintCanId (buffer + pos, sizeof (buffer) - pos, message->id, message->ide, false);

			printw ("┌─ %s ", buffer);
			for (size_t index = strlen (buffer) + 4; index < 138; ++index)
			{
				if (index == 35 || index == 48 || index == 59 || index == 72 || index == 87 || index == 102 || index == 117
					|| index == 129)
					printw ("┬");
				else
					printw ("─");
			}
			printw ("┐\n");
		}
		++currentRow;

		// Print column titles
		if (startRow <= currentRow && currentRow < endRow)
			printw ("│ %32s │ %10s │ %8s │ %10s │ %12s │ %12s │ %12s │ %9s │ %6s │\n",
				"Signal Name",
				"Value",
				"Bit Mask",
				"Bit Length",
				"Bit Position",
				"Scale Factor",
				"Offset",
				"Is Signed",
				"Endian");
		++currentRow;

		// Print divider
		if (startRow <= currentRow && currentRow < endRow)
		{
			printw ("│");
			for (size_t index = 1; index < 138; ++index)
			{
				if (index == 35 || index == 48 || index == 59 || index == 72 || index == 87 || index == 102 || index == 117
					|| index == 129)
					printw ("│");
				else
					printw ("┄");
			}
			printw ("┤\n");
		}
		++currentRow;

		// Print signals
		for (size_t signalIndex = 0; signalIndex < message->signalCount; ++signalIndex)
		{
			canSignal_t* signal = &message->signals [signalIndex];
			ssize_t globalIndex = canDatabaseGetGlobalIndex (database, messageIndex, signalIndex);

			char buffer [11] = "--";
			float value;
			if (canDatabaseGetFloat (database, globalIndex, &value) == CAN_DATABASE_VALID)
				snprintf (buffer, sizeof (buffer), "%.3f", value);

			// Print signal name, value, and metadata
			if (startRow <= currentRow && currentRow < endRow)
			{
				printw ("│ %32s │ %10s │ %8"PRIX64" │ %10i │ %12i │ %12f │ %12f │ %9u │ %6u │\n",
					signal->name,
					buffer,
					signal->bitmask,
					signal->bitLength,
					signal->bitPosition,
					signal->scaleFactor,
					signal->offset,
					signal->signedness,
					signal->endianness);
			}

			++currentRow;
		}

		// Print end of message divider
		if (startRow <= currentRow && currentRow < endRow)
		{
			printw ("└");
			for (size_t index = 1; index < 138; ++index)
			{
				if (index == 35 || index == 48 || index == 59 || index == 72 || index == 87 || index == 102 || index == 117
					|| index == 129)
					printw ("┴");
				else
					printw ("─");
			}
			printw ("┘\n");
		}
		++currentRow;

		// Print gap
		if (startRow <= currentRow && currentRow < endRow)
			printw ("\n");

		++currentRow;
	}
}