#ifndef MATH_H
#define MATH_H

// ZRE Common Math ------------------------------------------------------------------------------------------------------------
//
// Author: Cole Barach
// Date Created: 2025.12.29
//
// Description: TODO(Barach)

// Functions ------------------------------------------------------------------------------------------------------------------

/**
 * @brief Performs linear interpolation between the values A and B.
 * @param x The input scalar, [0, 1].
 * @param a The minimum value (x = 0 => lerp(x) = a).
 * @param b The maximum value (x = 1 => lerp(x) = b).
 * @return The interpolated value.
 */
static inline float lerp (float x, float a, float b)
{
	// Scale X by range, then offset by min.
	return x * (b - a) + a;
}

/**
 * @brief Performs inverse linear interpolation from the values A and B.
 * @param x The input value, from [A, B].
 * @param a The minimum value (x = A => inverseLerp(x) = 0).
 * @param b The maximum value (x = B => inverseLerp(x) = 1).
 * @return The inverse interpolated value.
 */
static inline float inverseLerp (float x, float a, float b)
{
	// Prevent division-by-zero. If both points are identical, this is the most useful answer.
	if (b == a)
		return 0;

	// Offset x by min, then divide by range.
	return (x - a) / (b - a);
}

/**
 * @brief Performs 2D linear interpolation from the the points A and B.
 * @note This can be used for inverse 2D linear interpolation by swapping the x and y coordinates.
 * @param cx The input point's x-coordinate.
 * @param ax Point A's x-coordinate.
 * @param ay Point A's y-coordinate.
 * @param bx Point B's x-coordinate.
 * @param by Point B's y-coordinate.
 * @return The y-coordinate of the interpolated point.
 */
static inline float lerp2d (float cx, float ax, float ay, float bx, float by)
{
	// Prevent division-by-zero. If both points are identical, this is the most useful answer.
	if (bx == ax)
		return ay;

	// Perform inverse lerp then lerp.
	return (cx - ax) / (bx - ax) * (by - ay) + ay;
}

#endif // MATH_H