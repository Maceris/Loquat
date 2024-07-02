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

	// Forward declarations

	template <typename T>
		requires std::integral<T> || std::floating_point<T>
	constexpr T square(T value) noexcept;

	// Regular declarations

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

	inline Float error_function_inverse(Float a) noexcept
	{
		// https://stackoverflow.com/a/49743348
		float p;
		float t = std::log(
			std::max(FMA(a, -a, static_cast<Float>(1.0f)), 
			std::numeric_limits<Float>::min())
		);
		LOG_ASSERT(!is_NaN(t) && !is_inf(t));
		if (std::abs(t) > 6.125f)
		{                                   // maximum ulp error = 2.35793
			p = 3.03697567e-10f;            //  0x1.4deb44p-32
			p = FMA(p, t, 2.93243101e-8f);  //  0x1.f7c9aep-26
			p = FMA(p, t, 1.22150334e-6f);  //  0x1.47e512p-20
			p = FMA(p, t, 2.84108955e-5f);  //  0x1.dca7dep-16
			p = FMA(p, t, 3.93552968e-4f);  //  0x1.9cab92p-12
			p = FMA(p, t, 3.02698812e-3f);  //  0x1.8cc0dep-9
			p = FMA(p, t, 4.83185798e-3f);  //  0x1.3ca920p-8
			p = FMA(p, t, -2.64646143e-1f); // -0x1.0eff66p-2
			p = FMA(p, t, 8.40016484e-1f);  //  0x1.ae16a4p-1
		}
		else
		{                                   // maximum ulp error = 2.35456
			p = 5.43877832e-9f;             //  0x1.75c000p-28
			p = FMA(p, t, 1.43286059e-7f);  //  0x1.33b458p-23
			p = FMA(p, t, 1.22775396e-6f);  //  0x1.49929cp-20
			p = FMA(p, t, 1.12962631e-7f);  //  0x1.e52bbap-24
			p = FMA(p, t, -5.61531961e-5f); // -0x1.d70c12p-15
			p = FMA(p, t, -1.47697705e-4f); // -0x1.35be9ap-13
			p = FMA(p, t, 2.31468701e-3f);  //  0x1.2f6402p-9
			p = FMA(p, t, 1.15392562e-2f);  //  0x1.7a1e4cp-7
			p = FMA(p, t, -2.32015476e-1f); // -0x1.db2aeep-3
			p = FMA(p, t, 8.86226892e-1f);  //  0x1.c5bf88p-1
		}
		return a * p;
	}
	
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

	template <std::predicate<size_t> Predicate>
	inline size_t find_interval(size_t size, const Predicate& predicate) noexcept
	{
		using ssize_t = std::make_signed_t<size_t>;
		ssize_t sz = static_cast<ssize_t>(size) - 2;
		ssize_t first = 1;
		while (sz > 0)
		{
			size_t half = static_cast<size_t>(sz) >> 1;
			size_t middle = first + half;
			bool result = predicate(middle);
			first = result ? middle + 1 : first;
			size = result ? size - (half + 1) : half;
		}
		return static_cast<size_t>(clamp(static_cast<size_t>(first) + 1, 0, size - 2));
	}

	inline Float gaussian(Float x, Float mu = 0, Float sigma = 1) noexcept
	{
		return 1 / std::sqrt(2 * PI * square(sigma))
			* fast_e(-square(x - mu) / (2 * square(sigma)));
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

	inline Float log2(Float x) noexcept
	{
		const double inv_log2 = 1.442695040888963387004650940071;
		return std::log(x) * inv_log2;
	}

	inline int log2_int(float v) noexcept
	{
		LOG_ASSERT(v > 0);
		if (v < 1)
		{
			return -log2_int(1 / v);
		}
		// https://graphics.stanford.edu/~seander/bithacks.html#IntegerLog
		// (With an additional check of the significant to get round-to-nearest
		// rather than round down.)
		// midsignif = Significand(std::pow(2., 1.5))
		// i.e. grab the significand of a value halfway between two exponents,
		// in log space.
		const uint32_t midsignif = 0x0053'04F3;
		return exponent(v) + ((significand(v) >= midsignif) ? 1 : 0);
	}

	inline int log2_int(double v) noexcept
	{
		LOG_ASSERT(v > 0);
		if (v < 1)
		{
			return -log2_int(1 / v);
		}
		// https://graphics.stanford.edu/~seander/bithacks.html#IntegerLog
		// (With an additional check of the significant to get round-to-nearest
		// rather than round down.)
		// midsignif = Significand(std::pow(2., 1.5))
		// i.e. grab the significand of a value halfway between two exponents,
		// in log space.
		const uint64_t midsignif = 0x0006'A09E'667F'3BCD;
		return exponent(v) + ((significand(v) >= midsignif) ? 1 : 0);
	}

	inline int log2_int(uint32_t v) noexcept
	{
#if __GNUC__
		return 31 - __builtin_clz(v);
#else
		unsigned long lz = 0;
#if defined(_WIN64)
		_BitScanReverse64(&lz, v);
#else
		if (_BitScanReverse(&lz, v >> 32))
		{
			lz += 32;
		}
		else
		{
			_BitScanReverse(&lz, v & 0xffffffff);
		}
#endif // _WIN64
		return lz;
#endif // __GNUC__
	}

	inline int log2_int(int32_t v) noexcept
	{
		return log2_int(static_cast<uint32_t>(v));
	}

	inline int log2_int(uint64_t v) noexcept
	{
#ifdef __GNUC__
		return 63 - __builtin_clzll(v);
#else
		unsigned long lz = 0;
#if defined(_WIN64)
		_BitScanReverse64(&lz, v);
#else
		if (_BitScanReverse(&lz, v >> 32))
		{
			lz += 32;
		}
		else
		{
			_BitScanReverse(&lz, v & 0xffffffff);
		}
#endif // _WIN64
		return lz;
#endif // __GNUC__
	}

	inline int log2_int(int64_t v) noexcept
	{
		return log2_int(static_cast<uint64_t>(v));
	}

	template <typename T>
		requires std::integral<T> || std::floating_point<T>
	inline int log4_int(T v) noexcept
	{
		return log2_int(v) / 2;
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

	[[nodiscard]]
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
	inline uint64_t reverse_bits_64(uint64_t n) noexcept
	{
		uint64_t n0 = reverse_bits_32((uint32_t) n);
		uint64_t n1 = reverse_bits_32((uint32_t)(n >> 32));
		return (n0 << 32) | n1;
	}

	[[nodiscard]]
	inline constexpr int32_t round_up_pow2(int32_t v) noexcept
	{
		--v;
		v |= v >> 1;
		v |= v >> 2;
		v |= v >> 4;
		v |= v >> 8;
		v |= v >> 16;
		return v + 1;
	}

	[[nodiscard]]
	inline constexpr int64_t round_up_pow2(int64_t v) noexcept
	{
		--v;
		v |= v >> 1;
		v |= v >> 2;
		v |= v >> 4;
		v |= v >> 8;
		v |= v >> 16;
		v |= v >> 32;
		return v + 1;
	}

	template <typename T>
		requires std::integral<T> || std::floating_point<T>
	[[nodiscard]]
	inline T round_up_pow4(T v) noexcept
	{
		return is_power_of_4(v) ? v : (1 << (2 * (1 + log4_int(v))));
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

}
