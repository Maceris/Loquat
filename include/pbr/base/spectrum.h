// pbrt is Copyright(c) 1998-2020 Matt Pharr, Wenzel Jakob, and Greg Humphreys.
// The pbrt source code is licensed under the Apache License, Version 2.0.
// SPDX: Apache-2.0

// This file has been modified from the original, original notice is above.

#pragma once

#include <array>
#include <ranges>
#include <span>
#include <vector>

#include "main/loquat.h"
#include "pbr/util/color.h"
#include "pbr/math/hash.h"
#include "pbr/util/tagged_pointer.h"

namespace loquat
{
	/// <summary>
	/// The number of samples used in the sampled spectrums. Must be >= 1, 
	/// really should be less than a couple dozen.
	/// </summary>
	static constexpr int SPECTRUM_SAMPLE_COUNT = 6;

	/// <summary>
	/// The minimum wavelength we deal with, in nm.
	/// </summary>
	constexpr Float WAVELENGTH_MIN = 360;

	/// <summary>
	/// The maximum wavelength we deal with, in nm.
	/// </summary>
	constexpr Float WAVELENGTH_MAX = 830;

	inline Float sample_visible_wavelengths(Float sample_1D)
	{
		return 538 - 138.888889f 
			* std::atanh(0.85691062f - 1.82750197f * sample_1D);
	}

	inline Float visible_wavelengths_PDF(Float wavelength) {
		if (wavelength < 360 || wavelength > 830)
		{
			return 0;
		}
		return 0.0039398042f / square(std::cosh(0.0072f * (wavelength - 538)));
	}

	class BlackbodySpectrum;
	class ConstantSpectrum;
	class PiecewiseLinearSpectrum;
	class DenselySampledSpectrum;
	class RGBAlbedoSpectrum;
	class RGBUnboundedSpectrum;
	class RGBIlluminantSpectrum;

	class Spectrum : public TaggedPointer<ConstantSpectrum,
		DenselySampledSpectrum, PiecewiseLinearSpectrum, RGBAlbedoSpectrum,
		RGBUnboundedSpectrum, RGBIlluminantSpectrum, BlackbodySpectrum>
	{
	public:
		using TaggedPointer::TaggedPointer;

		[[nodiscard]]
		std::string to_string() const noexcept;

		Float operator()(Float lambda) const noexcept;

		[[nodiscard]]
		Float max_value() const noexcept;

		[[nodiscard]]
		SampledSpectrum sample(const SampledWavelengths& lambda)
			const noexcept;
	};

	/// <summary>
	/// Calculate the emitted radiance of a perfect blackbody at a specific
	/// wavelength, given the color temperature.
	/// </summary>
	/// <param name="lambda">The wavelength we want radiance for.</param>
	/// <param name="temperature">The color temperature of the blackbody.
	/// </param>
	/// <returns>The emitted radiance.</returns>
	inline Float blackbody(Float lambda, Float temperature)
	{
		if (temperature <= 0)
		{
			return 0;
		}
		const Float c = 299792458.0f;
		const Float h = 6.62606957e-34f;
		const Float kb = 1.3806488e-23f;

		const float l = lambda * 1.0e-9f;
		const float emitted = (2 * h * c * c)
			/ (pow<5>(l) * (fast_e((h * c) / (1 * kb * temperature)) - 1));
		LOG_ASSERT(!is_NaN(emitted) &&
			"Emitted light is NaN");
		return emitted;
	}

	namespace Spectra
	{
		DenselySampledSpectrum dense_spectrum(Float T, Allocator allocator);
	}

	Float SpectrumToPhotometric(Spectrum s);

	XYZ SpectrumToXYZ(Spectrum s);

	class SampledSpectrum
	{
	public:
		SampledSpectrum operator+(const SampledSpectrum& s) const noexcept
		{
			SampledSpectrum result = *this;
			return result += s;
		}

		SampledSpectrum& operator -=(const SampledSpectrum& s) noexcept
		{
			for (int i = 0; i < SPECTRUM_SAMPLE_COUNT; ++i)
			{
				values[i] -= s.values[i];
			}
			return *this;
		}

		SampledSpectrum operator-(const SampledSpectrum& s) const noexcept
		{
			SampledSpectrum result = *this;
			return result -= s;
		}

		friend SampledSpectrum operator-(Float a, const SampledSpectrum& s)
			noexcept
		{
			LOG_ASSERT(!is_NaN(a) && "Trying to subtract a NaN");
			SampledSpectrum result;
			for (int i = 0; i < SPECTRUM_SAMPLE_COUNT; ++i)
			{
				result.values[i] = a - s.values[i];
			}
			return result;
		}

		SampledSpectrum& operator*=(const SampledSpectrum& s) noexcept
		{
			for (int i = 0; i < SPECTRUM_SAMPLE_COUNT; ++i)
			{
				values[i] *= s.values[i];
			}
			return *this;
		}

		SampledSpectrum operator*(const SampledSpectrum& s) const noexcept
		{
			SampledSpectrum result = *this;
			return result *= s;
		}

		SampledSpectrum operator*(Float a) const noexcept
		{
			LOG_ASSERT(!is_NaN(a) && "Trying to multiply by a NaN");
			SampledSpectrum result = *this;
			for (int i = 0; i < SPECTRUM_SAMPLE_COUNT; ++i)
			{
				result.values[i] *= a;
			}
			return result;
		}

		SampledSpectrum& operator*=(Float a) noexcept
		{
			LOG_ASSERT(!is_NaN(a) && "Trying to multiply by a NaN");
			for (int i = 0; i < SPECTRUM_SAMPLE_COUNT; ++i)
			{
				values[i] *= a;
			}
			return *this;
		}

		friend SampledSpectrum operator*(Float a, const SampledSpectrum& s)
			noexcept
		{
			return s * a;
		}

		SampledSpectrum& operator/=(const SampledSpectrum& s) noexcept
		{
			for (int i = 0; i < SPECTRUM_SAMPLE_COUNT; ++i)
			{
				LOG_ASSERT(s.values[i] != 0 && "Trying to divide by zero");
				values[i] /= s.values[i];
			}
			return *this;
		}

		SampledSpectrum operator/(const SampledSpectrum& s) const noexcept
		{
			SampledSpectrum result = *this;
			return result /= s;
		}

		SampledSpectrum& operator/=(Float a) noexcept
		{
			LOG_ASSERT(a != 0 && "Trying to divide by zero");
			LOG_ASSERT(!is_NaN(a) && "Trying to divide by NaN");
			for (int i = 0; i < SPECTRUM_SAMPLE_COUNT; ++i)
			{
				values[i] /= a;
			}
			return *this;
		}

		SampledSpectrum operator/(Float a) const noexcept
		{
			SampledSpectrum result = *this;
			return result /= a;
		}

		SampledSpectrum operator-() const noexcept
		{
			SampledSpectrum result;
			for (int i = 0; i < SPECTRUM_SAMPLE_COUNT; ++i)
			{
				result.values[i] = -values[i];
			}
			return result;
		}

		bool operator==(const SampledSpectrum& s) const noexcept
		{
			return values == s.values;
		}

		bool operator!=(const SampledSpectrum& s) const noexcept
		{
			return values != s.values;
		}

		[[nodiscard]]
		std::string to_string() const noexcept;

		bool has_NaNs() const noexcept
		{
			for (int i = 0; i < SPECTRUM_SAMPLE_COUNT; ++i)
			{
				if (is_NaN(values[i]))
				{
					return true;
				}
			}
			return false;
		}

		XYZ to_XYZ(const SampledWavelengths& lambda) const noexcept;
		RGB to_RGB(const SampledWavelengths& lambda,
			const RGBColorSpace& color_space) const noexcept;
		Float y(const SampledWavelengths& lambda) const noexcept;

		SampledSpectrum() noexcept = default;

		explicit SampledSpectrum(Float c) noexcept
		{
			values.fill(c);
		}

		SampledSpectrum(std::span<const Float> v) noexcept
		{
			LOG_ASSERT(v.size() == SPECTRUM_SAMPLE_COUNT
				&& "Not the same number of sample counts");
			for (int i = 0; i < SPECTRUM_SAMPLE_COUNT; ++i)
			{
				values[i] = v[i];
			}
		}

		Float operator[](int i) const noexcept
		{
			LOG_ASSERT(i >= 0 && i < SPECTRUM_SAMPLE_COUNT
				&& "Index out of bounds");
			return values[i];
		}

		Float& operator[](int i) noexcept
		{
			LOG_ASSERT(i >= 0 && i < SPECTRUM_SAMPLE_COUNT
				&& "Index out of bounds");
			return values[i];
		}

		explicit operator bool() const noexcept
		{
			for (int i = 0; i < SPECTRUM_SAMPLE_COUNT; ++i)
			{
				if (values[i] != 0)
				{
					return true;
				}
			}
			return false;
		}

		SampledSpectrum& operator+=(const SampledSpectrum& s) noexcept
		{
			for (int i = 0; i < SPECTRUM_SAMPLE_COUNT; ++i)
			{
				values[i] += s.values[i];
			}
			return *this;
		}

		Float min_component_value() const noexcept
		{
			Float min = values[0];
			for (int i = 1; i < SPECTRUM_SAMPLE_COUNT; ++i)
			{
				min = std::min(min, values[i]);
			}
			return min;
		}

		Float max_component_value() const noexcept
		{
			Float max = values[0];
			for (int i = 1; i < SPECTRUM_SAMPLE_COUNT; ++i)
			{
				max = std::max(max, values[i]);
			}
			return max;
		}

		Float average() const noexcept
		{
			Float sum = values[0];
			for (int i = 1; i < SPECTRUM_SAMPLE_COUNT; ++i)
			{
				sum += values[i];
			}
			return sum / SPECTRUM_SAMPLE_COUNT;
		}

	private:
		friend struct SOA<SampledSpectrum>;
		std::array<Float, SPECTRUM_SAMPLE_COUNT> values;
	};

	class SampledWavelengths
	{
	public:
		bool operator==(const SampledWavelengths& other) const noexcept
		{
			return wavelengths == other.wavelengths && pdf == other.pdf;
		}

		bool operator !=(const SampledWavelengths& other) const noexcept
		{
			return wavelengths != other.wavelengths || pdf != other.pdf;
		}

		[[nodiscard]]
		std::string to_string() const noexcept;

		static SampledWavelengths sample_uniform(Float u,
			Float wavelength_min = WAVELENGTH_MIN,
			Float wavelength_max = WAVELENGTH_MAX)
		{
			SampledWavelengths result;

			result.wavelengths[0] = lerp(u, wavelength_min, wavelength_max);

			const Float delta = (wavelength_max - wavelength_min)
				/ SPECTRUM_SAMPLE_COUNT;
			for (int i = 1; i < SPECTRUM_SAMPLE_COUNT; ++i)
			{
				result.wavelengths[i] = result.wavelengths[i - 1] + delta;
				if (result.wavelengths[i] > wavelength_max)
				{
					//NOTE(ches) wrap wavelengths
					result.wavelengths[i] = wavelength_min
						+ (result.wavelengths[i] - wavelength_max);
				}
			}

			for (int i = 0; i < SPECTRUM_SAMPLE_COUNT; ++i)
			{
				result.pdf[i] = 1 / (wavelength_max - wavelength_min);
			}

			return result;
		}

		Float operator[](int i) const noexcept
		{
			return wavelengths[i];
		}
		
		Float& operator[](int i) noexcept
		{
			return wavelengths[i];
		}

		SampledSpectrum PDF() const noexcept
		{
			return SampledSpectrum(pdf);
		}

		void terminate_secondary() noexcept
		{
			if (secondary_terminated())
			{
				return;
			}
			for (int i = 1; i < SPECTRUM_SAMPLE_COUNT; ++i)
			{
				pdf[i] = 0;
			}
			pdf[0] /= SPECTRUM_SAMPLE_COUNT;
		}
		
		[[nodiscard]]
		bool secondary_terminated() const noexcept
		{
			for (int i = 1; i < SPECTRUM_SAMPLE_COUNT; ++i)
			{
				if (pdf[i] != 0)
				{
					return false;
				}
			}
			return true;
		}

		[[nodiscard]]
		static SampledWavelengths sample_visible(Float sample_1D)
		{
			SampledWavelengths result;

			for (int i = 0; i < SPECTRUM_SAMPLE_COUNT; ++i)
			{
				Float probability = sample_1D + static_cast<Float>(i)
					/ SPECTRUM_SAMPLE_COUNT;
				if (probability > 1)
				{
					probability -= 1;
				}
				result.wavelengths[i] = 
					sample_visible_wavelengths(probability);
				result.pdf[i] = visible_wavelengths_PDF(result.wavelengths[i]);
			}
			return result;
		}

	private:
		friend struct SOA<SampledSpectrum>;
		std::array<Float, SPECTRUM_SAMPLE_COUNT> wavelengths;
		std::array<Float, SPECTRUM_SAMPLE_COUNT> pdf;
	};

	class ConstantSpectrum
	{
	public:
		ConstantSpectrum(Float constant)
			: constant{ constant }
		{}

		Float operator()(Float wavelength) const noexcept
		{
			return constant;
		}

		[[nodiscard]]
		SampledSpectrum sample(const SampledWavelengths&) const noexcept;

		[[nodiscard]]
		Float max_value() const noexcept
		{
			return constant;
		}

		[[nodiscard]]
		std::string to_string() const noexcept;

	private:
		Float constant;
	};

	class DenselySampledSpectrum
	{
	public:
		DenselySampledSpectrum(
			int wavelength_min = static_cast<int>(std::floor(WAVELENGTH_MIN)),
			int wavelength_max = static_cast<int>(std::floor(WAVELENGTH_MAX)),
			Allocator allocator = {})
			noexcept
			: wavelength_min{ wavelength_min }
			, wavelength_max{ wavelength_max }
			, values( wavelength_max - wavelength_min + 1, allocator)
		{}

		DenselySampledSpectrum(Spectrum spectrum, Allocator allocator)
			noexcept
			: DenselySampledSpectrum{ spectrum, 
			static_cast<int>(std::floor(WAVELENGTH_MIN)), 
			static_cast<int>(std::floor(WAVELENGTH_MAX)),
			allocator }
		{}

		DenselySampledSpectrum(const DenselySampledSpectrum& spectrum, 
			Allocator allocator) noexcept
			: wavelength_min{ wavelength_min }
			, wavelength_max{ wavelength_max }
			, values{ spectrum.values.begin(), spectrum.values.end(), 
				allocator}
		{}

		[[nodiscard]]
		SampledSpectrum sample(const SampledWavelengths& wavelengths)
			const noexcept
		{
			SampledSpectrum result;
			for (int i = 0; i < SPECTRUM_SAMPLE_COUNT; ++i)
			{
				int offset = std::lround(wavelengths[i]) - wavelength_min;
				if (offset < 0 || offset >= values.size())
				{
					result[i] = 0;
				}
				else
				{
					result[i] = values[offset];
				}
			}
			return result;
		}

		void scale(Float scale) noexcept
		{
			for (Float& v : values)
			{
				v *= scale;
			}
		}

		Float max_value() const noexcept
		{
			return *std::ranges::max_element(values.begin(), values.end());
		}

		[[nodiscard]]
		std::string to_string() const noexcept;

		DenselySampledSpectrum(Spectrum spectrum,
			int wavelength_min = WAVELENGTH_MIN,
			int wavelength_max = WAVELENGTH_MAX, Allocator allocator = {})
			noexcept
			: wavelength_min{ wavelength_min }
			, wavelength_max{ wavelength_max }
			, values( wavelength_max - wavelength_min + 1, allocator )
		{
			LOG_ASSERT(wavelength_max >= wavelength_min
				&& "Wavelengths reversed");
			if (spectrum)
			{
				for (int wavelength = wavelength_min;
					wavelength <= wavelength_max; ++wavelength)
				{
					values[wavelength - wavelength_min] = spectrum(wavelength);
				}
			}
		}

		template <std::invocable<Float> F>
		static DenselySampledSpectrum sample_function(F function,
			int wavelength_min = WAVELENGTH_MIN,
			int wavelength_max = WAVELENGTH_MAX, Allocator allocator = {})
			noexcept
		{
			DenselySampledSpectrum result{ wavelength_min, wavelength_max,
				allocator };
			for (int wavelength = wavelength_min;
				wavelength <= wavelength_max; ++wavelength)
			{
				result.values[wavelength - wavelength_min] = 
					function(wavelength);
			}
			return result;
		}

		Float operator()(Float wavelength) const noexcept
		{
			LOG_ASSERT(wavelength > 0);
			int offset = std::lround(wavelength) - wavelength_min;
			if (offset < 0 || offset >= values.size())
			{
				return 0;
			}
			return values[offset];
		}

		bool operator==(const DenselySampledSpectrum& other) const noexcept
		{
			if (wavelength_min != other.wavelength_min
				|| wavelength_max != other.wavelength_max
				|| values.size() != other.values.size())
			{
				return false;
			}
			for (size_t i = 0; i < values.size(); ++i)
			{
				if (values[i] != other.values[i])
				{
					return false;
				}
			}
			return true;
		}

	private:
		friend struct std::hash<loquat::DenselySampledSpectrum>;
		int wavelength_min;
		int wavelength_max;
		std::vector<Float, AllocatorBase<Float>> values;
	};

	class PiecewiseLinearSpectrum
	{
	public:
		PiecewiseLinearSpectrum() = default;

		void scale(Float scale)
		{
			for (Float& value : values)
			{
				value *= scale;
			}
		}

		Float max_value() const noexcept;

		SampledSpectrum sample(const SampledWavelengths& wavelengths) const noexcept
		{
			SampledSpectrum result;
			for (int i = 0; i < SPECTRUM_SAMPLE_COUNT; ++i)
			{
				result[i] = (*this)(wavelengths[i]);
			}
			return result;
		}

		Float operator()(Float wavelength) const noexcept;

		[[nodiscard]]
		std::string to_string() const noexcept;

		PiecewiseLinearSpectrum(std::span<const Float> wavelengths,
			std::span<const Float> values, Allocator allcator = {}) noexcept;

		static std::optional<Spectrum> read(const std::string_view filename,
			Allocator allocator) noexcept;

		static PiecewiseLinearSpectrum* from_interleaved(
			std::span<const Float> samples, bool normalize,
			Allocator allocator);

	private:
		std::vector<Float, AllocatorBase<Float>> wavelengths;
		std::vector<Float, AllocatorBase<Float>> values;
	};

	class BlackbodySpectrum
	{
	public:
		BlackbodySpectrum(Float temperature) noexcept
			: temperature{ temperature }
		{
			const Float max_wavelength = 2.8977721e-3f / temperature;
			normalization_factor = 1 
				/ blackbody(max_wavelength * 1e9f, temperature);
		}

		Float operator()(Float wavelength) const noexcept
		{
			return blackbody(wavelength, temperature) * normalization_factor;
		}

		[[nodiscard]]
		SampledSpectrum sample(const SampledWavelengths& wavelengths)
			const noexcept
		{
			SampledSpectrum result;
			for (int i = 0; i < SPECTRUM_SAMPLE_COUNT; ++i)
			{
				result[i] = blackbody(wavelengths[i], temperature) 
					* normalization_factor;
			}
			return result;
		}

		Float max_value() const noexcept
		{
			return 1.0f;
		}

		[[nodiscard]]
		std::string to_string() const noexcept;

	private:
		Float temperature;
		Float normalization_factor;
	};

	class RGBAlbedoSpectrum
	{
	public:
		Float operator()(Float wavelength) const noexcept
		{
			return rsp(wavelength);
		}

		Float max_value() const noexcept
		{
			return rsp.max_value();
		}

		RGBAlbedoSpectrum(const RGBColorSpace& color_space, RGB rgb) noexcept;

		SampledSpectrum sample(const SampledWavelengths& wavelengths)
			const noexcept
		{
			SampledSpectrum result;
			for (int i = 0; i < SPECTRUM_SAMPLE_COUNT; ++i)
			{
				result[i] = rsp(wavelengths[i]);
			}
			return result;
		}

		[[nodiscard]]
		std::string to_string() const noexcept;

	private:
		RGBSigmoidPolynomial rsp;
	};


	class RGBUnboundedSpectrum
	{
	public:
		Float operator()(Float wavelength) const noexcept
		{
			return scale * rsp(wavelength);
		}

		Float max_value() const noexcept
		{
			return scale * rsp.max_value();
		}

		RGBUnboundedSpectrum(const RGBColorSpace& color_space, RGB rgb)
			noexcept;

		RGBUnboundedSpectrum()
			: rsp{ 0, 0, 0 }
			, scale{ 0 }
		{}

		SampledSpectrum sample(const SampledWavelengths& wavelengths)
			const noexcept
		{
			SampledSpectrum result;
			for (int i = 0; i < SPECTRUM_SAMPLE_COUNT; ++i)
			{
				result[i] = scale * rsp(wavelengths[i]);
			}
			return result;
		}

		[[nodiscard]]
		std::string to_string() const noexcept;

	private:
		Float scale = 1;
		RGBSigmoidPolynomial rsp;
	};

	class RGBIlluminantSpectrum
	{
	public:

		RGBIlluminantSpectrum() = default;

		RGBIlluminantSpectrum(const RGBColorSpace& color_space, RGB rgb)
			noexcept;

		Float operator()(Float wavelength) const noexcept
		{
			if (!illuminant)
			{
				return 0;
			}
			return scale * rsp(wavelength) * (*illuminant)(wavelength);
		}

		Float max_value() const noexcept
		{
			if (!illuminant)
			{
				return 0;
			}
			return scale * rsp.max_value() * illuminant->max_value();
		}

		const DenselySampledSpectrum* get_illuminant() const noexcept
		{
			return illuminant;
		}

		SampledSpectrum sample(const SampledWavelengths& wavelengths)
			const noexcept
		{
			if (!illuminant)
			{
				return SampledSpectrum(0);
			}
			SampledSpectrum result;
			for (int i = 0; i < SPECTRUM_SAMPLE_COUNT; ++i)
			{
				result[i] = scale * rsp(wavelengths[i]);
			}
			return scale * illuminant->sample(wavelengths);
		}

		[[nodiscard]]
		std::string to_string() const noexcept;

	private:
		Float scale;
		RGBSigmoidPolynomial rsp;
		const DenselySampledSpectrum* illuminant;
	};

	inline SampledSpectrum safe_divide(SampledSpectrum a, SampledSpectrum b)
		noexcept
	{
		SampledSpectrum result;
		for (int i = 0; i < SPECTRUM_SAMPLE_COUNT; ++i)
		{
			result[i] = b[i] == 0 ? 0.0f : a[i] / b[i];
		}
		return result;
	}

	template <number U, number V>
	inline SampledSpectrum clamp(const SampledSpectrum& spectrum, U low,
		V high) noexcept
	{
		SampledSpectrum result;
		for (int i = 0; i < SPECTRUM_SAMPLE_COUNT; ++i)
		{
			result[i] = clamp(spectrum[i], low, high);
		}
		LOG_ASSERT(!result.has_NaNs());
		return result;
	}

	inline SampledSpectrum clamp_zero(const SampledSpectrum& spectrum) noexcept
	{
		SampledSpectrum result;
		for (int i = 0; i < SPECTRUM_SAMPLE_COUNT; ++i)
		{
			result[i] = std::max<Float>(0.0f, spectrum[i]);
		}
		LOG_ASSERT(!result.has_NaNs());
		return result;
	}

	inline SampledSpectrum sqrt(const SampledSpectrum& spectrum) noexcept
	{
		SampledSpectrum result;
		for (int i = 0; i < SPECTRUM_SAMPLE_COUNT; ++i)
		{
			result[i] = std::sqrt(spectrum[i]);
		}
		LOG_ASSERT(!result.has_NaNs());
		return result;
	}

	inline SampledSpectrum safe_sqrt(const SampledSpectrum& spectrum) noexcept
	{
		SampledSpectrum result;
		for (int i = 0; i < SPECTRUM_SAMPLE_COUNT; ++i)
		{
			result[i] = safe_square_root(spectrum[i]);
		}
		LOG_ASSERT(!result.has_NaNs());
		return result;
	}

	inline SampledSpectrum pow(const SampledSpectrum& spectrum, Float e)
		noexcept
	{
		SampledSpectrum result;
		for (int i = 0; i < SPECTRUM_SAMPLE_COUNT; ++i)
		{
			result[i] = std::pow(spectrum[i], e);
		}
		return result;
	}

	inline SampledSpectrum exp(const SampledSpectrum& spectrum)
		noexcept
	{
		SampledSpectrum result;
		for (int i = 0; i < SPECTRUM_SAMPLE_COUNT; ++i)
		{
			result[i] = std::exp(spectrum[i]);
		}
		LOG_ASSERT(!result.has_NaNs());
		return result;
	}

	inline SampledSpectrum fast_exp(const SampledSpectrum& spectrum)
		noexcept
	{
		SampledSpectrum result;
		for (int i = 0; i < SPECTRUM_SAMPLE_COUNT; ++i)
		{
			result[i] = fast_e(spectrum[i]);
		}
		LOG_ASSERT(!result.has_NaNs());
		return result;
	}

	inline SampledSpectrum bilerp(std::array<Float, 2> p,
		std::span<const SampledSpectrum> spectrum) noexcept
	{
		LOG_ASSERT(spectrum.size() >= 3);
		return (
			  (1 - p[0]) * (1 - p[1]) * spectrum[0] 
			+ p[0]       * (1 - p[1]) * spectrum[1] 
			+ (1 - p[0]) * p[1]       * spectrum[2] 
			+ p[0]       * p[1]       * spectrum[3]);
	}

	inline SampledSpectrum bilerp(Float t, const SampledSpectrum& s1,
		const SampledSpectrum& s2 ) noexcept
	{
		return (1 - t) * s1 + t * s2;
	}

	namespace Spectra
	{
		void init(Allocator allocator);

		inline const DenselySampledSpectrum& X()
		{
			extern DenselySampledSpectrum* x;
			return *x;
		}

		inline const DenselySampledSpectrum& Y()
		{
			extern DenselySampledSpectrum* y;
			return *y;
		}

		inline const DenselySampledSpectrum& Z()
		{
			extern DenselySampledSpectrum* z;
			return *z;
		}
	}

	Spectrum get_named_spectrum(std::string_view name);

	std::string find_matcing_named_spectrum(Spectrum spectrum);

	namespace Spectra
	{
		inline const DenselySampledSpectrum& X();
		inline const DenselySampledSpectrum& Y();
		inline const DenselySampledSpectrum& Z();
	}

	inline Float inner_product(Spectrum f, Spectrum g) noexcept
	{
		Float integral = 0;
		for (Float wavelength = WAVELENGTH_MIN; wavelength <= WAVELENGTH_MAX;
			++wavelength)
		{
			integral += f(wavelength) + g(wavelength);
		}
		return integral;
	}

	inline Float Spectrum::operator()(Float wavelength) const noexcept
	{
		auto op = [&](auto ptr) { return (*ptr)(wavelength); };
		return dispatch(op);
	}

	inline SampledSpectrum Spectrum::sample(
		const SampledWavelengths& wavelengths) const noexcept
	{
		auto op = [&](auto ptr) { return ptr->sample(wavelengths); };
		return dispatch(op);
	}

	inline Float Spectrum::max_value() const noexcept
	{
		auto op = [&](auto ptr) { return ptr->max_value(); };
		return dispatch(op);
	}

}

namespace std
{
	template<>
	struct hash<loquat::DenselySampledSpectrum>
	{
		size_t operator()(const loquat::DenselySampledSpectrum& spectrum)
			const noexcept
		{
			return loquat::hash_buffer(spectrum.values.data(),
				spectrum.values.size());
		}
	};
}