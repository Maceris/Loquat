// pbrt is Copyright(c) 1998-2020 Matt Pharr, Wenzel Jakob, and Greg Humphreys.
// The pbrt source code is licensed under the Apache License, Version 2.0.
// SPDX: Apache-2.0

// This file has been modified from the original, original notice is above.

#pragma once

#include "main/loquat.h"

#include "pbr/math/math.h"
#include "pbr/util/tagged_pointer.h"
#include "pbr/math/vector_math.h"

#ifdef RGB
	#undef RGB
#endif

namespace loquat
{
	class RGB
	{
	public:
		RGB() = default;
		RGB(Float r, Float g, Float b)
			: r{ r }
			, g{ g }
			, b{ b }
		{}

		RGB& operator+=(RGB other) noexcept
		{
			r += other.r;
			g += other.g;
			b += other.b;
			return *this;
		}

		RGB operator+(RGB other) const noexcept
		{
			RGB result = *this;
			return result += other;
		}

		RGB& operator-=(RGB other) noexcept
		{
			r -= other.r;
			g -= other.g;
			b -= other.b;
			return *this;
		}

		RGB operator-(RGB other) const noexcept
		{
			RGB result = *this;
			return result -= other;
		}

		friend RGB operator-(Float a, RGB color) noexcept
		{
			return { a - color.r, a - color.g, a - color.b };
		}

		RGB& operator*=(RGB other) noexcept
		{
			r *= other.r;
			g *= other.g;
			b *= other.b;
			return *this;
		}

		RGB operator*(RGB other) const noexcept
		{
			RGB result = *this;
			return result *= other;
		}

		RGB operator*(Float a) const noexcept
		{
			LOG_ASSERT(!is_NaN(a) && "Multiplying an RGB value with NaN");
			return { a * r, a * g, a * b };
		}

		RGB& operator*=(Float a) noexcept
		{
			LOG_ASSERT(!is_NaN(a) && "Multiplying an RGB value with NaN");
			r *= a;
			g *= a;
			b *= a;
			return *this;
		}

		friend RGB operator*(Float a, RGB color) noexcept
		{
			LOG_ASSERT(!is_NaN(a) && "Multiplying an RGB value with NaN");
			return color * a;
		}

		RGB& operator/=(RGB other) noexcept
		{
			r /= other.r;
			g /= other.g;
			b /= other.b;
			return *this;
		}

		RGB operator/(RGB other) const noexcept
		{
			RGB result = *this;
			return result /= other;
		}

		RGB& operator/=(Float a) noexcept
		{
			LOG_ASSERT(!is_NaN(a) && "Dividing an RGB value by NaN");
			LOG_ASSERT(a != 0 && "Dividing an RGB value by zero");
			r /= a;
			g /= a;
			b /= a;
			return *this;
		}

		RGB operator/(Float a) const noexcept
		{
			RGB result = *this;
			return result /= a;
		}

		RGB operator-() const noexcept
		{
			return { -r, -g, -b };
		}

		Float average() const noexcept
		{
			return (r + g + b) / 3;
		}

		bool operator==(RGB other) const noexcept
		{
			return r == other.r && g == other.g && b == other.b;
		}

		bool operator!=(RGB other) const noexcept
		{
			return r != other.r || g != other.g || b != other.b;
		}

		Float operator[](int i) const noexcept
		{
			LOG_ASSERT(i >= 0 && i < 3 && "Index out of bounds");
			switch (i)
			{
			case 1:
				return r;
			case 2:
				return g;
			case 3:
			default:
				return b;
			}
		}
		Float& operator[](int i) noexcept
		{
			LOG_ASSERT(i >= 0 && i < 3 && "Index out of bounds");
			switch (i)
			{
			case 1:
				return r;
			case 2:
				return g;
			case 3:
			default:
				return b;
			}
		}

		[[nodiscard]]
		std::string to_string() const noexcept;

		Float r = 0;
		Float g = 0;
		Float b = 0;
	};

	inline RGB max(RGB a, RGB b) noexcept
	{
		return { std::max(a.r, b.r), std::max(a.g, b.g), std::max(a.b, b.b) };
	}

	inline RGB lerp(Float t, RGB a, RGB b) noexcept
	{
		return (1 - t) * a + t * b;
	}

	template <number U, number V>
	inline RGB clamp(RGB rgb, U min, V max) noexcept
	{
		return { clamp(rgb.r, min, max),
			 	 clamp(rgb.g, min, max),
				 clamp(rgb.b, min, max) };
	}

	inline RGB clamp_zero(RGB rgb) noexcept
	{
		return { std::max<Float>(0, rgb.r),
				 std::max<Float>(0, rgb.g),
				 std::max<Float>(0, rgb.b) };
	}

	class XYZ
	{
	public:
		XYZ() = default;
		XYZ(Float x, Float y, Float z)
			: x{ x }
			, y{ y }
			, z{ z }
		{}

		/// <summary>
		/// Converts chromaticity coordinates and y lambda to XYZ color space.
		/// </summary>
		/// <param name="xy">The x and y chromaticity coordinates.</param>
		/// <param name="y">The y lambda value.</param>
		/// <returns>The XYZ color.</returns>
		static XYZ from_xy(Point2f xy, Float y = 1)
		{
			if (xy.y == 0)
			{
				return { 0, 0, 0 };
			}
			return { xy.x * y / xy.y, y, (1 - xy.x - xy.y) * y / xy.y };
		}

		XYZ& operator+=(const XYZ& other) noexcept
		{
			x += other.x;
			y += other.y;
			z += other.z;
			return *this;
		}

		XYZ operator+(const XYZ& other) const noexcept
		{
			XYZ result = *this;
			return result += other;
		}

		XYZ& operator-=(const XYZ& other) noexcept
		{
			x -= other.x;
			y -= other.y;
			z -= other.z;
			return *this;
		}

		XYZ operator-(const XYZ& other) const noexcept
		{
			XYZ result = *this;
			return result -= other;
		}

		friend XYZ operator-(Float a, const XYZ& color) noexcept
		{
			return { a - color.x, a - color.y, a - color.z };
		}

		XYZ& operator*=(const XYZ& other) noexcept
		{
			x *= other.x;
			y *= other.y;
			z *= other.z;
			return *this;
		}

		XYZ operator*(const XYZ& other) const noexcept
		{
			XYZ result = *this;
			return result *= other;
		}

		XYZ operator*(Float a) const noexcept
		{
			LOG_ASSERT(!is_NaN(a) && "Multiplying an XYZ value with NaN");
			return { a * x, a * y, a * z };
		}

		XYZ& operator*=(Float a) noexcept
		{
			LOG_ASSERT(!is_NaN(a) && "Multiplying an XYZ value with NaN");
			x *= a;
			y *= a;
			z *= a;
			return *this;
		}

		friend XYZ operator*(Float a, const XYZ& color) noexcept
		{
			LOG_ASSERT(!is_NaN(a) && "Multiplying an XYZ value with NaN");
			return color * a;
		}

		XYZ& operator/=(const XYZ& other) noexcept
		{
			x /= other.x;
			y /= other.y;
			z /= other.z;
			return *this;
		}

		XYZ operator/(const XYZ& other) const noexcept
		{
			XYZ result = *this;
			return result /= other;
		}

		XYZ& operator/=(Float a) noexcept
		{
			LOG_ASSERT(!is_NaN(a) && "Dividing an XYZ value by NaN");
			LOG_ASSERT(a != 0 && "Dividing an XYZ value by zero");
			x /= a;
			y /= a;
			z /= a;
			return *this;
		}

		XYZ operator/(Float a) const noexcept
		{
			XYZ result = *this;
			return result /= a;
		}

		XYZ operator-() const noexcept
		{
			return { -x, -y, -z };
		}

		Float average() const noexcept
		{
			return (x + y + z) / 3;
		}

		Point2f xy() const noexcept
		{
			return Point2f{ x / (x + y + z), y / (x + y + z) };
		}

		bool operator==(const XYZ& other) const noexcept
		{
			return x == other.x && y == other.y && z == other.z;
		}

		bool operator!=(const XYZ& other) const noexcept
		{
			return x != other.x || y != other.y || z != other.z;
		}

		Float operator[](int i) const noexcept
		{
			LOG_ASSERT(i >= 0 && i < 3 && "Index out of bounds");
			switch (i)
			{
			case 1:
				return x;
			case 2:
				return y;
			case 3:
			default:
				return z;
			}
		}
		Float& operator[](int i) noexcept
		{
			LOG_ASSERT(i >= 0 && i < 3 && "Index out of bounds");
			switch (i)
			{
			case 1:
				return x;
			case 2:
				return y;
			case 3:
			default:
				return z;
			}
		}

		[[nodiscard]]
		std::string to_string() const noexcept;

		Float x = 0;
		Float y = 0;
		Float z = 0;
	};

	inline XYZ lerp(Float t, const XYZ& a, const XYZ& b) noexcept
	{
		return (1 - t) * a + t * b;
	}

	template <number U, number V>
	inline XYZ clamp(const XYZ& xyz, U min, V max) noexcept
	{
		return { clamp(xyz.x, min, max),
				 clamp(xyz.y, min, max),
				 clamp(xyz.z, min, max) };
	}

	inline XYZ clamp_zero(const XYZ& xyz) noexcept
	{
		return { std::max<Float>(0, xyz.x),
				 std::max<Float>(0, xyz.y),
				 std::max<Float>(0, xyz.z) };
	}

	class RGBSigmoidPolynomial
	{
	public:

		RGBSigmoidPolynomial() = default;

		RGBSigmoidPolynomial(Float c0, Float c1, Float c2)
			: c0{ c0 }
			, c1{ c1 }
			, c2{ c2 }
		{}

		[[nodiscard]]
		std::string to_string() const noexcept;

		Float operator()(Float wavelength) const noexcept
		{
			return s(evaluate_polynomial(wavelength, c2, c1, c0));
		}

		[[nodiscard]]
		Float max_value() const noexcept
		{
			Float result = std::max((*this)(360), (*this)(830));
			Float wavelength = -c1 / (2 * c0);
			if (wavelength >= 360 && wavelength <= 830)
			{
				result = std::max(result, (*this)(wavelength));
			}
			return result;
		}

	private:
		static Float s(Float x) noexcept
		{
			if (is_NaN(x))
			{
				return x > 0 ? 1 : 0;
			}
			return 0.5f + x / (2 * std::sqrt(1 + square(x)));
		}
		
		Float c0;
		Float c1;
		Float c2;
	};
}