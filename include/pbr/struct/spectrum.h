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

}