// BMS TUI --------------------------------------------------------------------------------------------------------------------
//
// Authors: Cole Barach, Owen DiBacco
// Date Created: 2025.05.24
//
// Description: Terminal user interface for monitoring a battery management system.

// Includes -------------------------------------------------------------------------------------------------------------------

// Includes
#include "debug.h"
#include "error_codes.h"
#include "bms/bms.h"
#include "can_device/can_device.h"
#include "cjson/cjson_util.h"

// Curses
#ifdef __unix__
#include <curses.h>
#else // __unix__
#include <ncurses/curses.h>
#endif // __unix__

// C Standard Library
#include <errno.h>
#include <locale.h>
#include <math.h>

// Constants ------------------------------------------------------------------------------------------------------------------

#define COLOR_INVALID		1
#define COLOR_VALID			2
#define COLOR_BALANCING		3

// Functions ------------------------------------------------------------------------------------------------------------------

/**
 * @brief Prints a panel of statistics about a BMS.
 * @param row The row to print at.
 * @param column The column to print at.
 * @param bms The BMS to print the stats of.
 */
void printStatPanel (int row, int column, bms_t* bms);

/**
 * @brief Prints all information known about a segment.
 * @param pad The window to display onto.
 * @param row The row to print at.
 * @param column The column to print at.
 * @param bms The BMS owning the segment.
 * @param segmentIndex The index of the segment.
 */
void printSegment (WINDOW* pad, int row, int column, bms_t* bms, uint16_t segmentIndex);

/**
 * @brief Prints the voltage of a cell.
 * @param pad The window to display onto.
 * @param row The row to print at.
 * @param column The column to print at.
 * @param bms The BMS owning the cell.
 * @param cellIndex The global index of the cell.
 */
void printVoltage (WINDOW* pad, int row, int column, bms_t* bms, uint16_t cellIndex);

/**
 * @brief Prints the index of a cell.
 * @param pad The window to display onto.
 * @param row The row to print at.
 * @param column The column to print at.
 * @param index The global index of the cell.
 */
void printCellIndex (WINDOW* pad, int row, int column, uint16_t cellIndex);

/**
 * @brief Prints the temperature of a sense line.
 * @param pad The window to display onto.
 * @param row The row to print at.
 * @param column The column to print at.
 * @param bms The BMS owning the sense line.
 * @param senseLineIndex The global index of the sense line.
 */
void printTemperature (WINDOW* pad, int row, int column, bms_t* bms, uint16_t senseLineIndex);

/**
 * @brief Prints the status of a sense line.
 * @param pad The window to display onto.
 * @param row The row to print at.
 * @param column The column to print at.
 * @param bms The BMS owning the sense line.
 * @param senseLineIndex The global index of the sense line.
 */
void printSenseLineStatus (WINDOW* pad, int row, int column, bms_t* bms, uint16_t senseLineIndex);

/**
 * @brief Prints the index and status of an LTC.
 * @param pad The window to display onto.
 * @param row The row to print at.
 * @param column The column to print at.
 * @param bms The BMS owning the LTC.
 * @param ltcIndex The global index of the LTC.
 */
void printLtcStatus (WINDOW* pad, int row, int column, bms_t* bms, uint16_t ltcIndex);

// Entrypoint -----------------------------------------------------------------------------------------------------------------

int main (int argc, char** argv)
{
	// Debugging initialization
	debugInit ();

	// Validate standard arguments
	if (argc != 4)
	{
		fprintf (stderr, "Format: bms-tui <device name> <DBC file path> <config file path>\n");
		return -1;
	}
	char* deviceName	= argv [1];
	char* dbcPath		= argv [2];
	char* configPath	= argv [3];

	// Initialize the CAN device
	canDevice_t* device = canInit (deviceName);
	if (device == NULL)
	{
		int code = errno;
		fprintf (stderr, "Failed to create CAN device: %s.\n", errorMessage (code));
		return code;
	}

	// Initialize the CAN database
	canDatabase_t database;
	if (canDatabaseInit (&database, device, dbcPath) != 0)
	{
		int code = errno;
		fprintf (stderr, "Failed to initialize CAN database: %s.\n", errorMessage (code));
		return code;
	}

	// Load the BMS config file
	cJSON* config = jsonLoad (configPath);
	if (config == NULL)
	{
		int code = errno;
		fprintf (stderr, "Failed to load config JSON file: %s.\n", errorMessage (code));
		return code;
	}

	// Initialize the BMS object
	bms_t bms;
	if (bmsInit (&bms, config, &database) != 0)
	{
		int code = errno;
		fprintf (stderr, "Failed to initialize the BMS interface: %s.\n", errorMessage (code));
		return code;
	}

	// Set OS-specific locale for wide character support
	#ifdef __unix__
	setlocale (LC_ALL, "");
	#else
	setlocale (LC_ALL, ".utf8");
	#endif // __unix__

	// Initialize Curses
	initscr ();
	if (has_colors() == FALSE)
	{
		endwin ();
		fprintf (stderr, "Terminal environment does not support colors.\n");
		return -1;
	}

	// Text colors
	start_color ();
	init_pair (COLOR_INVALID,	COLOR_RED,		COLOR_BLACK);
	init_pair (COLOR_VALID,		COLOR_GREEN,	COLOR_BLACK);
	init_pair (COLOR_BALANCING,	COLOR_CYAN,		COLOR_BLACK);

	// Setup non-blocking keyboard input
	cbreak ();
	nodelay (stdscr, true);
	keypad(stdscr, true);

	// Get terminal size
	int scr_x, scr_y;
	getmaxyx (stdscr, scr_y, scr_x);

	const int STAT_HEIGHT = 6; // the height of the Stat Panel that will be in the pad
	const int SEGMENT_HEIGHT = 9; // the height of each Segment that will be in the pad
	const int TOTAL_ROWS = STAT_HEIGHT + (bms.segmentCount * SEGMENT_HEIGHT); // the total # of rows in the pad

	WINDOW *pad = newpad (TOTAL_ROWS, scr_x);
	if (!pad) {
		endwin ();
		fprintf (stderr, "Failed to create pad");
		return -1;
	}

	// Initially render segments to the pad
	for (uint16_t segmentIndex = 0; segmentIndex < bms.segmentCount; segmentIndex++) {
		int startingRow = STAT_HEIGHT + (segmentIndex * SEGMENT_HEIGHT);
		printSegment(pad, startingRow, 0, &bms, segmentIndex);
	}

	int offset = 0;
	while (true)
	{
		getmaxyx(stdscr, scr_y, scr_x);

		// Clamp offset
		int max_y = TOTAL_ROWS - scr_y;
		if (max_y < 0) max_y = 0;
		if (offset < 0) offset = 0;
		if (offset > max_y) offset = max_y;

		#ifdef __unix__
		clear ();
		#endif

		// Print the top-most panel
		printStatPanel (0, 0, &bms);
		wrefresh(stdscr);

		// Blit a slice of the pad to the terminal
		int startingRow = offset + STAT_HEIGHT;
		if (startingRow < 0) startingRow = 0;
		if (startingRow > TOTAL_ROWS -1) startingRow = TOTAL_ROWS;

		prefresh (pad, startingRow, 0, STAT_HEIGHT, 0, scr_y - 1, scr_x - 1);
		/*
			prefresh: copies the specified retangle from the pad's data structure to the virtual screen => updates the physical terminal
			pad: pointer to window struct 
			pminrow, pmincol: upper-left corner coords of the rectangle within the pad
			sminrow, smincol: upper left corner coords on the physical screen
			smaxrow, smaxcol: lower right coordinates on the physical screen
		*/

		// Get keyboard input
		int ret = getch ();
		if (ret == KEY_DOWN) {
			offset = (offset + 1 <= max_y) ? offset + 1 : max_y;
			clear ();
		} 
		else if (ret == KEY_UP) {
			offset = (offset - 1 >= 0) ? offset - 1 : 0;
			clear ();
		}
		else if (ret == ' ')
		{
			// Space key should refresh the screen
			endwin ();
			refresh ();
		}
		else if (ret == 'q')
		{
			// Q should quit the application
			break;
		}

		napms (48);
	}

	delwin(pad);
	endwin ();

	return 0;
}

// Functions ------------------------------------------------------------------------------------------------------------------

void printStatPanel (int row, int column, bms_t* bms)
{
	// Get the stats to print

	float cellMin, cellMax, cellAvg;
	bool cellsValid = bmsGetCellVoltageStats (bms, &cellMin, &cellMax, &cellAvg);

	float deltaMax, deltaAvg;
	bool deltasValid = bmsGetCellDeltaStats (bms, &deltaMax, &deltaAvg);

	float tempMin, tempMax, tempAvg;
	bool tempsValid = bmsGetTemperatureStats (bms, &tempMin, &tempMax, &tempAvg);

	// Print the start of the panel
	mvprintw (row + 0, column, "┌─");
	mvprintw (row + 1, column, "│");
	mvprintw (row + 2, column, "│");
	mvprintw (row + 3, column, "│");
	mvprintw (row + 4, column, "└─");
	column += 2;

	// Print the top and bottom
	mvprintw (row + 0, column, "────────────────────");
	mvprintw (row + 4, column, "────────────────────");

	// Print the pack voltage
	float packVoltage;
	bool packVoltageValid = bmsGetPackVoltage (bms, &packVoltage) == CAN_DATABASE_VALID;
	if (packVoltageValid)
	{
		mvprintw (row + 2, column, "Voltage: %-.2f V", packVoltage);
	}
	else
	{
		attron (COLOR_PAIR (COLOR_INVALID));
		mvprintw (row + 2, column, "Voltage: -- V");
		attroff (COLOR_PAIR (COLOR_INVALID));
	}

	// Print the min cell
	if (cellsValid)
	{
		mvprintw (row + 3, column, "Min Cell: %-.2f V", cellMin);
	}
	else
	{
		attron (COLOR_PAIR (COLOR_INVALID));
		mvprintw (row + 3, column, "Min Cell: -- V");
		attroff (COLOR_PAIR (COLOR_INVALID));
	}

	// Next column
	column += 20;

	// Print the top and bottom
	mvprintw (row + 0, column, "────────────────────");
	mvprintw (row + 4, column, "────────────────────");

	// Print the pack current
	float packCurrent;
	bool packCurrentValid = bmsGetPackCurrent (bms, &packCurrent) == CAN_DATABASE_VALID;
	if (packCurrentValid)
	{
		mvprintw (row + 2, column, "Current: %-.2f A", packCurrent);
	}
	else
	{
		attron (COLOR_PAIR (COLOR_INVALID));
		mvprintw (row + 2, column, "Current: -- A");
		attroff (COLOR_PAIR (COLOR_INVALID));
	}

	// Print the max cell
	if (cellsValid)
	{
		mvprintw (row + 3, column, "Max Cell: %-.2f V", cellMax);
	}
	else
	{
		attron (COLOR_PAIR (COLOR_INVALID));
		mvprintw (row + 3, column, "Max Cell: -- V");
		attroff (COLOR_PAIR (COLOR_INVALID));
	}

	// Next column
	column += 20;

	// Print the top and bottom
	mvprintw (row + 0, column, "────────────────────");
	mvprintw (row + 4, column, "────────────────────");

	// Print the power consumption
	if (packVoltageValid && packCurrentValid)
	{
		mvprintw (row + 2, column, "Power: %-.2f W", packVoltage * packCurrent);
	}
	else
	{
		attron (COLOR_PAIR (COLOR_INVALID));
		mvprintw (row + 2, column, "Power: -- W");
		attroff (COLOR_PAIR (COLOR_INVALID));
	}

	// Print the average cell
	if (cellsValid)
	{
		mvprintw (row + 3, column, "Avg Cell: %-.2f V", cellAvg);
	}
	else
	{
		attron (COLOR_PAIR (COLOR_INVALID));
		mvprintw (row + 3, column, "Avg Cell: -- V");
		attroff (COLOR_PAIR (COLOR_INVALID));
	}

	// Next column
	column += 20;

	// Print the top and bottom.
	mvprintw (row + 0, column, "────────────────────");
	mvprintw (row + 4, column, "────────────────────");

	// Print the min temp
	if (tempsValid)
	{
		mvprintw (row + 2, column, "Min Temp: %-.2f C", tempMin);
	}
	else
	{
		attron (COLOR_PAIR (COLOR_INVALID));
		mvprintw (row + 2, column, "Min Temp: -- C");
		attroff (COLOR_PAIR (COLOR_INVALID));
	}

	// Print the max delta
	if (deltasValid)
	{
		mvprintw (row + 3, column, "Max Delta: %-.2f V", deltaMax);
	}
	else
	{
		attron (COLOR_PAIR (COLOR_INVALID));
		mvprintw (row + 3, column, "Max Delta: -- V");
		attroff (COLOR_PAIR (COLOR_INVALID));
	}

	// Next column
	column += 20;

	// Print the top and bottom
	mvprintw (row + 0, column, "────────────────────");
	mvprintw (row + 4, column, "────────────────────");

	// Print the max temp
	if (tempsValid)
	{
		mvprintw (row + 2, column, "Max Temp: %-.2f C", tempMax);
	}
	else
	{
		attron (COLOR_PAIR (COLOR_INVALID));
		mvprintw (row + 2, column, "Max Temp: -- C");
		attroff (COLOR_PAIR (COLOR_INVALID));
	}

	// Print the average delta
	if (deltasValid)
	{
		mvprintw (row + 3, column, "Avg Delta: %-.2f V", deltaAvg);
	}
	else
	{
		attron (COLOR_PAIR (COLOR_INVALID));
		mvprintw (row + 3, column, "Avg Delta: -- V");
		attroff (COLOR_PAIR (COLOR_INVALID));
	}

	// Next column
	column += 20;

	// Print the top and bottom
	mvprintw (row + 0, column, "────────────────────");
	mvprintw (row + 4, column, "────────────────────");

	// Print the average temp
	if (tempsValid)
	{
		mvprintw (row + 2, column, "Avg Temp: %-.2f C", tempAvg);
	}
	else
	{
		attron (COLOR_PAIR (COLOR_INVALID));
		mvprintw (row + 2, column, "Avg Temp: -- C");
		attroff (COLOR_PAIR (COLOR_INVALID));
	}

	// Next column
	column += 20;

	// Print the end of the panel
	mvprintw (row + 0, column, "─┐");
	mvprintw (row + 1, column, " │");
	mvprintw (row + 2, column, " │");
	mvprintw (row + 3, column, " │");
	mvprintw (row + 4, column, "─┘");
}

void printSegment (WINDOW* pad, int row, int column, bms_t* bms, uint16_t segmentIndex)
{
	// Column index for the cells
	uint16_t columnCell = column;

	// Column index for the sense lines
	uint16_t columnSense = column;

	// Print the start of the segment
	// DiBacco: adjust here
	mvwprintw (pad, row + 0, 0, "┌");
	mvwprintw (pad, row + 1, 0, "├");
	mvwprintw (pad, row + 2, 0, "│");
	mvwprintw (pad, row + 3, 0, "│");
	mvwprintw (pad, row + 4, 0, "├┬");
	mvwprintw (pad, row + 5, 0, "││");
	mvwprintw (pad, row + 6, 0, "││");
	mvwprintw (pad, row + 7, 0, "│└");
	mvwprintw (pad, row + 8, 0, "└─");
	
	columnSense += 1;
	columnCell += 2;

	for (uint16_t ltcIndex = 0; ltcIndex < bms->ltcsPerSegment; ++ltcIndex)
	{
		// Print the segment's cells
		for (uint16_t cellIndex = 0; cellIndex < bms->cellsPerLtc; ++cellIndex)
		{
			// Convert local cell index to global cell index.
			uint16_t index = CELL_INDEX_LOCAL_TO_GLOBAL (bms, segmentIndex, ltcIndex, cellIndex);

			// Print the cell's box
			mvwprintw (pad, row + 4, columnCell, "┬─┘└─");
			mvwprintw (pad, row + 5, columnCell, "│    ");
			mvwprintw (pad, row + 6, columnCell, "│    ");
			mvwprintw (pad, row + 7, columnCell, "┴────");
			mvwprintw (pad, row + 8, columnCell, "─────");

			// If this is the first cell, extend the left side to connect to the start of the segement
			if (cellIndex == 0)
			{
				mvwprintw (pad, row + 4, columnCell, "─");
				mvwprintw (pad, row + 5, columnCell, " ");
				mvwprintw (pad, row + 6, columnCell, " ");
				mvwprintw (pad, row + 7, columnCell, "─");
				mvwprintw (pad, row + 8, columnCell, "─");
			}

			// Print the cell voltage text
			printVoltage (pad, row + 5, columnCell + 2, bms, index);
			// Print the cell index text
			// TODO: DiBacco: figure out solution to  3 digit index presses up against the "-" symbol
			printCellIndex (pad, row + 8, columnCell + 2, index);
			
			columnCell += 5;
		}

		if (ltcIndex != bms->ltcsPerSegment - 1)
		{
			// If this is not the last LTC, print the divider for the next one.
			mvwprintw (pad, row + 4, columnCell,	"─┬┬┬");
			mvwprintw (pad, row + 5, columnCell,	" │││");
			mvwprintw (pad, row + 6, columnCell,	" │││");
			mvwprintw (pad, row + 7, columnCell,	"─┘│└");
			mvwprintw (pad, row + 8, columnCell,	"──┴─");

			columnCell += 4;
		}
		else
		{
			// If this is the last LTC, print the end of the segment.
			mvwprintw (pad, row + 4, columnCell,	"─┬┤");
			mvwprintw (pad, row + 5, columnCell,	" ││");
			mvwprintw (pad, row + 6, columnCell,	" ││");
			mvwprintw (pad, row + 7, columnCell,	"─┘│");
			mvwprintw (pad, row + 8, columnCell,	"──┘");
		}

		// Print the segment's sense lines
		for (uint16_t senseLineIndex = 0; senseLineIndex < bms->senseLinesPerLtc; ++senseLineIndex)
		{
			// Convert local sense line index to global sense line index.
			uint16_t index = SENSE_LINE_INDEX_LOCAL_TO_GLOBAL (bms, segmentIndex, ltcIndex, senseLineIndex);

			// Print the sense line's box (or tab).
			uint16_t increment;
			if (senseLineIndex != bms->senseLinesPerLtc - 1)
			{
				mvwprintw (pad, row + 0, columnSense, "─────");
				mvwprintw (pad, row + 1, columnSense, "───╮╭");
				mvwprintw (pad, row + 2, columnSense, "   ├┤");
				mvwprintw (pad, row + 3, columnSense, "   ├┤");
				increment = 5;
			}
			else
			{
				mvwprintw (pad, row + 0, columnSense, "───");
				mvwprintw (pad, row + 1, columnSense, "───");
				mvwprintw (pad, row + 2, columnSense, "   ");
				mvwprintw (pad, row + 3, columnSense, "   ");
				increment = 3;
			}

			// Print the LTC's status. This is done after 6 sense lines have been printed, as otherwise they would print over
			// top this text.
			if (senseLineIndex == 5)
				printLtcStatus (pad, row, columnSense - 24, bms, LTC_INDEX_LOCAL_TO_GLOBAL (bms, segmentIndex, ltcIndex));

			// Print the sense line's temperature and status.
			printTemperature (pad, row + 2, columnSense, bms, index);
			printSenseLineStatus (pad, row + 3, columnSense, bms, index);
			columnSense += increment;
		}

		if (ltcIndex != bms->ltcsPerSegment - 1)
		{
			// If this is not the last LTC, print the divider for the next one.
			mvwprintw (pad, row + 0, columnSense,	"┬");
			mvwprintw (pad, row + 1, columnSense,	"┴");
			mvwprintw (pad, row + 2, columnSense,	"┊");
			mvwprintw (pad, row + 3, columnSense,	"┊");
			columnSense += 1;
		}
		else
		{
			// If this is the last LTC, print the end of the segment.
			mvwprintw (pad, row + 0, columnSense,	"┐");
			mvwprintw (pad, row + 1, columnSense,	"┤");
			mvwprintw (pad, row + 2, columnSense,	"│");
			mvwprintw (pad, row + 3, columnSense,	"│");
		}
	}
}

void printVoltage (WINDOW* pad, int row, int column, bms_t* bms, uint16_t cellIndex)
{
	float voltage;
	bool discharging;

	// If either measurement is invalid, print invalid.
	if (bmsGetCellVoltage (bms, cellIndex, &voltage) != CAN_DATABASE_VALID ||
		bmsGetCellDischarging (bms, cellIndex, &discharging) != CAN_DATABASE_VALID)
	{
		wattron (pad, COLOR_PAIR (COLOR_INVALID));
		mvwprintw (pad, row, column, "--");
		wattroff (pad, COLOR_PAIR (COLOR_INVALID));
		return;
	}

	// If the voltage is not nominal, print in the invalid color.
	bool voltageNominal = voltage >= bms->minCellVoltage && voltage <= bms->maxCellVoltage;
	NCURSES_PAIRS_T color = voltageNominal ? (discharging ? COLOR_BALANCING : COLOR_VALID) : COLOR_INVALID;

	wattron (pad, COLOR_PAIR (color));

	// Print the first 2 digits of the voltage
	uint16_t voltageRounded = (uint16_t) roundf (voltage * 100);
	uint8_t voltageInt = voltageRounded / 100;
	uint8_t voltageFrac = voltageRounded % 100;
	mvwprintw (pad, row, column, "%i.", voltageInt);
	mvwprintw (pad, row + 1, column, "%02i", voltageFrac);

	wattroff (pad, COLOR_PAIR (color));
}

void printCellIndex (WINDOW* pad, int row, int column, uint16_t cellIndex)
{
	// Print the index of the cell
	mvwprintw (pad, row, column, "%i", cellIndex);
}

void printTemperature (WINDOW* pad, int row, int column, bms_t* bms, uint16_t senseLineIndex)
{
	float temperature;
	switch (bmsGetSenseLineTemperature (bms, senseLineIndex, &temperature))
	{
	case CAN_DATABASE_MISSING:
		// If no signal is present, print nothing.
		break;

	case CAN_DATABASE_TIMEOUT:
		// If the signal is timed out, print invalid.
		wattron (pad, COLOR_PAIR (COLOR_INVALID));
		mvwprintw (pad, row, column, " - ");
		wattroff (pad, COLOR_PAIR (COLOR_INVALID));

		break;

	case CAN_DATABASE_VALID:
		// If the temperature is not nominal, print in the invalid color.
		bool temperatureNominal = temperature > bms->minTemperature && temperature < bms->maxTemperature;
		NCURSES_PAIRS_T color = temperatureNominal ? COLOR_VALID : COLOR_INVALID;

		// Print the temperature
		wattron (pad, COLOR_PAIR (color));
		mvwprintw (pad, row, column + 1, "%d\n", (int) temperature);
		mvwprintw (pad, row + 1, column + 1, "%s%.1fC\n", temperature - (int) temperature);

		// TODO: DiBacco: need to split into two lines and possibly remove the C
		// TODO: DiBacco: maybe just look into shrinking the text size
		wattroff (pad, COLOR_PAIR (color));

		break;
	}
}

void printSenseLineStatus (WINDOW* pad, int row, int column, bms_t* bms, uint16_t senseLineIndex)
{
	bool open;
	switch (bmsGetSenseLineOpen (bms, senseLineIndex, &open))
	{
	case CAN_DATABASE_MISSING:
	case CAN_DATABASE_TIMEOUT:
		// If the signal is not valid, print invalid.
		wattron (pad, COLOR_PAIR (COLOR_INVALID));
		mvwprintw (pad, row, column, " - ");
		wattroff (pad, COLOR_PAIR (COLOR_INVALID));
		break;

	case CAN_DATABASE_VALID:
		// If the sense line is open, print so.
		if (open)
		{
			wattron (pad, COLOR_PAIR (COLOR_INVALID));
			mvwprintw (pad, row, column, " XXX ");
			wattroff (pad, COLOR_PAIR (COLOR_INVALID));
		}
	}
}

void printLtcStatus (WINDOW* pad, int row, int column, bms_t* bms, uint16_t ltcIndex)
{
	switch (bmsGetLtcState (bms, ltcIndex))
	{
	case BMS_LTC_STATE_MISSING:
		// If no other information is available, print only the index.
		mvwprintw (pad, row, column, " LTC %i ", ltcIndex);
		break;

	case BMS_LTC_STATE_TIMEOUT:
		// If either signal is timed out, print so.
		wattron (pad, COLOR_PAIR (COLOR_INVALID));
		mvwprintw (pad, row, column, " LTC %i: CAN Timeout ", ltcIndex);
		wattroff (pad, COLOR_PAIR (COLOR_INVALID));
		break;

	case BMS_LTC_STATE_ISOSPI_FAULT:
		// If the LTC is faulted, print so.
		wattron (pad, COLOR_PAIR (COLOR_INVALID));
		mvwprintw (pad, row, column, " LTC %i: IsoSPI Fault ", ltcIndex);
		wattroff (pad, COLOR_PAIR (COLOR_INVALID));
		break;

	case BMS_LTC_STATE_SELF_TEST_FAULT:
		// If the LTC is faulted, print so.
		wattron (pad, COLOR_PAIR (COLOR_INVALID));
		mvwprintw (pad, row, column, " LTC %i: Self-Test Fault ", ltcIndex);
		wattroff (pad, COLOR_PAIR (COLOR_INVALID));
		break;

	case BMS_LTC_STATE_OKAY:
		// If the LTC is okay, print so.
		wattron (pad, COLOR_PAIR (COLOR_VALID));
		mvwprintw (pad, row, column, " LTC %i: Comms Okay ", ltcIndex);
		wattroff (pad, COLOR_PAIR (COLOR_VALID));
		break;
	}

	float temperature;
	switch (bmsGetLtcTemperature (bms, ltcIndex, &temperature))
	{
	case CAN_DATABASE_MISSING:
		// If no temperature information is available, print so.
		break;

	case CAN_DATABASE_TIMEOUT:
		// If the signal is timed out, print so.
		wattron (pad, COLOR_PAIR (COLOR_INVALID));
		printw ("(-- C) ");
		wattron (pad, COLOR_PAIR (COLOR_INVALID));
		break;

	case CAN_DATABASE_VALID:
		// If the temperature is not nominal, print in the invalid color.
		bool temperatureNominal = temperature < bms->maxLtcTemperature;
		NCURSES_PAIRS_T color = temperatureNominal ? COLOR_VALID : COLOR_INVALID;

		// Print the temperature.
		wattron (pad, COLOR_PAIR (color));
		printw ("(%4.2f C) ", temperature);
		wattroff (pad, COLOR_PAIR (color));
	}
}