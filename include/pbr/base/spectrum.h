// pbrt is Copyright(c) 1998-2020 Matt Pharr, Wenzel Jakob, and Greg Humphreys.
// The pbrt source code is licensed under the Apache License, Version 2.0.
// SPDX: Apache-2.0

// This file has been modified from the original, original notice is above.

#pragma once

#include <array>

namespace loquat
{
	static constexpr int SPECTRUM_SAMPLE_COUNT = 4;


	class SampledSpectrum
	{
	public:
		SampledSpectrum operator+(const SampledSpectrum& s) const noexcept
		{
			SampledSpectrum result = *this;
			return result += s;
		}
		//TODO(ches) fill this out

		SampledSpectrum() = default;

		explicit SampledSpectrum(Float c) noexcept
		{
			values.fill(c);
		}

		SampledSpectrum& operator+=(const SampledSpectrum& s) noexcept
		{
			for (int i = 0; i < SPECTRUM_SAMPLE_COUNT; ++i)
			{
				values[i] += s.values[i];
			}
			return *this;
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

		SampledSpectrum operator*(Float f) const noexcept
		{
			LOG_ASSERT(!is_NaN(f) && "Multiplying spectrum by NaN");
			SampledSpectrum result = *this;
			for (int i = 0; i < SPECTRUM_SAMPLE_COUNT; ++i)
			{
				result.values[i] *= f;
			}
			return result;
		}

		SampledSpectrum& operator*=(Float f) noexcept
		{
			LOG_ASSERT(!is_NaN(f) && "Multiplying spectrum by NaN");
			for (int i = 0; i < SPECTRUM_SAMPLE_COUNT; ++i)
			{
				values[i] *= f;
			}
			return *this;
		}

		SampledSpectrum operator/(Float f) const noexcept
		{
			LOG_ASSERT(f != 0 && "Dividing by zero");
			LOG_ASSERT(!is_NaN(f) && "Dividing spectrum by NaN");
			SampledSpectrum result = *this;
			for (int i = 0; i < SPECTRUM_SAMPLE_COUNT; ++i)
			{
				result.values[i] /= f;
			}
			return result;
		}

		SampledSpectrum& operator/=(Float f) noexcept
		{
			LOG_ASSERT(f != 0 && "Dividing by zero");
			LOG_ASSERT(!is_NaN(f) && "Dividing spectrum by NaN");
			for (int i = 0; i < SPECTRUM_SAMPLE_COUNT; ++i)
			{
				values[i] /= f;
			}
			return *this;
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

	private:
		std::array<Float, SPECTRUM_SAMPLE_COUNT> values;
	};

	class SampledWavelengths
	{

	};
}