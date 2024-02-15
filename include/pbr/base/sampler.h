// pbrt is Copyright(c) 1998-2020 Matt Pharr, Wenzel Jakob, and Greg Humphreys.
// The pbrt source code is licensed under the Apache License, Version 2.0.
// SPDX: Apache-2.0

// This file has been modified from the original, original notice is above.

#pragma once

#include <string>

#include "main/loquat.h"
#include "pbr/math/vector_math.h"
#include "pbr/struct/parameter_dictionary.h"
#include "pbr/util/tagged_pointer.h"

namespace loquat
{
	struct CameraSample
	{
		Point2f pFilm;
		Point2f pLens;
		Float time = 0;
		Float filterWeight = 1;
		std::string to_string() const noexcept;
	};

	class HaltonSampler;
	class PaddedSobolSampler;
	/// <summary>
	/// Progressive Multi-Jittered Sample Sequence sampler, from
	/// https://graphics.pixar.com/library/ProgressiveMultiJitteredSampling/paper.pdf
	/// </summary>
	class PMJ02BNSampler;
	class IndependentSampler;
	class SobolSampler;
	class StratifiedSampler;
	class ZSobolSampler;
	class MLTSampler;
	class DebugMLTSampler;

	class Sampler : public TaggedPointer<PMJ02BNSampler, IndependentSampler,
		StratifiedSampler, HaltonSampler, PaddedSobolSampler, SobolSampler,
		ZSobolSampler, MLTSampler, DebugMLTSampler>
	{
	public:
		using TaggedPointer::TaggedPointer;

		static Sampler create(const std::string_view name,
			const ParameterDictionary& parameters, Point2i full_resolution,
			Allocator allocator);

		[[nodiscard]]
		inline int samples_per_pixel() const noexcept;

		inline void start_pixel_sample(Point2i point, int sample_index,
			int dimension = 0) noexcept;

		[[nodiscard]]
		inline Float get_1D() noexcept;

		[[nodiscard]]
		inline Point2f get_2D() noexcept;
	
		[[nodiscard]]
		inline Point2f get_pixel_2D() noexcept;

		Sampler clone(Allocator allocator = {});

		std::string to_string() const noexcept;
	};
}