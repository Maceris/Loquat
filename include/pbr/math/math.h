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
	inline constexpr Float evaluate_polynomial(Float t, C c) noexcept
	{
		return c;
	}

	template <typename Float, typename C, typename... Args>
	inline constexpr Float evaluate_polynomial(Float t, C c,
		Args... remaining) noexcept
	{
		return std::fma(t, evaluate_polynomial(t, remaining...), c);
	}

	/// <summary>
	/// Calculate e^x based on https://stackoverflow.com/a/10792321.
	/// </summary>
	/// <param name="x">The exponent.</param>
	/// <returns>e to the power of the provided x.</returns>
	inline float fast_e(float x) noexcept
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

	inline uint64_t left_shift_2(uint64_t x) noexcept
	{
		x &= 0xffffffff;
		x = (x ^ (x << 16)) & 0x0000ffff0000ffff;
		x = (x ^ (x << 8)) & 0x00ff00ff00ff00ff;
		x = (x ^ (x << 4)) & 0x0f0f0f0f0f0f0f0f;
		x = (x ^ (x << 2)) & 0x3333333333333333;
		x = (x ^ (x << 1)) & 0x5555555555555555;
		return x;
	}

	inline Float lerp(Float x, Float a, Float b) noexcept
	{
		return (1 - x) * a + x * b;
	}

	inline uint64_t encode_morton_2(uint32_t x, uint32_t y) noexcept
	{
		return (left_shift_2(y) << 1) | left_shift_2(x);
	}

	template <std::integral T>
	inline constexpr bool is_power_of_2(T v) noexcept
	{
		return v && !(v & (v - 1));
	}

    inline int permutation_element(uint32_t i, uint32_t l, uint32_t p) noexcept
    {
        uint32_t w = l - 1;
        w |= w >> 1;
        w |= w >> 2;
        w |= w >> 4;
        w |= w >> 8;
        w |= w >> 16;
        do {
            i ^= p;
            i *= 0xe170893d;
            i ^= p >> 16;
            i ^= (i & w) >> 4;
            i ^= p >> 8;
            i *= 0x0929eb3f;
            i ^= p >> 23;
            i ^= (i & w) >> 1;
            i *= 1 | p >> 27;
            i *= 0x6935fa69;
            i ^= (i & w) >> 11;
            i *= 0x74dcb303;
            i ^= (i & w) >> 2;
            i *= 0x9e501cc3;
            i ^= (i & w) >> 2;
            i *= 0xc860a3df;
            i &= w;
            i ^= i >> 5;
        } while (i >= l);
        
        return (i + p) % l;
    }

	template <int n>
	constexpr float pow(const float v) noexcept
	{
		if constexpr (n < 0)
		{
			return 1 / pow<-n>(v);
		}
		const float n2 = pow<n/2>(v);
		return n2 * n2 * pow<n & 1>(v);
	}

	template <>
	constexpr float pow<1>(const float v) noexcept
	{
		return v;
	}

	template <>
	constexpr float pow<0>(const float v) noexcept
	{
		return 1;
	}

	template <int n>
	constexpr float pow(const double v) noexcept
	{
		if constexpr (n < 0)
		{
			return 1 / pow<-n>(v);
		}
		const float n2 = pow<n / 2>(v);
		return n2 * n2 * pow<n & 1>(v);
	}

	template <>
	constexpr float pow<1>(const double v) noexcept
	{
		return v;
	}

	template <>
	constexpr float pow<0>(const double v) noexcept
	{
		return 1;
	}

	[[nodiscard]]
	inline uint32_t reverse_bits_32(uint32_t n) noexcept
	{
		n = (n << 16) | (n >> 16);
		n = ((n & 0x00ff00ff) << 8) | ((n & 0xff00ff00) >> 8);
		n = ((n & 0x0f0f0f0f) << 4) | ((n & 0xf0f0f0f0) >> 4);
		n = ((n & 0x33333333) << 2) | ((n & 0xcccccccc) >> 2);
		n = ((n & 0x55555555) << 1) | ((n & 0xaaaaaaaa) >> 1);
		return n;
	}

	[[nodiscard]]
	inline uint64_t ReverseBits64(uint64_t n) noexcept
	{
		uint64_t n0 = reverse_bits_32((uint32_t) n);
		uint64_t n1 = reverse_bits_32((uint32_t)(n >> 32));
		return (n0 << 32) | n1;
	}

	template <typename T>
		requires std::integral<T> || std::floating_point<T>
	constexpr T square(T value) noexcept
	{
		return value * value;
	}

	[[nodiscard]]
	inline float safe_square_root(float x) noexcept
	{
		LOG_ASSERT(x >= -1e-3f 
			&& "Computing square root of very negative float");
		return std::sqrt(std::max(0.0f, x));
	}

	[[nodiscard]]
	inline float safe_square_root(double x) noexcept
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
