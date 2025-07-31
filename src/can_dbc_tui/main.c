// CAN Database TUI -----------------------------------------------------------------------------------------------------------
//
// Author: Cole Barach
// Date Created: 2024.10.27
//
// Description: Terminal user interface for monitoring a CAN bus's traffic in real-time.

// Includes -------------------------------------------------------------------------------------------------------------------

// Includes
#include "can_database/can_database.h"
#include "can_device/can_device.h"
#include "error_codes.h"

// Curses
#ifdef __unix__
#include <curses.h>
#else // __unix__
#include <ncurses/curses.h>
#endif // __unix__

// C Standard Library
#include <errno.h>

// Entrypoint -----------------------------------------------------------------------------------------------------------------

void printTuiDatabaseLine (canDatabase_t* database, uint16_t lineIndex);

int main (int argc, char** argv)
{
	if (argc != 3)
	{
		fprintf (stderr, "Format: can-dbc-tui <device name> <DBC file path>\n");
		return -1;
	}

	char* deviceName = argv [1];
	char* dbcPath = argv [2];

	canDevice_t* device = canInit (deviceName);
	if (device == NULL)
	{
		int code = errno;
		fprintf (stderr, "Failed to create CAN device: %s.\n", errorMessage (code));
		return code;
	}

	// Initialize the database.
	canDatabase_t database;
	if (canDatabaseInit (&database, device, dbcPath) != 0)
	{
		int code = errno;
		fprintf (stderr, "Failed to initialize CAN database: %s.\n", errorMessage (code));
		return code;
	}

	initscr ();

	// Non-blocking input
	cbreak ();
	nodelay (stdscr, true);
	keypad(stdscr, true);

	uint16_t offset = 0;

	while (true)
	{
		clear ();

		int row, col;
		getmaxyx(stdscr, row, col);
		(void) col;

		mvprintw (0, 0, "");

		int ret = getch ();
		if (ret != ERR)
		{
			if (ret == KEY_UP)
				--offset;
			else if (ret == KEY_DOWN)
				++offset;
		}

		for (uint16_t index = 0; index < row; ++index)
			printTuiDatabaseLine (&database, index + offset);

		refresh ();
		napms(16);
	}

	endwin ();

	return 0;
}

void printTuiDatabaseLine (canDatabase_t* database, uint16_t lineIndex)
{
	size_t signalOffset = 0;

	if (lineIndex == 0)
	{
		printw ("%32s | %10s | %8s | %10s | %12s | %12s | %12s | %9s | %6s\n\n",
			"Signal Name", "Value", "Bit Mask", "Bit Length", "Bit Position",
			"Scale Factor", "Offset", "Is Signed", "Endian");
		return;
	}

	uint16_t currentIndex = 0;
	for (size_t messageIndex = 0; messageIndex < database->messageCount; ++messageIndex)
	{
		canMessage_t* message = database->messages + messageIndex;

		++currentIndex;
		if (currentIndex == lineIndex)
		{
			printw ("%s - ID: %3X\n", message->name, message->id);
			return;
		}

		for (size_t signalIndex = 0; signalIndex < message->signalCount; ++signalIndex)
		{
			++currentIndex;

			canSignal_t* signal = message->signals + signalIndex;
			bool valid = database->signalsValid [signalOffset + signalIndex];
			float value = database->signalValues [signalOffset + signalIndex];

			if (currentIndex == lineIndex)
			{
				if (valid)
				{
					printw ("%32s | %10.3f | %8lX | %10i | %12i | %12f | %12f | %9u | %6u\n",
						signal->name, value, signal->bitmask, signal->bitLength, signal->bitPosition,
						signal->scaleFactor, signal->offset, signal->signedness, signal->endianness);
					return;
				}
				else
				{
					printw ("%32s | %10s | %8lX | %10i | %12i | %12f | %12f | %9u | %6u\n",
						signal->name, "--", signal->bitmask, signal->bitLength, signal->bitPosition,
						signal->scaleFactor, signal->offset, signal->signedness, signal->endianness);
					return;
				}
			}
		}

		++currentIndex;
		if (currentIndex == lineIndex)
		{
			printw ("\n");
			return;
		}

		signalOffset += message->signalCount;
	}

	printw ("\n");
}