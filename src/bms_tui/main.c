// BMS TUI --------------------------------------------------------------------------------------------------------------------
//
// Author: Cole Barach
// Date Created: 2025.05.24
//
// Description: Terminal user interface for monitoring a battery management system.

// Includes -------------------------------------------------------------------------------------------------------------------

// Includes
#include "bms.h"
#include "cjson/cjson_util.h"
#include "error_codes.h"

// NCurses
#include <ncurses.h>
#include <locale.h>

// C Standard Library
#include <errno.h>

// Entrypoint -----------------------------------------------------------------------------------------------------------------

int main (int argc, char** argv)
{
	if (argc != 4)
	{
		fprintf (stderr, "Format: bms-tui <device name> <DBC file path> <config file path>\n");
		return -1;
	}

	const char* deviceName = argv [1];
	const char* dbcPath = argv [2];
	const char* configPath = argv [3];

	// Initialize the database.
	canDatabase_t database;
	if (canDatabaseInit (&database, deviceName, dbcPath) != 0)
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

	setlocale (LC_ALL, "");
	initscr ();
	if(has_colors() == FALSE)
	{
		endwin ();
		fprintf (stderr, "Terminal environment does not support colors.\n");
		return -1;
	}

	start_color ();
	NCURSES_PAIRS_T colorRed = 1;
	NCURSES_PAIRS_T colorGreen = 2;
	NCURSES_PAIRS_T colorCyan = 3;
	init_pair (colorRed, COLOR_RED, COLOR_BLACK);
	init_pair (colorGreen, COLOR_GREEN, COLOR_BLACK);
	init_pair (colorCyan, COLOR_CYAN, COLOR_BLACK);

	while (true)
	{
		uint16_t row = 0;
		uint16_t cellColumn = 0;
		uint16_t senseLineColumn = 0;

		for (uint16_t segmentIndex = 0; segmentIndex < bms.segmentCount; ++segmentIndex)
		{
			// Start of segment
			mvprintw (row + 3, cellColumn, "├─");
			mvprintw (row + 4, cellColumn, "│═");
			mvprintw (row + 5, cellColumn, "│═");
			mvprintw (row + 6, cellColumn, "└─");
			cellColumn += 2;

			for (uint16_t ltcIndex = 0; ltcIndex < bms.ltcsPerSegment; ++ltcIndex)
			{
				for (uint16_t senseLineIndex = 0; senseLineIndex < bms.senseLinesPerLtc; ++senseLineIndex)
				{
					uint16_t increment;
					if (senseLineIndex == bms.senseLinesPerLtc - 1 && ltcIndex != bms.ltcsPerSegment - 1)
					{
						mvprintw (row + 0, senseLineColumn, "┌────┬");
						mvprintw (row + 1, senseLineColumn, "│    │");
						mvprintw (row + 2, senseLineColumn, "│    │");
						increment = 6;
					}
					else if (senseLineIndex == 0 && ltcIndex != 0)
					{
						mvprintw (row + 0, senseLineColumn, "────┐");
						mvprintw (row + 1, senseLineColumn, "    │");
						mvprintw (row + 2, senseLineColumn, "    │");
						senseLineColumn -= 1;
						increment = 7;
					}
					else
					{
						mvprintw (row + 0, senseLineColumn, "┌────┐");
						mvprintw (row + 1, senseLineColumn, "│    │");
						mvprintw (row + 2, senseLineColumn, "│    │");
						increment = 7;
					}

					uint16_t index = LTC_TO_SENSE_LINE_INDEX (&bms, segmentIndex, ltcIndex, senseLineIndex);
					
					if (bms.senseLineTemperaturesValid [index] == NULL || !*bms.senseLineTemperaturesValid [index])
					{
						mvprintw (row + 1, senseLineColumn + 2, "--");
					}
					else
					{
						float temperature = *bms.senseLineTemperatures [index];
						
						NCURSES_PAIRS_T color = colorGreen;
						if (temperature > bms.maxTemperature || temperature < bms.minTemperature)
							color = colorRed;

						attron (COLOR_PAIR (color));
						mvprintw (row + 1, senseLineColumn + 1, "%.1f", temperature);
						attroff (COLOR_PAIR (color));
					}


					if (*bms.senseLinesOpenValid [index])
					{
						if (signalToBool (*bms.senseLinesOpen [index]))
						{
							attron (COLOR_PAIR (colorRed));
							mvprintw (row + 2, senseLineColumn + 2, "XX");
							attroff (COLOR_PAIR (colorRed));
						}
					}
					else
					{
						mvprintw (row + 2, senseLineColumn + 2, "--");
					}

					senseLineColumn += increment;
				}

				for (uint16_t cellIndex = 0; cellIndex < bms.cellsPerLtc; ++cellIndex)
				{
					mvprintw (row + 3, cellColumn, "┬──┴─┴─");
					mvprintw (row + 4, cellColumn, "│      ");
					mvprintw (row + 5, cellColumn, "│      ");
					mvprintw (row + 6, cellColumn, "┴──────");

					uint16_t index = LTC_TO_CELL_INDEX (&bms, segmentIndex, ltcIndex, cellIndex);
					if (!*bms.cellVoltagesValid [index] || !*bms.cellsDischargingValid [index])
					{
						mvprintw (row + 4, cellColumn + 3, "--");
					}
					else
					{
						float voltage = *bms.cellVoltages [index];
						
						NCURSES_PAIRS_T color = colorGreen;
						if (voltage > bms.maxCellVoltage || voltage < bms.minCellVoltage)
							color = colorRed;
						else if (signalToBool (*bms.cellsDischarging [index]))
							color = colorCyan;
						
						attron (COLOR_PAIR (color));

						int voltageInt = (uint8_t) voltage;
						int voltageFrac = (uint8_t) ((voltage - voltageInt) * 100);
						mvprintw (row + 4, cellColumn + 3, "%i.", voltageInt);
						mvprintw (row + 5, cellColumn + 3, "%02i", voltageFrac);

						attroff (COLOR_PAIR (color));
					}

					cellColumn += 7;
				}

				// Middle of LTCs
				if (ltcIndex == bms.ltcsPerSegment - 1)
					continue;
				mvprintw (row + 3, cellColumn, "┬──┴─");
				mvprintw (row + 4, cellColumn, "│════");
				mvprintw (row + 5, cellColumn, "│════");
				mvprintw (row + 6, cellColumn, "┴────");
				cellColumn += 5;
			}

			// End of segment
			mvprintw (row + 3, cellColumn, "┬──┤");
			mvprintw (row + 4, cellColumn, "│══│");
			mvprintw (row + 5, cellColumn, "│══│");
			mvprintw (row + 6, cellColumn, "┴──┘");

			cellColumn = 0;
			senseLineColumn = 0;
			row += 8;
		}

		refresh ();
		napms(1);
	}

	endwin ();

	return 0;
}