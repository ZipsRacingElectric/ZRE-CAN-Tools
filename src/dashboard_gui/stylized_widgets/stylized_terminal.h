#ifndef STYLIZED_TERMINAL_H
#define STYLIZED_TERMINAL_H

// Stylized GTK Terminal Widget -----------------------------------------------------------------------------------------------
//
// Author: Cole Barach
// Date Created: 2026.01.06
//
// Description: TODO(Barach): A terminal-like label.

// Includes -------------------------------------------------------------------------------------------------------------------

// GTK
#include <gtk/gtk.h>

// Datatypes ------------------------------------------------------------------------------------------------------------------

typedef struct
{
	/// @brief The maximum length, in characters, of each line of the terminal. Does not include the terminator character.
	size_t lineLengthMax;
	/// @brief The maximum number of lines in the terminal. Note the actual number of lines may be limited due to widget size.
	size_t lineCountMax;
	/// @brief The size, in pixels, of the font.
	float fontSize;
	/// @brief The size, in pixels, of the gaps between each line of text.
	float fontSpacing;
	/// @brief Indicates whether the terminal should support scrolling.
	bool scrollEnabled;
	/// @brief The minimum inclusive value of the scroll position. Only used if @c scrollEnabled is true.
	int scrollMin;
	/// @brief The maximum exclusive value of the scroll position. Only used if @c scrollEnabled is true.
	int scrollMax;
	/// @brief The color of the terminal's background.
	GdkRGBA backgroundColor;
	/// @brief The color of the terminal's font.
	GdkRGBA fontColor;
	/// @brief The color of the outline of the terminal's font, if any.
	GdkRGBA fontOutlineColor;
	/// @brief The thickness, in pixels, of the outline of the terminal's font.
	float fontOutlineThickness;
} stylizedTerminalConfig_t;

typedef struct
{
	char* buffer;
	GtkWidget* widget;
	stylizedTerminalConfig_t config;
	size_t linesWritten;
	size_t lineCount;

	int scrollPosition;
	int scrollPressed;
	float yPressed;
	bool pressed;
} stylizedTerminal_t;

#define STYLIZED_TERMINAL_TO_WIDGET(term) ((term)->widget)

// Functions ------------------------------------------------------------------------------------------------------------------

/**
 * @brief Creates and initializes a terminal widget.
 * @param config The configuration to use.
 * @return The terminal widget, if successful, @c NULL otherwise.
 */
stylizedTerminal_t* stylizedTerminalInit (stylizedTerminalConfig_t* config);

/**
 * @brief Gets the size of a terminal's line buffer. See @c stylizedTerminalGetBuffer .
 * @param term The terminal widget to use.
 * @return The size of the buffer, including a byte for the terminator.
 */
static inline size_t stylizedTerminalGetBufferSize (stylizedTerminal_t* term)
{
	return term->config.lineLengthMax + 1;
}

/**
 * @brief Starts a write operation to a terminal widget. All lines in the buffer are cleared and a buffer for the first line is
 * returned.
 * @param term The terminal to get the buffer for.
 * @note Use @c stylizedTerminalGetBufferSize to determine the size of this buffer.
 * @return The buffer belonging to the first line. @c NULL if there are no lines to write to.
 */
char* stylizedTerminalGetBuffer (stylizedTerminal_t* term);

/**
 * @brief Gets a buffer for the next line in a terminal widget.
 * @param term The terminal to get the buffer for.
 * @return The buffer belonging to the next line. @c NULL if there is no next line to write to.
 */
char* stylizedTerminalNextLine (stylizedTerminal_t* term);

/**
 * @brief Finalizes a write operation to a terminal widget. The number of lines to render is based on the number of buffers
 * accessed since the last call to @c stylizedTerminalGetBuffer .
 * @param term The terminal to finish the write of.
 */
void stylizedTerminalWriteBuffer (stylizedTerminal_t* term);

/**
 * @brief Gets the number of visible lines in a terminal widget. Note this is only valid after a call to
 * @c stylizedTerminalGetBuffer .
 * @param term The terminal to get the line count of.
 * @return The number of visable and usable lines in the terminal.
 */
size_t stylizedTerminalGetLineCount (stylizedTerminal_t* term);

/**
 * @brief Gets the scroll position of a terminal, in number of lines.
 * @param term The terminal to get the scroll of.
 * @return The scroll position.
 */
static inline int stylizedTerminalGetScrollPosition (stylizedTerminal_t* term)
{
	return term->scrollPosition;
}

#endif // STYLIZED_TERMINAL_H