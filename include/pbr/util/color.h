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

		LOQUAT_CPU_GPU
		RGB(Float r, Float g, Float b)
			: r{ r }
			, g{ g }
			, b{ b }
		{}

		LOQUAT_CPU_GPU
		RGB(Vec3f color)
			: r{ color.r }
			, g{ color.g }
			, b{ color.b }
		{}

		LOQUAT_CPU_GPU
		RGB& operator+=(RGB other)
		{
			r += other.r;
			g += other.g;
			b += other.b;
			return *this;
		}

		LOQUAT_CPU_GPU
		RGB operator+(RGB other) const
		{
			RGB result = *this;
			return result += other;
		}

		LOQUAT_CPU_GPU
		RGB& operator-=(RGB other)
		{
			r -= other.r;
			g -= other.g;
			b -= other.b;
			return *this;
		}

		LOQUAT_CPU_GPU
		RGB operator-(RGB other) const
		{
			RGB result = *this;
			return result -= other;
		}

		LOQUAT_CPU_GPU
		friend RGB operator-(Float a, RGB color)
		{
			return { a - color.r, a - color.g, a - color.b };
		}

		LOQUAT_CPU_GPU
		RGB& operator*=(RGB other)
		{
			r *= other.r;
			g *= other.g;
			b *= other.b;
			return *this;
		}

		LOQUAT_CPU_GPU
		RGB operator*(RGB other) const
		{
			RGB result = *this;
			return result *= other;
		}

		LOQUAT_CPU_GPU
		RGB operator*(Float a) const
		{
			LOG_ASSERT(!is_NaN(a) && "Multiplying an RGB value with NaN");
			return { a * r, a * g, a * b };
		}

		LOQUAT_CPU_GPU
		RGB& operator*=(Float a)
		{
			LOG_ASSERT(!is_NaN(a) && "Multiplying an RGB value with NaN");
			r *= a;
			g *= a;
			b *= a;
			return *this;
		}

		LOQUAT_CPU_GPU
		friend RGB operator*(Float a, RGB color)
		{
			LOG_ASSERT(!is_NaN(a) && "Multiplying an RGB value with NaN");
			return color * a;
		}

		LOQUAT_CPU_GPU
		RGB& operator/=(RGB other)
		{
			r /= other.r;
			g /= other.g;
			b /= other.b;
			return *this;
		}

		LOQUAT_CPU_GPU
		RGB operator/(RGB other) const
		{
			RGB result = *this;
			return result /= other;
		}

		LOQUAT_CPU_GPU
		RGB& operator/=(Float a)
		{
			LOG_ASSERT(!is_NaN(a) && "Dividing an RGB value by NaN");
			LOG_ASSERT(a != 0 && "Dividing an RGB value by zero");
			r /= a;
			g /= a;
			b /= a;
			return *this;
		}

		LOQUAT_CPU_GPU
		RGB operator/(Float a) const
		{
			RGB result = *this;
			return result /= a;
		}

		LOQUAT_CPU_GPU
		RGB operator-() const
		{
			return { -r, -g, -b };
		}

		LOQUAT_CPU_GPU
		Float average() const
		{
			return (r + g + b) / 3;
		}

		LOQUAT_CPU_GPU
		bool operator==(RGB other) const
		{
			return r == other.r && g == other.g && b == other.b;
		}

		LOQUAT_CPU_GPU
		bool operator!=(RGB other) const
		{
			return r != other.r || g != other.g || b != other.b;
		}

		LOQUAT_CPU_GPU
		Float operator[](int i) const
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

		LOQUAT_CPU_GPU
		Float& operator[](int i)
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
		std::string to_string() const;

		explicit operator Vec3f() const
		{
			return Vec3f{ r, g, b };
		}

		Float r = 0;
		Float g = 0;
		Float b = 0;
	};

	LOQUAT_CPU_GPU
	inline RGB max(RGB a, RGB b)
	{
		return { std::max(a.r, b.r), std::max(a.g, b.g), std::max(a.b, b.b) };
	}

	LOQUAT_CPU_GPU
	inline RGB lerp(Float t, RGB a, RGB b)
	{
		return (1 - t) * a + t * b;
	}

	template <number U, number V>
	LOQUAT_CPU_GPU
	inline RGB clamp(RGB rgb, U min, V max)
	{
		return { clamp(rgb.r, min, max),
			 	 clamp(rgb.g, min, max),
				 clamp(rgb.b, min, max) };
	}

	LOQUAT_CPU_GPU
	inline RGB clamp_zero(RGB rgb)
	{
		return { std::max<Float>(0, rgb.r),
				 std::max<Float>(0, rgb.g),
				 std::max<Float>(0, rgb.b) };
	}

	class XYZ
	{
	public:
		XYZ() = default;

		LOQUAT_CPU_GPU
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
		LOQUAT_CPU_GPU
		static XYZ from_xy(Point2f xy, Float y = 1)
		{
			if (xy.y == 0)
			{
				return { 0, 0, 0 };
			}
			return { xy.x * y / xy.y, y, (1 - xy.x - xy.y) * y / xy.y };
		}

		LOQUAT_CPU_GPU
		XYZ& operator+=(const XYZ& other)
		{
			x += other.x;
			y += other.y;
			z += other.z;
			return *this;
		}

		LOQUAT_CPU_GPU
		XYZ operator+(const XYZ& other) const
		{
			XYZ result = *this;
			return result += other;
		}

		LOQUAT_CPU_GPU
		XYZ& operator-=(const XYZ& other)
		{
			x -= other.x;
			y -= other.y;
			z -= other.z;
			return *this;
		}

		LOQUAT_CPU_GPU
		XYZ operator-(const XYZ& other) const
		{
			XYZ result = *this;
			return result -= other;
		}

		LOQUAT_CPU_GPU
		friend XYZ operator-(Float a, const XYZ& color)
		{
			return { a - color.x, a - color.y, a - color.z };
		}

		LOQUAT_CPU_GPU
		XYZ& operator*=(const XYZ& other)
		{
			x *= other.x;
			y *= other.y;
			z *= other.z;
			return *this;
		}

		LOQUAT_CPU_GPU
		XYZ operator*(const XYZ& other) const
		{
			XYZ result = *this;
			return result *= other;
		}

		LOQUAT_CPU_GPU
		XYZ operator*(Float a) const
		{
			LOG_ASSERT(!is_NaN(a) && "Multiplying an XYZ value with NaN");
			return { a * x, a * y, a * z };
		}

		LOQUAT_CPU_GPU
		XYZ& operator*=(Float a)
		{
			LOG_ASSERT(!is_NaN(a) && "Multiplying an XYZ value with NaN");
			x *= a;
			y *= a;
			z *= a;
			return *this;
		}

		LOQUAT_CPU_GPU
		friend XYZ operator*(Float a, const XYZ& color)
		{
			LOG_ASSERT(!is_NaN(a) && "Multiplying an XYZ value with NaN");
			return color * a;
		}

		LOQUAT_CPU_GPU
		XYZ& operator/=(const XYZ& other)
		{
			x /= other.x;
			y /= other.y;
			z /= other.z;
			return *this;
		}

		LOQUAT_CPU_GPU
		XYZ operator/(const XYZ& other) const
		{
			XYZ result = *this;
			return result /= other;
		}

		LOQUAT_CPU_GPU
		XYZ& operator/=(Float a)
		{
			LOG_ASSERT(!is_NaN(a) && "Dividing an XYZ value by NaN");
			LOG_ASSERT(a != 0 && "Dividing an XYZ value by zero");
			x /= a;
			y /= a;
			z /= a;
			return *this;
		}

		LOQUAT_CPU_GPU
		XYZ operator/(Float a) const
		{
			XYZ result = *this;
			return result /= a;
		}

		LOQUAT_CPU_GPU
		XYZ operator-() const
		{
			return { -x, -y, -z };
		}

		LOQUAT_CPU_GPU
		Float average() const
		{
			return (x + y + z) / 3;
		}

		LOQUAT_CPU_GPU
		Point2f xy() const
		{
			return Point2f{ x / (x + y + z), y / (x + y + z) };
		}

		LOQUAT_CPU_GPU
		bool operator==(const XYZ& other) const
		{
			return x == other.x && y == other.y && z == other.z;
		}

		LOQUAT_CPU_GPU
		bool operator!=(const XYZ& other) const
		{
			return x != other.x || y != other.y || z != other.z;
		}

		LOQUAT_CPU_GPU
		Float operator[](int i) const
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

		LOQUAT_CPU_GPU
		Float& operator[](int i)
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
		std::string to_string() const;

		LOQUAT_CPU_GPU
		explicit operator Vec3f() const
		{
			return Vec3f{ x, y, z };
		}

		Float x = 0;
		Float y = 0;
		Float z = 0;
	};

	LOQUAT_CPU_GPU
	inline XYZ lerp(Float t, const XYZ& a, const XYZ& b)
	{
		return (1 - t) * a + t * b;
	}

	template <number U, number V>
	LOQUAT_CPU_GPU
	inline XYZ clamp(const XYZ& xyz, U min, V max)
	{
		return { clamp(xyz.x, min, max),
				 clamp(xyz.y, min, max),
				 clamp(xyz.z, min, max) };
	}

	LOQUAT_CPU_GPU
	inline XYZ clamp_zero(const XYZ& xyz)
	{
		return { std::max<Float>(0, xyz.x),
				 std::max<Float>(0, xyz.y),
				 std::max<Float>(0, xyz.z) };
	}

	class RGBSigmoidPolynomial
	{
	public:

		RGBSigmoidPolynomial() = default;

		LOQUAT_CPU_GPU
		RGBSigmoidPolynomial(Float c0, Float c1, Float c2)
			: c0{ c0 }
			, c1{ c1 }
			, c2{ c2 }
		{}

		[[nodiscard]]
		std::string to_string() const;

		LOQUAT_CPU_GPU
		Float operator()(Float wavelength) const
		{
			return s(evaluate_polynomial(wavelength, c2, c1, c0));
		}

		[[nodiscard]]
		LOQUAT_CPU_GPU
		Float max_value() const
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
		LOQUAT_CPU_GPU
		static Float s(Float x)
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

	class RGBToSpectrumTable
	{
	public:
		static constexpr int resolution = 64;

		using CoefficientArray = float[3][resolution][resolution][resolution][3];

		RGBToSpectrumTable(const float* z_nodes,
			const CoefficientArray* coefficients)
			: z_nodes{ z_nodes }
			, coefficients{ coefficients }
		{}

		LOQUAT_CPU_GPU
		RGBSigmoidPolynomial operator()(RGB rgb) const;

		static void init(Allocator allocator);

		static const RGBToSpectrumTable* sRGB;
		static const RGBToSpectrumTable* DCI_P3;
		static const RGBToSpectrumTable* Rec2020;
		static const RGBToSpectrumTable* ACES2065_1;

		[[nodiscard]]
		std::string to_string() const;

	private:
		const float* z_nodes;
		const CoefficientArray* coefficients;
	};

	class LinearColorEncoding;
	class sRGBColorEncoding;
	class GammaColorEncoding;

	class ColorEncoding : public TaggedPointer<LinearColorEncoding,
		sRGBColorEncoding, GammaColorEncoding>
	{
	public:
		using TaggedPointer::TaggedPointer;

		LOQUAT_CPU_GPU
		inline void to_linear(std::span<const uint8_t> v_in,
			std::span<Float> v_out) const;

		LOQUAT_CPU_GPU
		inline void from_linear(std::span<const Float> v_in,
			std::span<uint8_t> v_out) const;

		LOQUAT_CPU_GPU
		inline Float to_float_linear(Float v) const;

		[[nodiscard]]
		std::string to_string() const;

		static const ColorEncoding get(std::string_view name,
			Allocator allocator);

		static ColorEncoding Linear;
		static ColorEncoding sRGB;

		static void init(Allocator allocator);
	};

	class LinearColorEncoding
	{
	public:

		LOQUAT_CPU_GPU
		void to_linear(std::span<const uint8_t> v_in, std::span<Float> v_out)
			const
		{
			LOG_ASSERT(v_in.size() == v_out.size()
				&& "Incompatible span sizes");
			for (size_t i = 0; i < v_in.size(); ++i)
			{
				v_out[i] = v_in[i] / 255.0f;
			}
		}

		LOQUAT_CPU_GPU
		Float to_float_linear(Float v) const
		{
			return v;
		}

		LOQUAT_CPU_GPU
		void from_linear(std::span<const Float> v_in,
			std::span<uint8_t> v_out) const
		{
			LOG_ASSERT(v_in.size() == v_out.size()
				&& "Incompatible span sizes");
			for (size_t i = 0; i < v_in.size(); ++i)
			{
				v_out[i] = static_cast<uint8_t>(clamp(v_in[i] * 255.0f + 0.5f,
					0, 255));
			}
		}

		[[nodiscard]]
		std::string to_string() const
		{
			return "[ LinearColorEncoding ]";
		}
	};

	class sRGBColorEncoding
	{
	public:
		LOQUAT_CPU_GPU
		void to_linear(std::span<const uint8_t> v_in, std::span<Float> v_out)
			const;

		LOQUAT_CPU_GPU
		Float to_float_linear(Float v) const;

		LOQUAT_CPU_GPU
		void from_linear(std::span<const Float> v_in,
			std::span<uint8_t> v_out) const;

		[[nodiscard]]
		std::string to_string() const
		{
			return "[ sRGBColorEncoding ]";
		}
	};

	class GammaColorEncoding
	{
	public:

		LOQUAT_CPU_GPU
		GammaColorEncoding(Float gamma);

		LOQUAT_CPU_GPU
		void to_linear(std::span<const uint8_t> v_in, std::span<Float> v_out)
			const;

		LOQUAT_CPU_GPU
		Float to_float_linear(Float v) const;

		LOQUAT_CPU_GPU
		void from_linear(std::span<const Float> v_in,
			std::span<uint8_t> v_out) const;

		[[nodiscard]]
		std::string to_string() const;

	private:
		Float gamma;
		pstd::array<Float, 256> apply_LUT;
		pstd::array<Float, 1024> inverse_LUT;
	};

	inline void ColorEncoding::to_linear(std::span<const uint8_t> v_in,
		std::span<Float> v_out) const
	{
		auto to_linear = [&](auto ptr) { return ptr->to_linear(v_in, v_out); };
		dispatch(to_linear);
	}

	inline Float ColorEncoding::to_float_linear(Float v) const
	{
		auto to_float = [&](auto ptr) { return ptr->to_float_linear(v); };
		return dispatch(to_float);
	}
	
	inline void ColorEncoding::from_linear(std::span<const Float> v_in,
		std::span<uint8_t> v_out) const
	{
		auto from_linear =
			[&](auto ptr) { return ptr->from_linear(v_in, v_out); };
		dispatch(from_linear);
	}

	[[nodiscard]]
	LOQUAT_CPU_GPU
	inline Float linear_to_sRGB(Float value)
	{
		if (value <= 0.0031308f)
		{
			return 12.92f * value;
		}
		// Minimax polynomial approximation from enoki's color.h.
		const float sqrt_value = safe_square_root(value);
		const float p = evaluate_polynomial(sqrt_value,
			-0.0016829072605308378f, 0.03453868659826638f,
			0.7642611304733891f, 2.0041169284241644f,
			0.7551545191665577f, -0.016202083165206348f);
		const float q = evaluate_polynomial(sqrt_value, 4.178892964897981e-7f,
			-0.00004375359692957097f, 0.03467195408529984f,
			0.6085338522168684f, 1.8970238036421054f, 1.0f);

		return p / q * value;
	}

	LOQUAT_CPU_GPU
	inline uint8_t linear_to_sRGB8(Float value, Float dither = 0)
	{
		if (value <= 0)
		{
			return 0;
		}
		if (value >= 1)
		{
			return 255;
		}
		return clamp(std::round(255.0f * linear_to_sRGB(value) + dither),
			0, 255);
	}

	LOQUAT_CPU_GPU
	inline Float sRGB_to_linear(float value)
	{
		if (value <= 0.04045f)
		{
			return value * (1 / 12.92f);
		}
		// Minimax polynomial approximation from enoki's color.h.
		const float p = evaluate_polynomial(value, -0.0163933279112946f,
			-0.7386328024653209f, -11.199318357635072f,
			-47.46726633009393f, -36.04572663838034f);
		const float q = evaluate_polynomial(value, -0.004261480793199332f,
			-19.140923959601675f, -59.096406619244426f, -18.225745396846637f,
			1.0f);

		return p / q * value;
	}

	extern LOQUAT_CONST Float sRGB_to_linear_LUT[256];

	LOQUAT_CPU_GPU
	inline Float sRGB8_to_linear(uint8_t value)
	{
		return sRGB_to_linear_LUT[value];
	}

	const Mat3 LMS_from_XYZ(0.8951,  0.2664, -0.1614,
						   -0.7502,  1.7135,  0.0367,
							0.0389, -0.0685,  1.0296);
	const Mat3 XYZ_from_LMS(0.986993,  -0.147054,  0.159963,
							0.432305,   0.51836,   0.0492912,
						   -0.00852866, 0.0400428, 0.968487);
	
	inline Mat3 white_balance(Point2f src_white, Point2f target_white)
	{
		XYZ source_xyz = XYZ::from_xy(src_white);
		XYZ dest_xyz = XYZ::from_xy(target_white);
		auto source_lms = LMS_from_XYZ * static_cast<Vec3f>(source_xyz);
		auto dest_lms = LMS_from_XYZ * static_cast<Vec3f>(dest_xyz);

		Mat3 lms_correct{ 
			dest_lms[0] / source_lms[0], 0.0f, 0.0f,
			0.0f, dest_lms[1] / source_lms[1], 0.0f,
			0.0f, 0.0f, dest_lms[2] / source_lms[2] };
		return XYZ_from_LMS * lms_correct * LMS_from_XYZ;
	}
}