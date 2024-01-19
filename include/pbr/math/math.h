// pbrt is Copyright(c) 1998-2020 Matt Pharr, Wenzel Jakob, and Greg Humphreys.
// The pbrt source code is licensed under the Apache License, Version 2.0.
// SPDX: Apache-2.0

// This file has been modified from the original, original notice is above.

#pragma once

#include <numbers>

namespace loquat
{
	/// <summary>
	/// To avoid incorrect intersections with surfaces due to floating point
	/// errors, this is used to set t_max just under 1 to stop before 
	/// hitting light source surfaces.
	/// </summary>
	constexpr Float SHADOW_EPSILON = 0.0001f;

	/// <summary>
	/// Pi, in the floating point format that we are using.
	/// </summary>
	constexpr Float PI = static_cast<Float>(std::numbers::pi);

	/// <summary>
	/// 1 / Pi, in the floating point format we are using.
	/// </summary>
	constexpr Float INV_PI = static_cast<Float>(std::numbers::inv_pi);

	/// <summary>
	/// 1 / (2 * Pi), in the floating point format we are using.
	/// </summary>
	constexpr Float INV_2PI = static_cast<Float>(std::numbers::inv_pi / 2);

	/// <summary>
	/// 1 / (4 * Pi), in the floating point format we are using.
	/// </summary>
	constexpr Float INV_4PI = static_cast<Float>(std::numbers::inv_pi / 4);

	/// <summary>
	/// Pi / 2, in the floating point format we are using.
	/// </summary>
	constexpr Float PI_OVER_2 = static_cast<Float>(std::numbers::pi / 2);

	/// <summary>
	/// Pi / 4, in the floating point format we are using.
	/// </summary>
	constexpr Float PI_OVER_4 = static_cast<Float>(std::numbers::pi / 4);

	/// <summary>
	/// sqrt(2), in the floating point format we are using.
	/// </summary>
	constexpr Float SQRT2 = static_cast<Float>(std::numbers::sqrt2);

	template <typename T>
		requires std::integral<T> || std::floating_point<T>
	constexpr T square(T value) noexcept
	{
		return value * value;
	}
	
}