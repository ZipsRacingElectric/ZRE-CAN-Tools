// BMS TUI --------------------------------------------------------------------------------------------------------------------
//
// Author: Cole Barach
// Date Created: 2025.05.24
//
// Description: Terminal user interface for monitoring a battery management system.

// Includes -------------------------------------------------------------------------------------------------------------------

// Includes
#include "bms/bms.h"
#include "can_device/can_device.h"
#include "cjson/cjson_util.h"
#include "error_codes.h"

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

void printPowerStats (int row, int column, bms_t* bms)
{
	float cellMin, cellMax, cellAvg;
	bool cellsValid = bmsGetCellVoltageStats(bms, &cellMin, &cellMax, &cellAvg);

	float deltaMax, deltaAvg;
	bool deltasValid = bmsGetCellDeltaStats (bms, &deltaMax, &deltaAvg);

	float tempMin, tempMax, tempAvg;
	bool tempsValid = bmsGetTemperatureStats (bms, &tempMin, &tempMax, &tempAvg);

	mvprintw (row + 0, column, "┌─");
	mvprintw (row + 1, column, "│");
	mvprintw (row + 2, column, "│");
	mvprintw (row + 3, column, "│");
	mvprintw (row + 4, column, "└─");
	column += 2;

	mvprintw (row + 0, column, "────────────────────");

	mvprintw (row + 4, column, "────────────────────");
	if (!*bms->packVoltageValid)
	{
		attron (COLOR_PAIR (COLOR_INVALID));
		mvprintw (row + 2, column, "Voltage: -- V");
		attroff (COLOR_PAIR (COLOR_INVALID));
	}
	else
	{
		mvprintw (row + 2, column, "Voltage: %-.2f V", *bms->packVoltage);
	}

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

	column += 20;

	mvprintw (row + 0, column, "────────────────────");

	mvprintw (row + 4, column, "────────────────────");
	if (!*bms->packCurrentValid)
	{
		attron (COLOR_PAIR (COLOR_INVALID));
		mvprintw (row + 2, column, "Current: -- A");
		attroff (COLOR_PAIR (COLOR_INVALID));
	}
	else
	{
		mvprintw (row + 2, column, "Current: %-.2f A", *bms->packCurrent);
	}

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
	column += 20;

	mvprintw (row + 0, column, "────────────────────");

	mvprintw (row + 4, column, "────────────────────");

	if (!*bms->packVoltageValid || !*bms->packCurrentValid)
	{
		attron (COLOR_PAIR (COLOR_INVALID));
		mvprintw (row + 2, column, "Power: -- W");
		attroff (COLOR_PAIR (COLOR_INVALID));
	}
	else
	{
		mvprintw (row + 2, column, "Power: %-.2f W", *bms->packVoltage * *bms->packCurrent);
	}

	if (cellsValid)
	{
		mvprintw (row + 3, column, "Avg Cell: %-.2f V", cellMax);
	}
	else
	{
		attron (COLOR_PAIR (COLOR_INVALID));
		mvprintw (row + 3, column, "Avg Cell: -- V");
		attroff (COLOR_PAIR (COLOR_INVALID));
	}

	column += 20;

	mvprintw (row + 0, column, "────────────────────");
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

	mvprintw (row + 4, column, "────────────────────");
	column += 20;

	mvprintw (row + 0, column, "────────────────────");
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

	mvprintw (row + 4, column, "────────────────────");
	column += 20;

	mvprintw (row + 0, column, "────────────────────");

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

	mvprintw (row + 4, column, "────────────────────");
	column += 20;

	mvprintw (row + 0, column, "─┐");
	mvprintw (row + 1, column, " │");
	mvprintw (row + 2, column, " │");
	mvprintw (row + 3, column, " │");
	mvprintw (row + 4, column, "─┘");
}

void printVoltage (int row, int column, bms_t* bms, uint16_t index)
{
	if (!*bms->cellVoltagesValid [index] || !*bms->cellsDischargingValid [index])
	{
		attron (COLOR_PAIR (COLOR_INVALID));
		mvprintw (row, column, "--");
		attroff (COLOR_PAIR (COLOR_INVALID));
		return;
	}

	float voltage = *bms->cellVoltages [index];
	bool balancing = *bms->cellsDischarging [index];
	bool voltageNominal = voltage >= bms->minCellVoltage && voltage <= bms->maxCellVoltage;

	NCURSES_PAIRS_T color = voltageNominal ? (balancing ? COLOR_BALANCING : COLOR_VALID) : COLOR_INVALID;

	attron (COLOR_PAIR (color));
	uint8_t voltageInt = (uint8_t) voltage;
	uint8_t voltageFrac = (uint8_t) roundf ((voltage - voltageInt) * 100);
	mvprintw (row, column, "%i.", voltageInt);
	mvprintw (row + 1, column, "%02i", voltageFrac);
	attroff (COLOR_PAIR (color));
}

void printTemperature (int row, int column, bms_t* bms, uint16_t index)
{
	if (bms->senseLineTemperatures [index] == NULL || bms->senseLineTemperaturesValid [index] == NULL)
		return;

	if (!*bms->senseLineTemperaturesValid [index])
	{
		attron (COLOR_PAIR (COLOR_INVALID));
		mvprintw (row, column, " --- ");
		attroff (COLOR_PAIR (COLOR_INVALID));
		return;
	}

	float temperature = *bms->senseLineTemperatures [index];
	bool temperatureNominal = temperature > bms->minTemperature && temperature < bms->maxTemperature;

	NCURSES_PAIRS_T color = temperatureNominal ? COLOR_VALID : COLOR_INVALID;

	attron (COLOR_PAIR (color));
	mvprintw (row, column, "%.1fC", temperature);
	attroff (COLOR_PAIR (color));
}

void printSenseLineStatus (int row, int column, bms_t* bms, uint16_t index)
{
	if (!*bms->senseLinesOpenValid [index])
	{
		attron (COLOR_PAIR (COLOR_INVALID));
		mvprintw (row, column + 1, " - ");
		attroff (COLOR_PAIR (COLOR_INVALID));
		return;
	}

	if (*bms->senseLinesOpen [index])
	{
		attron (COLOR_PAIR (COLOR_INVALID));
		mvprintw (row, column, " XXX ");
		attroff (COLOR_PAIR (COLOR_INVALID));
	}
}

void printLtcIndex (int row, int column, bms_t* bms, uint16_t index)
{
	if (bms->ltcIsoSpiFaults [index] == NULL)
	{
		mvprintw (row, column, "LTC %i", index);
	}
	else if (!*bms->ltcIsoSpiFaultsValid [index] || !*bms->ltcSelfTestFaultsValid [index])
	{
		attron (COLOR_PAIR (COLOR_INVALID));
		mvprintw (row, column, " LTC %i: CAN Timeout ", index);
		attroff (COLOR_PAIR (COLOR_INVALID));
	}
	else
	{
		if (*bms->ltcIsoSpiFaults [index])
		{
			attron (COLOR_PAIR (COLOR_INVALID));
			mvprintw (row, column, " LTC %i: IsoSPI Fault ", index);
			attroff (COLOR_PAIR (COLOR_INVALID));
		}
		else if (*bms->ltcSelfTestFaults [index])
		{
			attron (COLOR_PAIR (COLOR_INVALID));
			mvprintw (row, column, " LTC %i: Self-Test Fault ", index);
			attroff (COLOR_PAIR (COLOR_INVALID));
		}
		else
		{
			attron (COLOR_PAIR (COLOR_VALID));
			mvprintw (row, column, " LTC %i: Comms Okay ", index);
			attroff (COLOR_PAIR (COLOR_VALID));
		}
	}

	if (bms->ltcTemperatures [index] != NULL)
	{
		if (!*bms->ltcTemperaturesValid [index])
		{
			attron (COLOR_PAIR (COLOR_INVALID));
			printw ("(-- C) ");
			attron (COLOR_PAIR (COLOR_INVALID));
		}
		else
		{
			bool temperatureNominal = *bms->ltcTemperatures [index] < bms->maxLtcTemperature;
			NCURSES_PAIRS_T color = temperatureNominal ? COLOR_VALID : COLOR_INVALID;
			attron (COLOR_PAIR (color));
			printw ("(%4.2f C) ", *bms->ltcTemperatures [index]);
			attroff (COLOR_PAIR (color));
		}
	}
}

void printCellIndex (int row, int column, uint16_t index)
{
	mvprintw (row, column, " %i ", index);
}

// Entrypoint -----------------------------------------------------------------------------------------------------------------

int main (int argc, char** argv)
{
	if (argc != 4)
	{
		fprintf (stderr, "Format: bms-tui <device name> <DBC file path> <config file path>\n");
		return -1;
	}

	char* deviceName = argv [1];
	char* dbcPath = argv [2];
	char* configPath = argv [3];

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

	// Load the config file.
	cJSON* config = jsonLoad (configPath);
	if (config == NULL)
	{
		int code = errno;
		fprintf (stderr, "Failed to load config JSON file: %s.\n", errorMessage (code));
		return code;
	}

	// Initialize the BMS.
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

	initscr ();
	if(has_colors() == FALSE)
	{
		endwin ();
		fprintf (stderr, "Terminal environment does not support colors.\n");
		return -1;
	}

	start_color ();
	init_pair (COLOR_INVALID,	COLOR_RED,		COLOR_BLACK);
	init_pair (COLOR_VALID,		COLOR_GREEN,	COLOR_BLACK);
	init_pair (COLOR_BALANCING,	COLOR_CYAN,		COLOR_BLACK);

	while (true)
	{
		uint16_t row = 0;
		uint16_t columnCell = 0;
		uint16_t columnSense = 0;

		clear ();

		printPowerStats (row, 0, &bms);
		row += 6;

		for (uint16_t segmentIndex = 0; segmentIndex < bms.segmentCount; ++segmentIndex)
		{
			// Start of segment
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

			for (uint16_t ltcIndex = 0; ltcIndex < bms.ltcsPerSegment; ++ltcIndex)
			{
				for (uint16_t cellIndex = 0; cellIndex < bms.cellsPerLtc; ++cellIndex)
				{
					uint16_t index = LTC_TO_CELL_INDEX (&bms, segmentIndex, ltcIndex, cellIndex);

					mvprintw (row + 3, columnCell, "┬──┘└──");
					mvprintw (row + 4, columnCell, "│      ");
					mvprintw (row + 5, columnCell, "│      ");
					mvprintw (row + 6, columnCell, "┴──────");
					mvprintw (row + 7, columnCell, "───────");

					if (cellIndex == 0)
					{
						mvprintw (row + 3, columnCell, "─");
						mvprintw (row + 4, columnCell, " ");
						mvprintw (row + 5, columnCell, " ");
						mvprintw (row + 6, columnCell, "─");
						mvprintw (row + 7, columnCell, "─");
					}

					printVoltage (row + 4, columnCell + 3, &bms, index);
					printCellIndex (row + 7, columnCell + 2, index);

					columnCell += 7;
				}

				if (ltcIndex != bms.ltcsPerSegment - 1)
				{
					mvprintw (row + 3, columnCell,	"──┬┬┬─");
					mvprintw (row + 4, columnCell,	"  │││ ");
					mvprintw (row + 5, columnCell,	"  │││ ");
					mvprintw (row + 6, columnCell,	"──┘│└─");
					mvprintw (row + 7, columnCell,	"───┴──");
					columnCell += 6;
				}
				else
				{
					mvprintw (row + 3, columnCell,	"──┬┤");
					mvprintw (row + 4, columnCell,	"  ││");
					mvprintw (row + 5, columnCell,	"  ││");
					mvprintw (row + 6, columnCell,	"──┘│");
					mvprintw (row + 7, columnCell,	"───┘");
				}

				for (uint16_t senseLineIndex = 0; senseLineIndex < bms.senseLinesPerLtc; ++senseLineIndex)
				{
					uint16_t index = LTC_TO_SENSE_LINE_INDEX (&bms, segmentIndex, ltcIndex, senseLineIndex);
					uint16_t increment;

					if (senseLineIndex != bms.senseLinesPerLtc - 1)
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

					if (senseLineIndex == 5)
						printLtcIndex (row, columnSense - 34, &bms, ltcIndex + segmentIndex * bms.ltcsPerSegment);

					printTemperature (row + 2, columnSense, &bms, index);
					printSenseLineStatus (row + 3, columnSense, &bms, index);
					columnSense += increment;
				}

				if (ltcIndex != bms.ltcsPerSegment - 1)
				{
					mvprintw (row + 0, columnSense,	"┬");
					mvprintw (row + 1, columnSense,	"┴");
					mvprintw (row + 2, columnSense,	"┊");
					columnSense += 1;
				}
			}

			// End of segment
			mvprintw (row + 0, columnSense,	"┐");
			mvprintw (row + 1, columnSense,	"┤");
			mvprintw (row + 2, columnSense,	"│");
			columnSense = 0;
			columnCell = 0;
			row += 10;
		}

		refresh ();
		napms(16);
	}

	endwin ();

	return 0;
}