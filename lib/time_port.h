#ifndef TIME_PORT_H
#define TIME_PORT_H

// POSIX Time Portability -----------------------------------------------------------------------------------------------------
//
// Author: Cole Barach
// Date Created: 2025.07.31
//
// Description: Portability layer for the POSIX sys/time.h header.

// Includes -------------------------------------------------------------------------------------------------------------------

// POSIX
#include <sys/time.h>

// Macros ---------------------------------------------------------------------------------------------------------------------

#ifndef __unix__
/// @brief timeradd implementation, as MinGW does not define it.
#define timeradd(a, b, result)								\
	do {													\
		(result)->tv_sec = (a)->tv_sec + (b)->tv_sec;		\
		(result)->tv_usec = (a)->tv_usec + (b)->tv_usec;	\
		if ((result)->tv_usec >= 1000000)					\
		{													\
			++(result)->tv_sec;								\
			(result)->tv_usec -= 1000000;					\
		}													\
	} while (0)
#endif // __unix__

#endif // TIME_PORT_H