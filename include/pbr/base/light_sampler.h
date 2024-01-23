// pbrt is Copyright(c) 1998-2020 Matt Pharr, Wenzel Jakob, and Greg Humphreys.
// The pbrt source code is licensed under the Apache License, Version 2.0.
// SPDX: Apache-2.0

// This file has been modified from the original, original notice is above.

#pragma once

#include <string>
#include <span>

#include "main/loquat.h"

#include "pbr/util/tagged_pointer.h"

namespace loquat
{
	/// <summary>
	/// A light and the discrete probability of it having been sampled.
	/// </summary>
	struct SampledLight
	{
		Float light;
		Float probability = 0;

		[[nodiscard]]
		std::string to_string() const noexcept;
	};

	class UniformLightSampler;
	class PowerLightSampler;
	class BVHLightSampler;
	class ExhaustiveLightSampler;

	class LightSampler : public TaggedPointer<UniformLightSampler,
		PowerLightSampler, ExhaustiveLightSampler, BVHLightSampler>
	{
	public:
		using TaggedPointer::TaggedPointer;

		[[nodiscard]]
		static LightSampler create(const std::string_view name,
			std::span<const Light> lights, Allocator allocator) noexcept;

		[[nodiscard]]
		std::string to_string() const noexcept;

		[[nodiscard]]
		inline std::optional<SampledLight> sample(
			const LightSampleContext& context, Float sample_1D) const noexcept;

		[[nodiscard]]
		inline Float probability_mass_function(
			const LightSampleContext& context, Light light) const noexcept;

		[[nodiscard]]
		inline std::optional<SampledLight> sample(Float sample_1D)
			const noexcept;

		[[nodiscard]]
		inline Float probability_mass_function(Light light) const noexcept;
	};
}