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

// C Standard Library
#include <time.h>

// Macros ---------------------------------------------------------------------------------------------------------------------

#ifndef timeradd

/// @brief timeradd implementation, as MSYS does not define it.
#define timeradd(a, b, result)								\
	do														\
	{														\
		(result)->tv_sec = (a)->tv_sec + (b)->tv_sec;		\
		(result)->tv_usec = (a)->tv_usec + (b)->tv_usec;	\
		if ((result)->tv_usec >= 1000000)					\
		{													\
			++(result)->tv_sec;								\
			(result)->tv_usec -= 1000000;					\
		}													\
	} while (0)

#endif // timeradd

#ifndef timersub

/// @brief timersub implementation, as MSYS does not define it.
#define timersub(a, b, result)								\
	do														\
	{														\
		(result)->tv_sec = (a)->tv_sec - (b)->tv_sec;		\
		(result)->tv_usec = (a)->tv_usec - (b)->tv_usec;	\
		if ((result)->tv_usec < 0)							\
		{													\
			--(result)->tv_sec;								\
			(result)->tv_usec += 1000000;					\
		}													\
	} while (0)

#endif // timersub

#define timespecCompare(a, b, CMP) 							\
	(((a)->tv_sec == (b)->tv_sec) 							\
	? ((a)->tv_nsec CMP (b)->tv_nsec) 						\
	: ((a)->tv_sec CMP (b)->tv_sec))

/**
 * @brief Adds the values of two timespec structures. This is most commonly used to add a time interval to an actual time.
 * @param a The augend.
 * @param b The addend.
 * @return The sum of the two timespecs, that is, A + B.
 */
static inline struct timespec timespecAdd (const struct timespec* a, const struct timespec* b)
{
	struct timespec c =
	{
		.tv_sec = a->tv_sec + b->tv_sec,
		.tv_nsec = a->tv_nsec + b->tv_nsec
	};

	if (c.tv_nsec > 1e9)
	{
		c.tv_nsec -= 1e9;
		c.tv_sec++;
	}

	return c;
}

/**
 * @brief Subtracts the values to two timespec structures. This is most commonly used to find the interval of time that has
 * passed between two actual times.
 * @param a The minuend.
 * @param b The subtrahend.
 * @return The difference of the two timespecs, that is, A - B.
 */
static inline struct timespec timespecSub (const struct timespec* a, const struct timespec* b)
{
	struct timespec c =
	{
		.tv_sec = a->tv_sec - b->tv_sec,
		.tv_nsec = a->tv_nsec - b->tv_nsec
	};

	if (c.tv_nsec < 0)
	{
		c.tv_nsec += 1e9;
		c.tv_sec--;
	}

	return c;
}

/**
 * @brief Converts a timespec structure into a measurment in nanoseconds.
 * @note This function is susceptible to overflow, should only be used for relative times (a difference in two times). Use at
 * own risk.
 * @param a The time to convert.
 * @return The time, in nanoseconds.
 */
static inline long long timespecToNs (const struct timespec* a)
{
	return a->tv_nsec + a->tv_sec * 1e9;
}

#endif // TIME_PORT_H