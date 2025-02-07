// pbrt is Copyright(c) 1998-2020 Matt Pharr, Wenzel Jakob, and Greg Humphreys.
// The pbrt source code is licensed under the Apache License, Version 2.0.
// SPDX: Apache-2.0

// This file has been modified from the original, original notice is above.

#include "pbr/struct/interaction.h"

namespace loquat
{

	BSDF SurfaceInteraction::get_BSDF(const RayDifferential& ray,
		SampledWavelengths& lambda, Camera camera,
		ScratchBuffer& scratch_buffer, Sampler sampler)
	{
		//TODO(ches) fill out
		return {};
	}

	std::string Interaction::to_string() const
	{
		//TODO(ches) finish this
		return "";
	}

	std::string SurfaceInteraction::to_string() const
	{
		//TODO(ches) finish this
		return "";
	}
}