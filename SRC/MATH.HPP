#ifndef MATH_HPP
#define MATH_HPP

/** Compute minimum of two values of same type.
 *
 * @param a First value.
 * @param b Second value.
 * @return Minimum value.
 */
template <class T>
T min(T a, T b) {
   return ((a < b) ? a : b);
}

/** Compute maximum of two values of same type.
 *
 * @param a First value.
 * @param b Second value.
 * @return Maximum value.
 */
template <class T>
T max(T a, T b) {
   return ((a > b) ? a : b);
}

/** Clamp value to specified boundaries inclusively.
 *
 * @param v Value.
 * @param min Minimum inclusive value.
 * @param max Maximum inclusive value.
 * @return Clamped value.
 */
template <class T>
T clamp(T v, T min, T max) {
	if (max < min)
		max = min;

	if (v < min) return min;
	if (v > max) return max;

	return v;
}

/** Linear interpolate from v0 to v1.
 *
 * @param v0 First value.
 * @param v1 Second value.
 * @param t Interpolation factor in range [0.0, 1.0].
 * @return Interpolated value of same type as v0 and v1.
 */
template <class T>
T lerp(T v0, T v1, float t) {
	return (T)((1 - t) * v0 + t * v1);
}

#endif