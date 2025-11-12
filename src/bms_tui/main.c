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
#include <stdlib.h>
#include <errno.h>
#include <locale.h>
#include <math.h>

// Constants ------------------------------------------------------------------------------------------------------------------

#define COLOR_INVALID		1
#define COLOR_VALID			2
#define COLOR_BALANCING		3

// The height of the BMS Stat Panel
#define CONTROL_PANEL_HEIGHT 3

// The height of each BMS Segment
#define BMS_SEGMENT_HEIGHT 9 

// Functions ------------------------------------------------------------------------------------------------------------------
 
/** 
 * @param row The row to print at.
 * @param column The column to print at.
 * @param bms The BMS to print the stats of.
 */
void printControlPanel (int row, int column, bms_t* bms);

/** 
 * @brief Prints a panel of statistics about a BMS.
 * @param scrlTop The coordinate of the top of the scrolling window.
 * @param scrlBottom The coordinate of the bottom of the scrolling window.
 * @param scrRow The row on the screen to print at.
 * @param row The row to print at.
 * @param column The column to print at.
 * @param height The height of the panel.
 * @param bms The BMS to print the stats of.
 */
void printStatPanel (int scrlTop, int scrlBottom, uint16_t scrRow, uint16_t row, uint16_t column, size_t height, bms_t* bms);

/**
 * @brief Prints all the signals on the BMS_Status message
 * @param scrlTop The coordinate of the top of the scrolling window.
 * @param scrlBottom The coordinate of the bottom of the scrolling window.
 * @param scrRow The row on the screen to print at.
 * @param row The row of the panel.
 * @param column The column to print at.
 * @param height The height of the panel.
 * @param bms The BMS to print the stats of. 
*/
void printBmsStatusSignals (int scrlTop, int scrlBottom, uint16_t* scrRow, uint16_t row, uint16_t column, size_t height, bms_t* bms);

/** 
 * @brief Prints all information known about a segment.
 * @param scrlTop The coordinate of the top of the scrolling window.
 * @param scrlBottom The coordinate of the bottom of the scrolling window.
 * @param scrRow The row on the screen to print at.
 * @param row The row of the segment.
 * @param column The column to print at.
 * @param bms The BMS owning the segment.
 * @param segmentIndex The index of the segment.
 */
int printSegment (int scrlTop, int scrlBottom, uint16_t* scrRow, uint16_t row, uint16_t column, bms_t* bms, uint16_t segmentIndex);

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

/**
 * @brief Validates a given row coordinate is between the top and bottom coordinates of a window.
 * @param startRow The row coordinate of the top of the window.
 * @param endRow The row coordinate of the bottom of the window.
 * @param row The coordinate of the row to validate.
 */
bool checkRow (int startRow, int endRow, int row);

// Entrypoint -----------------------------------------------------------------------------------------------------------------

int main (int argc, char** argv)
{
	// Debugging initialization
	debugInit ();

	// Initialize the CAN device
	canDevice_t* device;

	// Validate standard arguments
	if (argc != 4)
	{
		fprintf (stderr, "Format: bms-tui <device name> <DBC file path> <config file path>\n");
		// TODO(DiBacco): implement baud rate detection when the command doesn't specify a device
		// fprintf (stderr, "Format: bms-tui <baud rate> <DBC file path> <config file path>\n");
		return -1;
	}
	// Enumerate device if the command only specifies a baudRate
	// TODO(DiBacco): implement baud rate detection when the command doesn't specify a device
	/*
	else if (strspn (argv [1], "0123456789") == strlen (argv [1]))
	{
		char* baudRate = argv [1];
		device = enumerateDevice (baudRate);

	}
	*/
	else
	{
		char* deviceName  = argv [1];
		device = canInit (deviceName);
	}

	char* dbcPath	 = argv [2];
	char* configPath = argv [3];

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

	
	// The height of BMS_Status Signals panel
	const size_t BMS_STATUS_SIGNALS_PANEL_HEIGHT = 4 + bms.bmsStatusSignalsCount; // the header + footer + the total number of signals

	// The height of the BMS Stat panel
	const size_t BMS_STAT_FRAME_HEIGHT = 16; // # of rows in the header (4) + the # of signals (11) + footer (1)

	// The total # of rows in the window
	const size_t TOTAL_ROWS = CONTROL_PANEL_HEIGHT + (bms.segmentCount * BMS_SEGMENT_HEIGHT) + BMS_STATUS_SIGNALS_PANEL_HEIGHT + 10; // extra 10 rows after the content

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
	keypad (stdscr, true);

	// Get terminal size
	int scr_x, scr_y;
	getmaxyx (stdscr, scr_y, scr_x);

	// Specifies the first row from the content that will be displayed in the terminal 
	int offset = 0;
	while (true)
	{	
		// The row to begin displaying content at in the terminal (after the BMS Stat Panel)
		uint16_t scrRow = CONTROL_PANEL_HEIGHT; 
		
		// Refine terminal dimensions
		getmaxyx (stdscr, scr_y, scr_x);

		#ifdef __unix__
		clear ();
		#endif
		
		// Ensures the scrolling implementation does not extend past the bms content
		int overflow_y = TOTAL_ROWS - scr_y; // # of rows that will extend beyond the bottom of the terminal window
		if (overflow_y < 0) overflow_y = 0; // if the rows of content do not reach the bottom of the terminal window
		if (offset < 0) offset = 0; // denies the ability to scroll past the top of the content
		if (offset > overflow_y) offset = overflow_y; // associates the last row of the content with the last row of the terminal

		// Get the top and bottom row of the terminal window that can display bms content
		int scrlTop = offset + CONTROL_PANEL_HEIGHT; // the top row of the content in the scrolling implementation (after the bms stat panel)
		int scrlBottom = offset + scr_y -2; // the bottom row of the content in the scrolling implementation (the bottom of the terminal window)
		if (scrlTop < 0) scrlTop = 0; // ensures the scrolling implementation does not extend past the top of the content
		if (scrlTop > TOTAL_ROWS -1) scrlTop = TOTAL_ROWS; // ensures the scrolling implementation does not extend past the bottom of the content
		
		// Print the control panel
		printControlPanel (0, 0, &bms);

		uint16_t bmsStatusStartingRow = CONTROL_PANEL_HEIGHT; // the bms status signals panel is displayed after each segment
		uint16_t bmsStatStartingColumn = 64; // the bms stat panel is displaying on the right side of the tui (the middle is 64)
		
		// Get the height of the bms status signals panel and the bms stat panel 
		size_t height;
		if (BMS_STATUS_SIGNALS_PANEL_HEIGHT > BMS_STAT_FRAME_HEIGHT) height = BMS_STATUS_SIGNALS_PANEL_HEIGHT;
		else height = BMS_STAT_FRAME_HEIGHT;

		// Print the bms status signals panel & the top-most statistics panel	
		printStatPanel (scrlTop, scrlBottom, scrRow, bmsStatusStartingRow, bmsStatStartingColumn, height, &bms);
		printBmsStatusSignals (scrlTop, scrlBottom, &scrRow, bmsStatusStartingRow, 0, height, &bms);
		
		// Print each battery segment
		for (uint16_t segmentIndex = 0; segmentIndex < bms.segmentCount; ++segmentIndex)
		{	// Get the actual row from the segment content
			uint16_t segmentStartingRow = CONTROL_PANEL_HEIGHT + height + (BMS_SEGMENT_HEIGHT * segmentIndex); // The segments are displayed after the bms status signals panel
			printSegment (scrlTop, scrlBottom, &scrRow, segmentStartingRow, 0, &bms, segmentIndex);
		}

		// Render the frame to the screen and wait for the next update
		refresh ();

		// Get keyboard input
		int ret = getch ();
		if (ret == KEY_UP) 
		{
			// Move offset down
			offset = (offset - 1 >= 0) ? offset - 1 : 0;
			clear ();
			
		}
		else if (ret == KEY_DOWN) 
		{ 
			// Move offset up
			offset = (offset + 1 <= overflow_y) ? offset + 1 : overflow_y;
			clear ();

		}
		else if (ret == ' ')
		{
			// Spaces should refresh the terminal
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

	endwin ();

	return 0;
}

// Functions ------------------------------------------------------------------------------------------------------------------

void printControlPanel (int row, int column, bms_t* bms)
{
	size_t const CTRL_PANEL_WIDTH = 112;

	// Print instructions
	char* instruction1 = "Space - Refresh the Terminal";
	char* instruction2 = "↑/↓ - Scroll";
	char* instruction3 = "Q - Quit";
	
	// Calculate gap between instructions
	int gap = (CTRL_PANEL_WIDTH - 
		strlen(instruction1) - 
		strlen(instruction2) - 
		strlen(instruction3)) / 4;
	
	// Display instructions
	mvprintw(row + 1, column + gap, "%s", instruction1);
	mvprintw(row + 1, column + gap * 2 + strlen(instruction1), "%s", instruction2);
	mvprintw(row + 1, column + gap * 3 + strlen(instruction1) + strlen(instruction2), "%s", instruction3);	

	// Print the start of the panel
	mvprintw (row + 0, column, "┌─");
	mvprintw (row + 1, column, "│");
	mvprintw (row + 2, column, "└─");
	column += 2;

	// Print the top and bottom
	mvprintw (row + 0, column, "────────────────────");
	mvprintw (row + 2, column, "────────────────────");

	// Print the title of the control panel
	mvprintw (row + 0, column + 1, "Control Panel");
	
	// Next column
	column += 20;	

	// Print the top and bottom
	mvprintw (row + 0, column, "────────────────────");
	mvprintw (row + 2, column, "────────────────────");

	// Next column
	column += 20;

	// Print the top and bottom
	mvprintw (row + 0, column, "────────────────────");
	mvprintw (row + 2, column, "────────────────────");

	// Next column
	column += 20;

	// Print the top and bottom
	mvprintw (row + 0, column, "────────────────────");
	mvprintw (row + 2, column, "────────────────────");

	// Next column
	column += 20;

	// Print the top and bottom
	mvprintw (row + 0, column, "────────────────────");
	mvprintw (row + 2, column, "────────────────────");

	// Next column
	column += 20;

	// Print the top and bottom
	mvprintw (row + 0, column, "────────────────────");
	mvprintw (row + 2, column, "────────────────────");

	// Next column
	column += 20;

	// Print the end of the panel
	mvprintw (row + 0, column, "─┐");
	mvprintw (row + 1, column, " │");
	mvprintw (row + 2, column, "─┘");
}

void printStatPanel (int scrlTop, int scrlBottom, uint16_t scrRow, uint16_t row, uint16_t column, size_t height, bms_t* bms)
{
	// Validate the rows in the BMS Stat Panel
	bool validRows [height + 1];

	// Map each row of the segment to its position in the terminal window
	int mapRowToPosition [height + 1];

	// Validate & map position for each row of the bms stat panel  
	for (int i = 0; i < height + 1; i++) {
		if (checkRow(scrlTop, scrlBottom, row++)) { 
			validRows[i] = true;
			mapRowToPosition[i] = scrRow++; 
		}
		else {
			validRows[i] = false;
		} 
	}

	// Get the stats to print
	float cellMin, cellMax, cellAvg;
	bool cellsValid = bmsGetCellVoltageStats (bms, &cellMin, &cellMax, &cellAvg);

	float deltaMax, deltaAvg;
	bool deltasValid = bmsGetCellDeltaStats (bms, &deltaMax, &deltaAvg);

	float tempMin, tempMax, tempAvg;
	bool tempsValid = bmsGetTemperatureStats (bms, &tempMin, &tempMax, &tempAvg);
	
	// Print header
	for (int i = 0; i < 13; i++) {
		if (validRows[0]) mvprintw (mapRowToPosition[0], column + (i * 5), "─────");
		if (validRows[1]) mvprintw (mapRowToPosition[1], column + (i * 5), "─────");
		if (validRows[3]) mvprintw (mapRowToPosition[3], column + (i * 5), "─────");
	}

	// Print the title of the BMS Stat Panel
	if (validRows[0]) mvprintw (mapRowToPosition[0], column + 2, " BMS Stats ");

	// Print headers 
	if (validRows[2]) {
		mvprintw (mapRowToPosition[2], column + 2,  "Stat");
		mvprintw (mapRowToPosition[2], column + 42, "Value");
	}

	// Print each (valid) side of the BMS Stat Panel 
	for (int i = 1; i < height; i++) { 
		if (validRows[i]) mvprintw (mapRowToPosition[i], column + 0,  "│");
		if (validRows[i]) mvprintw (mapRowToPosition[i], column + 39, "│");
		if (validRows[i]) mvprintw (mapRowToPosition[i], column + 40, "│");
		if (validRows[i]) mvprintw (mapRowToPosition[i], column + 64, "│");
	}

	// Fill in gaps in the frame of the BMS Stat Panel
	if (validRows[0]) mvprintw (mapRowToPosition[0], column + 64,  "┐");

	if (validRows[1]) mvprintw (mapRowToPosition[1], column + 39, "╮");
	if (validRows[1]) mvprintw (mapRowToPosition[1], column + 40, "╭");

	if (validRows[1]) mvprintw (mapRowToPosition[1], column + 0,  "─");
	if (validRows[1]) mvprintw (mapRowToPosition[1], column + 64, "┤");

	if (validRows[2]) mvprintw (mapRowToPosition[2], column + 0,  "┊");
	
	if (validRows[3]) mvprintw (mapRowToPosition[3], column + 0,  "┬");
	if (validRows[3]) mvprintw (mapRowToPosition[3], column + 40, "├");

	if (validRows[3]) mvprintw (mapRowToPosition[3], column + 39, "┤");
	if (validRows[3]) mvprintw (mapRowToPosition[3], column + 64, "┤");

	// Print the pack voltage
	float packVoltage;
	bool packVoltageValid = bmsGetPackVoltage (bms, &packVoltage) == CAN_DATABASE_VALID;
	if (validRows[4]) {
		if (packVoltageValid)
		{
			//TODO(Barach): Round consistently?
			mvprintw (mapRowToPosition[4], column + 2,  "Voltage");
			mvprintw (mapRowToPosition[4], column + 42, "%.2f V", packVoltage);
		}
		else
		{
			attron (COLOR_PAIR (COLOR_INVALID));
			mvprintw (mapRowToPosition[4], column + 2,  "Voltage");
			mvprintw (mapRowToPosition[4], column + 42, "-- V");
			attroff (COLOR_PAIR (COLOR_INVALID));
		}
	}
	
	// Print the pack current
	float packCurrent;
	bool packCurrentValid = bmsGetPackCurrent (bms, &packCurrent) == CAN_DATABASE_VALID;
	if (validRows[5]) {
		if (packCurrentValid)
		{
			mvprintw (mapRowToPosition[5], column + 2,  "Current");
			mvprintw (mapRowToPosition[5], column + 42, "%.2f A", packCurrent);
		}
		else
		{
			attron (COLOR_PAIR (COLOR_INVALID));
			mvprintw (mapRowToPosition[5], column + 2,  "Current");
			mvprintw (mapRowToPosition[5], column + 42, "-- A");
			attroff (COLOR_PAIR (COLOR_INVALID));
		}
	}

	// Print the power consumption
	if (validRows[6]) {
		if (packVoltageValid && packCurrentValid)
		{
			mvprintw (mapRowToPosition[6], column + 2,  "Power");
			mvprintw (mapRowToPosition[6], column + 42, "%.2f W", packVoltage * packCurrent);
		}
		else
		{
			attron (COLOR_PAIR (COLOR_INVALID));
			mvprintw (mapRowToPosition[6], column + 2,  "Power");
			mvprintw (mapRowToPosition[6], column + 42, "-- W");
			attroff (COLOR_PAIR (COLOR_INVALID));
		}
	}

	// Print the average cell
	if (validRows[7]) {
		if (cellsValid)
		{
			mvprintw (mapRowToPosition[7], column + 2,  "Avg Cell");
			mvprintw (mapRowToPosition[7], column + 42, "%.2f V", cellAvg);
		}
		else
		{
			attron (COLOR_PAIR (COLOR_INVALID));
			mvprintw (mapRowToPosition[7], column + 2,  "Avg Cell");
			mvprintw (mapRowToPosition[7], column + 42, "-- V");
			attroff (COLOR_PAIR (COLOR_INVALID));
		}
	}

	// Print the max cell
	if (validRows[8]) {
		if (cellsValid)
		{
			mvprintw (mapRowToPosition[8], column + 2,  "Max Cell");
			mvprintw (mapRowToPosition[8], column + 42, "%.2f V", cellMax);
		}
		else
		{
			attron (COLOR_PAIR (COLOR_INVALID));
			mvprintw (mapRowToPosition[8], column + 2,  "Max Cell");
			mvprintw (mapRowToPosition[8], column + 42, "-- V");
			attroff (COLOR_PAIR (COLOR_INVALID));
		}
	}
	

	// Print the min cell
	if (validRows[9]) {
		if (cellsValid)
		{
			mvprintw (mapRowToPosition[9], column + 2,  "Min Cell");
			mvprintw (mapRowToPosition[9], column + 42, "%.2f V", cellMin);
		}
		else
		{
			attron (COLOR_PAIR (COLOR_INVALID));
			mvprintw (mapRowToPosition[9], column + 2,  "Min Cell");
			mvprintw (mapRowToPosition[9], column + 42, "-- V");
			attroff (COLOR_PAIR (COLOR_INVALID));
		}
	}

	// Print the average temp
	if (validRows[10]) { 
		if (tempsValid)
		{
			mvprintw (mapRowToPosition[10], column + 2,  "Avg Temp");
			mvprintw (mapRowToPosition[10], column + 42, "%.2f C", tempAvg);
		}
		else
		{
			attron (COLOR_PAIR (COLOR_INVALID));
			mvprintw (mapRowToPosition[10], column + 2,  "Avg Temp");
			mvprintw (mapRowToPosition[10], column + 42, "-- C");
			attroff (COLOR_PAIR (COLOR_INVALID));
		}
	}

	// Print the max temp
	if (validRows[11]) {
		if (tempsValid)
		{
			mvprintw (mapRowToPosition[11], column + 2,  "Max Temp");
			mvprintw (mapRowToPosition[11], column + 42, "%.2f C", tempMax);
		}
		else
		{
			attron (COLOR_PAIR (COLOR_INVALID));
			mvprintw (mapRowToPosition[11], column + 2,  "Max Temp");
			mvprintw (mapRowToPosition[11], column + 42, "-- C");
			attroff (COLOR_PAIR (COLOR_INVALID));
		}
	}

	// Print the min temp
	if (validRows[12]) {
		if (tempsValid)
		{
			mvprintw (mapRowToPosition[12], column + 2,  "Min Temp");
			mvprintw (mapRowToPosition[12], column + 42, "%.2f C", tempMin);
		}
		else
		{
			attron (COLOR_PAIR (COLOR_INVALID));
			mvprintw (mapRowToPosition[12], column + 2,  "Min Temp");
			mvprintw (mapRowToPosition[12], column + 42, "-- C");
			attroff (COLOR_PAIR (COLOR_INVALID));
		}
	}

	// Print the average delta
	if (validRows[13]) {
		if (deltasValid)
		{
			mvprintw (mapRowToPosition[13], column + 2,  "Avg Delta");
			mvprintw (mapRowToPosition[13], column + 42, "%.2f V", deltaAvg);
		}
		else
		{
			attron (COLOR_PAIR (COLOR_INVALID));
			mvprintw (mapRowToPosition[13], column + 2,  "Avg Delta");
			mvprintw (mapRowToPosition[13], column + 42, "-- V");
			attroff (COLOR_PAIR (COLOR_INVALID));
		}
	}

	// Print the max delta
	if (validRows[14]) {
		if (deltasValid)
		{
			mvprintw (mapRowToPosition[14], column + 2,  "Max Delta");
			mvprintw (mapRowToPosition[14], column + 42, "%.2f V", deltaMax);
		}
		else
		{
			attron (COLOR_PAIR (COLOR_INVALID));
			mvprintw (mapRowToPosition[14], column + 2,  "Max Delta");
			mvprintw (mapRowToPosition[14], column + 42, "-- V");
			attroff (COLOR_PAIR (COLOR_INVALID));
		}
	}

	// Print bottom of the stat panel
	if (validRows[height]) {
		for (int i = 0; i < 13; i++) {
			mvprintw (mapRowToPosition[height], column + (i * 5), "─────"); 
		}

		mvprintw (mapRowToPosition[height], column + 0,  "┴");
		mvprintw (mapRowToPosition[height], column + 64, "┘");

		// Fill in gaps in the frame of the BMS Stat Panel
		mvprintw (mapRowToPosition[height], column + 39, "┴");
		mvprintw (mapRowToPosition[height], column + 40, "┴");
	}
}

void printBmsStatusSignals (int scrlTop, int scrlBottom, uint16_t* scrRow, uint16_t row, uint16_t column, size_t height, bms_t* bms) 
{	
	// Get the database associated with the bms instance
	canDatabase_t* database = bms->database;

	// Validate the rows of the BMS Status Signals Panel 
	bool validRows [height + 1];

	// Map each row of the segment to its position in the terminal window
	int mapRowToPosition [height + 1];

	// Validate & map position for each row of the bms status signals panel  
	for (int i = 0; i < height + 1; i++) {
		if (checkRow(scrlTop, scrlBottom, row++)) { 
			validRows[i] = true;
			mapRowToPosition[i] = (*scrRow)++; 
		}
		else {
			validRows[i] = false;
		} 
	}

	// Print the header of the BMS Status Signals Panel
	for (int i = 0; i < 13; i++) { // (prints to the middle of the screen)
		if (validRows[0]) mvprintw (mapRowToPosition[0], column + (i * 5), "─────");
		if (validRows[1]) mvprintw (mapRowToPosition[1], column + (i * 5), "─────");
		if (validRows[3]) mvprintw (mapRowToPosition[3], column + (i * 5), "─────");
	}

	// Print the title of the BMS Status Signals Panel
	if (validRows[0]) mvprintw (mapRowToPosition[0], column + 2, " BMS Status ");
		
	// Prints corners of the BMS Status Signals Panel header
	if (validRows[1]) {
		mvprintw (mapRowToPosition[1], column + 0,  "┌");
	}
	
	// Print headers 
	if (validRows[2]) {
		mvprintw (mapRowToPosition[2], column + 2,  "Signal");
		mvprintw (mapRowToPosition[2], column + 42, "Value");
	}
	
	// Print every (valid) side of the BMS Status Signals Panel
	for (int i = 1; i < height; i++) {
		if (validRows[i]) mvprintw (mapRowToPosition[i], column + 0,  "│");
		if (validRows[i]) mvprintw (mapRowToPosition[i], column + 39, "│");
		if (validRows[i]) mvprintw (mapRowToPosition[i], column + 40, "│");
		if (validRows[i]) mvprintw (mapRowToPosition[i], column + 64, "│");
	}

	// Fill in gaps in the frame of the BMS Status Signals Panel
	if (validRows[0]) mvprintw (mapRowToPosition[0], column + 0,  "┌");
	if (validRows[0]) mvprintw (mapRowToPosition[0], column + 64, "─");

	if (validRows[1]) mvprintw (mapRowToPosition[1], column + 39, "╮");
	if (validRows[1]) mvprintw (mapRowToPosition[1], column + 40, "╭");

	if (validRows[1]) mvprintw (mapRowToPosition[1], column + 0,  "├");
	if (validRows[1]) mvprintw (mapRowToPosition[1], column + 64, "─");

	if (validRows[2]) mvprintw (mapRowToPosition[2], column + 64, "┊");
	
	if (validRows[3]) mvprintw (mapRowToPosition[3], column + 0,  "├");
	if (validRows[3]) mvprintw (mapRowToPosition[3], column + 40, "├");

	if (validRows[3]) mvprintw (mapRowToPosition[3], column + 39, "┤");
	if (validRows[3]) mvprintw (mapRowToPosition[3], column + 64, "┬");

	// Display each signal name and its corresponding value in a single row
	for (size_t signalIndex = 0; signalIndex < bms->bmsStatusSignalsCount; signalIndex++) { 
		float value;
		
		// Set the row of the current signal
		row = signalIndex + 4;  
		if (validRows[row]) {

			// TODO(DiBacco): the code below will create a merge conflict
			// Get the signal and the local index 
			ssize_t bmsStatusSignalIndex = bms->bmsStatusSignalIndices[signalIndex];
			canSignal_t* bmsStatusSignal = bms->bmsStatusSignals[signalIndex];

			// Get & display signal value or insert placeholder value
			switch (bmsGetSignalValue(database, bms->bmsStatusMessageIndex, bmsStatusSignalIndex, &value)) 
			{
				case CAN_DATABASE_MISSING: {
					// Shouldn't reach here
					break;
				}
				case CAN_DATABASE_TIMEOUT: {
					attron (COLOR_PAIR (COLOR_INVALID));
					mvprintw (mapRowToPosition[row], column + 2, bmsStatusSignal->name);
					mvprintw (mapRowToPosition[row], column + 42, "---");
					mvprintw (mapRowToPosition[row], column + 50, "%s", bmsStatusSignal->unit);
					attroff (COLOR_PAIR (COLOR_INVALID));
					break;
				}
				case CAN_DATABASE_VALID: {
					mvprintw (mapRowToPosition[row], column + 2, bmsStatusSignal->name);
					mvprintw (mapRowToPosition[row], column + 42, "%.01f", value);
					mvprintw (mapRowToPosition[row], column + 50, "%s", bmsStatusSignal->unit);
					break;
				}
			}
		}
	}

	// Print the bottom of the BMS Status Signals Panel
	if (validRows[height]) {
		for (int i = 0; i < 13; i++) 
			mvprintw (mapRowToPosition[height], column + (i * 5), "─────");

		mvprintw (mapRowToPosition[height], column + 0,  "└");
		mvprintw (mapRowToPosition[height], column + 64, "┴");

		// Fill in gaps in the frame of the BMS Status Signals Panel
		mvprintw (mapRowToPosition[height], column + 39, "┴");
		mvprintw (mapRowToPosition[height], column + 40, "┴");
	} 
}

int printSegment (int scrlTop, int scrlBottom, uint16_t* scrRow, uint16_t row, uint16_t column, bms_t* bms, uint16_t segmentIndex)
{	
	// Column index for the cells
	uint16_t columnCell = column;

	// Column index for the sense lines
	uint16_t columnSense = column;

	// Get valid rows (rows that can be displayed)
	bool validSegment = false;
	bool validRows [BMS_SEGMENT_HEIGHT];

	// Map each row of the segment to its position in the window
	int mapRowToPosition [BMS_SEGMENT_HEIGHT];

	// Validate & map position for each row of the segment 
	for (int i = 0; i < BMS_SEGMENT_HEIGHT; i++) {
		if (checkRow(scrlTop, scrlBottom, row + i)) {
			validRows[i] = true;
			validSegment = true;
			mapRowToPosition[i] = *scrRow;
			*scrRow += 1;
			
		}
		else {
			validRows[i] = false;
		} 
	}

	// Check if the entire segment cannot be displayed
	if (!validSegment) return 0;

	// Print the start of the segment
	if (validRows[0]) mvprintw (mapRowToPosition[0], 0, "┌"); 
	if (validRows[1]) mvprintw (mapRowToPosition[1], 0, "├");  
	if (validRows[2]) mvprintw (mapRowToPosition[2], 0, "│"); 
	if (validRows[3]) mvprintw (mapRowToPosition[3], 0, "│"); 
	if (validRows[4]) mvprintw (mapRowToPosition[4], 0, "├┬"); 
	if (validRows[5]) mvprintw (mapRowToPosition[5], 0, "││"); 
	if (validRows[6]) mvprintw (mapRowToPosition[6], 0, "││"); 
	if (validRows[7]) mvprintw (mapRowToPosition[7], 0, "│└"); 
	if (validRows[8]) mvprintw (mapRowToPosition[8], 0, "└─"); 
		
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
			if (validRows[4]) mvprintw (mapRowToPosition[4], columnCell, "┬─┘└─");
			if (validRows[5]) mvprintw (mapRowToPosition[5], columnCell, "│    ");
			if (validRows[6]) mvprintw (mapRowToPosition[6], columnCell, "│    ");
			if (validRows[7]) mvprintw (mapRowToPosition[7], columnCell, "┴────");
			if (validRows[8]) mvprintw (mapRowToPosition[8], columnCell, "─────");

			// If this is the first cell, extend the left side to connect to the start of the segement
			if (cellIndex == 0)
			{
				if (validRows[4]) mvprintw (mapRowToPosition[4], columnCell, "─");
				if (validRows[5]) mvprintw (mapRowToPosition[5], columnCell, " ");
				if (validRows[6]) mvprintw (mapRowToPosition[6], columnCell, " ");
				if (validRows[7]) mvprintw (mapRowToPosition[7], columnCell, "─");
				if (validRows[8]) mvprintw (mapRowToPosition[8], columnCell, "─");
			}

			// Print the cell voltage text
			if (validRows[5]) printVoltage (mapRowToPosition[5], columnCell + 2, bms, index);
			// Print the cell index text
			if (validRows[8]) printCellIndex (mapRowToPosition[8], columnCell + 2, index); 
	
			columnCell += 5;
		}

		if (ltcIndex != bms->ltcsPerSegment - 1)
		{
			// If this is not the last LTC, print the divider for the next one.
			if (validRows[4]) mvprintw (mapRowToPosition[4], columnCell,	"─┬┬┬"); 
			if (validRows[5]) mvprintw (mapRowToPosition[5], columnCell,	" │││"); 
			if (validRows[6]) mvprintw (mapRowToPosition[6], columnCell,	" │││"); 
			if (validRows[7]) mvprintw (mapRowToPosition[7], columnCell,	"─┘│└"); 
			if (validRows[8]) mvprintw (mapRowToPosition[8], columnCell,	"──┴─"); 

			columnCell += 4;
		}
		else
		{
			// If this is the last LTC, print the end of the segment.
			if (validRows[4]) mvprintw (mapRowToPosition[4], columnCell,	"─┬┤"); 
			if (validRows[5]) mvprintw (mapRowToPosition[5], columnCell,	" ││"); 
			if (validRows[6]) mvprintw (mapRowToPosition[6], columnCell,	" ││"); 
			if (validRows[7]) mvprintw (mapRowToPosition[7], columnCell,	"─┘│"); 
			if (validRows[8]) mvprintw (mapRowToPosition[8], columnCell,	"──┘"); 
		}

		// Print the segment's sense lines
		for (uint16_t senseLineIndex = 0; senseLineIndex < bms->senseLinesPerLtc; ++senseLineIndex)
		{	
			// Convert local sense line index to global sense line index.
			uint16_t index = SENSE_LINE_INDEX_LOCAL_TO_GLOBAL (bms, segmentIndex, ltcIndex, senseLineIndex);

			// Print the sense line's box (or tab).
			uint16_t increment;

			int temperatureRowOffset = 0;
			if (senseLineIndex != bms->senseLinesPerLtc - 1)
			{
				if (validRows[0]) mvprintw (mapRowToPosition[0], columnSense, "─────"); 
				if (validRows[1]) mvprintw (mapRowToPosition[1], columnSense, "───╮╭"); 
				if (validRows[2]) mvprintw (mapRowToPosition[2], columnSense, "   ├┤");
				if (validRows[3]) mvprintw (mapRowToPosition[3], columnSense, "   ├┤"); 
				increment = 5;
			}
			else
			{
				if (validRows[0]) mvprintw (mapRowToPosition[0], columnSense, "───"); 
				if (validRows[1]) mvprintw (mapRowToPosition[1], columnSense, "───"); 
				if (validRows[2]) mvprintw (mapRowToPosition[2], columnSense, "   ");
				if (validRows[3]) mvprintw (mapRowToPosition[3], columnSense, "   "); 
				increment = 3;
			}

			// Print the LTC's status. This is done after 6 sense lines have been printed, as otherwise they would print over
			// top this text.
			if (senseLineIndex == 5)
				if (validRows[0]) printLtcStatus (mapRowToPosition[0], columnSense - 24, bms, LTC_INDEX_LOCAL_TO_GLOBAL (bms, segmentIndex, ltcIndex)); 

			// Print the sense line's temperature and status.
			if (validRows[2]) printTemperature (mapRowToPosition[2], columnSense, bms, index);
			if (validRows[4]) printSenseLineStatus (mapRowToPosition[4], columnSense, bms, index); 
			columnSense += increment;
		}

		if (ltcIndex != bms->ltcsPerSegment - 1)
		{
			// If this is not the last LTC, print the divider for the next one.
			if (validRows[0]) mvprintw (mapRowToPosition[0], columnSense,	"┬");
			if (validRows[1]) mvprintw (mapRowToPosition[1], columnSense,	"┴"); 
			if (validRows[2]) mvprintw (mapRowToPosition[2], columnSense,	"┊");
			if (validRows[3]) mvprintw (mapRowToPosition[3], columnSense,	"┊"); 
			columnSense += 1;
		}
		else
		{
			// If this is the last LTC, print the end of the segment.
			if (validRows[0]) mvprintw (mapRowToPosition[0], columnSense,	"┐");
			if (validRows[1]) mvprintw (mapRowToPosition[1], columnSense,	"┤"); 
			if (validRows[2]) mvprintw (mapRowToPosition[2], columnSense,	"│");
			if (validRows[3]) mvprintw (mapRowToPosition[3], columnSense,	"│"); 
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
	mvprintw (row, column, "%i", cellIndex);
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
		mvprintw (row, column, " - ");
		attroff (COLOR_PAIR (COLOR_INVALID));

		break;

	case CAN_DATABASE_VALID:
		// If the temperature is not nominal, print in the invalid color.
		bool temperatureNominal = temperature > bms->minTemperature && temperature < bms->maxTemperature;
		NCURSES_PAIRS_T color = temperatureNominal ? COLOR_VALID : COLOR_INVALID;

		// Print the temperature
		attron (COLOR_PAIR (color));

		// Display the whole number and decimal of the temperature on seperate lines at allow it to fit in a cell
		mvprintw(row, column, "%d.", (int) temperature);      
		mvprintw(row + 1, column, "%01dC", (int) roundf( fabsf ((int) temperature - (float) temperature) * 10.0f));   
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
		mvprintw (row, column, " - ");
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
		attroff (COLOR_PAIR (COLOR_INVALID));
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

bool checkRow (int startRow, int endRow, int row) {
	if (startRow <= row && row <= endRow) 
		return true;
	return false;
}