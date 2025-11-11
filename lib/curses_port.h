#ifndef CURSES_PORT_H
#define CURSES_PORT_H

// Curses Header Portability --------------------------------------------------------------------------------------------------
//
// Author: Cole Barach
// Date Created: 2025.11.10
//
// Description: Portability header for the POSIX curses library. For some reason the include path of MSYS is not fully
//   compliant with how POSIX specifies it, so this header translates that.

// Includes -------------------------------------------------------------------------------------------------------------------

#ifdef ZRE_CANTOOLS_OS_linux
#include <curses.h>
#endif // ZRE_CANTOOLS_OS_linux

#ifdef ZRE_CANTOOLS_OS_windows
#include <ncurses/curses.h>
#endif // ZRE_CANTOOLS_OS_windows

// C Standard Library
#include <locale.h>

// Functions ------------------------------------------------------------------------------------------------------------------

/**
 * @brief Sets the program's locale to support wide characters.
 */
static inline void cursesSetWideLocale (void)
{
	#ifdef ZRE_CANTOOLS_OS_linux
	setlocale (LC_ALL, "");
	#endif // ZRE_CANTOOLS_OS_linux

	#ifdef ZRE_CANTOOLS_OS_windows
	setlocale (LC_ALL, ".utf8");
	#endif // ZRE_CANTOOLS_OS_windows
}

#endif // CURSES_PORT_H