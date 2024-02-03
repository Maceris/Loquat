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

	/// <summary>
	/// A concept for checking if a type is either an integral or a
	/// floating point value.
	/// </summary>
	template <typename T>
	concept number = std::integral<T> || std::floating_point<T>;
	
	template <typename Float, typename C>
	inline constexpr Float evaluate_polynomial(Float t, C c)
	{
		return c;
	}

	template <typename Float, typename C, typename... Args>
	inline constexpr Float evaluate_polynomial(Float t, C c,
		Args... remaining)
	{
		return std::fma(t, evaluate_polynomial(t, remaining...), c);
	}

	/// <summary>
	/// Calculate e^x based on https://stackoverflow.com/a/10792321.
	/// </summary>
	/// <param name="x">The exponent.</param>
	/// <returns>e to the power of the provided x.</returns>
	inline float fast_e(float x)
	{
		// Compute x' such that e^x = 2^x'
		float xp = x * 1.442695041f;

		// Find integer and fractional components of x'
		const float fxp = std::floor(xp);
		const float f = xp - fxp;
		const int i = (int)fxp;

		const float twoToF = evaluate_polynomial(f, 1.0f, 0.695556856f,
			0.226173572f, 0.0781455737f);

		const int exp = exponent(twoToF) + i;

		if (exp < -126)
		{
			return 0;
		}
		if (exp > 127)
		{
			return FLOAT_INFINITY;
		}

		uint32_t bits = std::bit_cast<uint32_t>(twoToF);
		bits &= 0b10000000011111111111111111111111u;
		bits |= (exp + 127) << 23;
		return std::bit_cast<float>(bits);
	}

	inline Float lerp(Float x, Float a, Float b)
	{
		return (1 - x) * a + x * b;
	}

	template <int n>
	constexpr float pow(const float v)
	{
		if constexpr (n < 0)
		{
			return 1 / pow<-n>(v);
		}
		const float n2 = pow<n/2>(v);
		return n2 * n2 * pow<n & 1>(v);
	}

	template <>
	constexpr float pow<1>(const float v)
	{
		return v;
	}

	template <>
	constexpr float pow<0>(const float v)
	{
		return 1;
	}

	template <int n>
	constexpr float pow(const double v)
	{
		if constexpr (n < 0)
		{
			return 1 / pow<-n>(v);
		}
		const float n2 = pow<n / 2>(v);
		return n2 * n2 * pow<n & 1>(v);
	}

	template <>
	constexpr float pow<1>(const double v)
	{
		return v;
	}

	template <>
	constexpr float pow<0>(const double v)
	{
		return 1;
	}

	template <typename T>
		requires std::integral<T> || std::floating_point<T>
	constexpr T square(T value) noexcept
	{
		return value * value;
	}

	[[nodiscard]]
	inline float safe_square_root(float x)
	{
		LOG_ASSERT(x >= -1e-3f 
			&& "Computing square root of very negative float");
		return std::sqrt(std::max(0.0f, x));
	}

	[[nodiscard]]
	inline float safe_square_root(double x)
	{
		LOG_ASSERT(x >= -1e-3f
			&& "Computing square root of very negative double");
		return std::sqrt(std::max(0.0, x));
	}

	template <number T, number U, number V>
	inline constexpr T clamp(T value, U low, V high) noexcept
	{
		if (value < low)
		{
			return static_cast<T>(low);
		}
		else if (value > high)
		{
			return static_cast<T>(high);
		}
		else
		{
			return value;
		}
	}

}