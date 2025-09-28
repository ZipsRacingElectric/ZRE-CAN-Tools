// BMS TUI --------------------------------------------------------------------------------------------------------------------
//
// Author: Cole Barach
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
 * @param row The row to print at.
 * @param column The column to print at.
 * @param bms The BMS owning the segment.
 * @param segmentIndex The index of the segment.
 */
void printSegment (int row, int column, bms_t* bms, uint16_t segmentIndex);

/**
 * @brief Prints the voltage of a cell.
 * @param row The row to print at.
 * @param column The column to print at.
 * @param bms The BMS owning the cell.
 * @param cellIndex The global index of the cell.
 */
void printVoltage (int row, int column, bms_t* bms, uint16_t cellIndex);

/**
 * @brief Prints the index of a cell.
 * @param row The row to print at.
 * @param column The column to print at.
 * @param index The global index of the cell.
 */
void printCellIndex (int row, int column, uint16_t cellIndex);

/**
 * @brief Prints the temperature of a sense line.
 * @param row The row to print at.
 * @param column The column to print at.
 * @param bms The BMS owning the sense line.
 * @param senseLineIndex The global index of the sense line.
 */
void printTemperature (int row, int column, bms_t* bms, uint16_t senseLineIndex);

/**
 * @brief Prints the status of a sense line.
 * @param row The row to print at.
 * @param column The column to print at.
 * @param bms The BMS owning the sense line.
 * @param senseLineIndex The global index of the sense line.
 */
void printSenseLineStatus (int row, int column, bms_t* bms, uint16_t senseLineIndex);

/**
 * @brief Prints the index and status of an LTC.
 * @param row The row to print at.
 * @param column The column to print at.
 * @param bms The BMS owning the LTC.
 * @param ltcIndex The global index of the LTC.
 */
void printLtcStatus (int row, int column, bms_t* bms, uint16_t ltcIndex);

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

	while (true)
	{
		uint16_t row = 0;

		#ifdef __unix__
		clear ();
		#endif

		// Print the top-most panel
		printStatPanel (row, 0, &bms);
		row += 6;

		// Print each battery segment
		for (uint16_t segmentIndex = 0; segmentIndex < bms.segmentCount; ++segmentIndex)
		{
			printSegment (row, 0, &bms, segmentIndex);
			row += 10;
		}

		// Get keyboard input
		int ret = getch ();
		if (ret == ' ')
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

		// Render the frame to the screen and wait for the next update
		refresh ();
		napms (48);
	}

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

void printSegment (int row, int column, bms_t* bms, uint16_t segmentIndex)
{
	// Column index for the cells
	uint16_t columnCell = column;

	// Column index for the sense lines
	uint16_t columnSense = column;

	// Print the start of the segment
	mvprintw (row + 0, 0, "┌");
	mvprintw (row + 1, 0, "├");
	mvprintw (row + 2, 0, "│");
	mvprintw (row + 3, 0, "├┬──");
	mvprintw (row + 4, 0, "││  ");
	mvprintw (row + 5, 0, "││  ");
	mvprintw (row + 6, 0, "│└──");
	mvprintw (row + 7, 0, "└───");
	columnSense += 1;
	columnCell += 3;

	for (uint16_t ltcIndex = 0; ltcIndex < bms->ltcsPerSegment; ++ltcIndex)
	{
		// Print the segment's cells
		for (uint16_t cellIndex = 0; cellIndex < bms->cellsPerLtc; ++cellIndex)
		{
			// Convert local cell index to global cell index.
			uint16_t index = CELL_INDEX_LOCAL_TO_GLOBAL (bms, segmentIndex, ltcIndex, cellIndex);

			// Print the cell's box
			mvprintw (row + 3, columnCell, "┬──┘└──");
			mvprintw (row + 4, columnCell, "│      ");
			mvprintw (row + 5, columnCell, "│      ");
			mvprintw (row + 6, columnCell, "┴──────");
			mvprintw (row + 7, columnCell, "───────");

			// If this is the first cell, extend the left side to connect to the start of the segement
			if (cellIndex == 0)
			{
				mvprintw (row + 3, columnCell, "─");
				mvprintw (row + 4, columnCell, " ");
				mvprintw (row + 5, columnCell, " ");
				mvprintw (row + 6, columnCell, "─");
				mvprintw (row + 7, columnCell, "─");
			}

			// Print the cell voltage text
			printVoltage (row + 4, columnCell + 3, bms, index);
			// Print the cell index text
			printCellIndex (row + 7, columnCell + 2, index);

			columnCell += 7;
		}

		if (ltcIndex != bms->ltcsPerSegment - 1)
		{
			// If this is not the last LTC, print the divider for the next one.
			mvprintw (row + 3, columnCell,	"──┬┬┬─");
			mvprintw (row + 4, columnCell,	"  │││ ");
			mvprintw (row + 5, columnCell,	"  │││ ");
			mvprintw (row + 6, columnCell,	"──┘│└─");
			mvprintw (row + 7, columnCell,	"───┴──");
			columnCell += 6;
		}
		else
		{
			// If this is the last LTC, print the end of the segment.
			mvprintw (row + 3, columnCell,	"──┬┤");
			mvprintw (row + 4, columnCell,	"  ││");
			mvprintw (row + 5, columnCell,	"  ││");
			mvprintw (row + 6, columnCell,	"──┘│");
			mvprintw (row + 7, columnCell,	"───┘");
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
				mvprintw (row + 0, columnSense, "───────");
				mvprintw (row + 1, columnSense, "─────╮╭");
				mvprintw (row + 2, columnSense, "     ├┤");
				increment = 7;
			}
			else
			{
				mvprintw (row + 0, columnSense, "─────");
				mvprintw (row + 1, columnSense, "─────");
				mvprintw (row + 2, columnSense, "     ");
				increment = 5;
			}

			// Print the LTC's status. This is done after 6 sense lines have been printed, as otherwise they would print over
			// top this text.
			if (senseLineIndex == 5)
				printLtcStatus (row, columnSense - 34, bms, LTC_INDEX_LOCAL_TO_GLOBAL (bms, segmentIndex, ltcIndex));

			// Print the sense line's temperature and status.
			printTemperature (row + 2, columnSense, bms, index);
			printSenseLineStatus (row + 3, columnSense, bms, index);
			columnSense += increment;
		}

		if (ltcIndex != bms->ltcsPerSegment - 1)
		{
			// If this is not the last LTC, print the divider for the next one.
			mvprintw (row + 0, columnSense,	"┬");
			mvprintw (row + 1, columnSense,	"┴");
			mvprintw (row + 2, columnSense,	"┊");
			columnSense += 1;
		}
		else
		{
			// If this is the last LTC, print the end of the segment.
			mvprintw (row + 0, columnSense,	"┐");
			mvprintw (row + 1, columnSense,	"┤");
			mvprintw (row + 2, columnSense,	"│");
		}
	}
}

void printVoltage (int row, int column, bms_t* bms, uint16_t cellIndex)
{
	float voltage;
	bool discharging;

	// If either measurement is invalid, print invalid.
	if (bmsGetCellVoltage (bms, cellIndex, &voltage) != CAN_DATABASE_VALID ||
		bmsGetCellDischarging (bms, cellIndex, &discharging) != CAN_DATABASE_VALID)
	{
		attron (COLOR_PAIR (COLOR_INVALID));
		mvprintw (row, column, "--");
		attroff (COLOR_PAIR (COLOR_INVALID));
		return;
	}

	// If the voltage is not nominal, print in the invalid color.
	bool voltageNominal = voltage >= bms->minCellVoltage && voltage <= bms->maxCellVoltage;
	NCURSES_PAIRS_T color = voltageNominal ? (discharging ? COLOR_BALANCING : COLOR_VALID) : COLOR_INVALID;

	attron (COLOR_PAIR (color));

	// Print the first 2 digits of the voltage
	uint16_t voltageRounded = (uint16_t) roundf (voltage * 100);
	uint8_t voltageInt = voltageRounded / 100;
	uint8_t voltageFrac = voltageRounded % 100;
	mvprintw (row, column, "%i.", voltageInt);
	mvprintw (row + 1, column, "%02i", voltageFrac);

	attroff (COLOR_PAIR (color));
}

void printCellIndex (int row, int column, uint16_t cellIndex)
{
	// Print the index of the cell
	mvprintw (row, column, " %i ", cellIndex);
}

void printTemperature (int row, int column, bms_t* bms, uint16_t senseLineIndex)
{
	float temperature;
	switch (bmsGetSenseLineTemperature (bms, senseLineIndex, &temperature))
	{
	case CAN_DATABASE_MISSING:
		// If no signal is present, print nothing.
		break;

	case CAN_DATABASE_TIMEOUT:
		// If the signal is timed out, print invalid.
		attron (COLOR_PAIR (COLOR_INVALID));
		mvprintw (row, column, " --- ");
		attroff (COLOR_PAIR (COLOR_INVALID));

		break;

	case CAN_DATABASE_VALID:
		// If the temperature is not nominal, print in the invalid color.
		bool temperatureNominal = temperature > bms->minTemperature && temperature < bms->maxTemperature;
		NCURSES_PAIRS_T color = temperatureNominal ? COLOR_VALID : COLOR_INVALID;

		// Print the temperature
		attron (COLOR_PAIR (color));
		mvprintw (row, column, "%.1fC", temperature);
		attroff (COLOR_PAIR (color));

		break;
	}
}

void printSenseLineStatus (int row, int column, bms_t* bms, uint16_t senseLineIndex)
{
	bool open;
	switch (bmsGetSenseLineOpen (bms, senseLineIndex, &open))
	{
	case CAN_DATABASE_MISSING:
	case CAN_DATABASE_TIMEOUT:
		// If the signal is not valid, print invalid.
		attron (COLOR_PAIR (COLOR_INVALID));
		mvprintw (row, column + 1, " - ");
		attroff (COLOR_PAIR (COLOR_INVALID));
		break;

	case CAN_DATABASE_VALID:
		// If the sense line is open, print so.
		if (open)
		{
			attron (COLOR_PAIR (COLOR_INVALID));
			mvprintw (row, column, " XXX ");
			attroff (COLOR_PAIR (COLOR_INVALID));
		}
	}
}

void printLtcStatus (int row, int column, bms_t* bms, uint16_t ltcIndex)
{
	switch (bmsGetLtcState (bms, ltcIndex))
	{
	case BMS_LTC_STATE_MISSING:
		// If no other information is available, print only the index.
		mvprintw (row, column, " LTC %i ", ltcIndex);
		break;

	case BMS_LTC_STATE_TIMEOUT:
		// If either signal is timed out, print so.
		attron (COLOR_PAIR (COLOR_INVALID));
		mvprintw (row, column, " LTC %i: CAN Timeout ", ltcIndex);
		attroff (COLOR_PAIR (COLOR_INVALID));
		break;

	case BMS_LTC_STATE_ISOSPI_FAULT:
		// If the LTC is faulted, print so.
		attron (COLOR_PAIR (COLOR_INVALID));
		mvprintw (row, column, " LTC %i: IsoSPI Fault ", ltcIndex);
		attroff (COLOR_PAIR (COLOR_INVALID));
		break;

	case BMS_LTC_STATE_SELF_TEST_FAULT:
		// If the LTC is faulted, print so.
		attron (COLOR_PAIR (COLOR_INVALID));
		mvprintw (row, column, " LTC %i: Self-Test Fault ", ltcIndex);
		attroff (COLOR_PAIR (COLOR_INVALID));
		break;

	case BMS_LTC_STATE_OKAY:
		// If the LTC is okay, print so.
		attron (COLOR_PAIR (COLOR_VALID));
		mvprintw (row, column, " LTC %i: Comms Okay ", ltcIndex);
		attroff (COLOR_PAIR (COLOR_VALID));
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
		attron (COLOR_PAIR (COLOR_INVALID));
		printw ("(-- C) ");
		attron (COLOR_PAIR (COLOR_INVALID));
		break;

	case CAN_DATABASE_VALID:
		// If the temperature is not nominal, print in the invalid color.
		bool temperatureNominal = temperature < bms->maxLtcTemperature;
		NCURSES_PAIRS_T color = temperatureNominal ? COLOR_VALID : COLOR_INVALID;

		// Print the temperature.
		attron (COLOR_PAIR (color));
		printw ("(%4.2f C) ", temperature);
		attroff (COLOR_PAIR (color));
	}
}