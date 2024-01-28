// pbrt is Copyright(c) 1998-2020 Matt Pharr, Wenzel Jakob, and Greg Humphreys.
// The pbrt source code is licensed under the Apache License, Version 2.0.
// SPDX: Apache-2.0

// This file has been modified from the original, original notice is above.

#pragma once

#include "main/loquat.h"
#include "pbr/util/tagged_pointer.h"

namespace loquat
{
	/// <summary>
	/// The minimum wavelength we deal with, in nm.
	/// </summary>
	constexpr Float WAVELENGTH_MIN = 360;

	/// <summary>
	/// The maximum wavelength we deal with, in nm.
	/// </summary>
	constexpr Float WAVELENGTH_MAX = 830;

	/// <summary>
	/// The number of samples used in the sampled spectrums. Must be >= 1, 
	/// really should be less than a couple dozen.
	/// </summary>
	static constexpr int NUM_SPECTRUM_SAMPLES = 6;

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
}