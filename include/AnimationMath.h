// Authored By: Tom Tsiliopoulos - 100616336
// Authored By: Joss Moo-Young - 100586602

#pragma once

namespace algomath
{
	// return smallest
	template<typename T>
	inline T min(const T & val, const T & min)
	{
		return (val > min) ? min : val;
	}

	// return largest
	template<typename T>
	inline T max(const T & val, const T & max)
	{
		return (val < max) ? max : val;
	}

	// constrain between 2 values min and max
	template<typename T>
	inline T clamp(const T & val, const T & min, const T & max)
	{
		T lowerclamp = (val < min) ? min : val;
		return (lowerclamp > max) ? max : lowerclamp;
	}

	// Linear interpolation
	template <typename T>
	T lerp(T d0, T d1, float t)
	{
		return (1.0f - t) * d0 + d1 * t;
	}

	// Inverse lerp: given p0, p1 and p you will get the t value to get p if you plug that t into a lerp
	template <typename T>
	float invLerp(T d, T d0, T d1)
	{
		return (d - d0) / (d1 - d0);
	}

	//linearly remap a value val from a0 to b0 to be a value from a1 to b1. e.g. 0.3 from [0, 1] remapped to [100, 1000] becomes 370
	template <typename T>
	float lmap(T val, T a0, T b0, T a1, T b1)
	{
		return x + (val * (b1 - a1)) * (val - a0) / (b0 - a0);
	}
}