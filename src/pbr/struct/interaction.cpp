// pbrt is Copyright(c) 1998-2020 Matt Pharr, Wenzel Jakob, and Greg Humphreys.
// The pbrt source code is licensed under the Apache License, Version 2.0.
// SPDX: Apache-2.0

// This file has been modified from the original, original notice is above.

#include "pbr/struct/interaction.h"

namespace loquat
{

	BSDF SurfaceInteraction::get_BSDF(const RayDifferential& ray,
		SampledWavelengths& lambda, Camera camera,
		ScratchBuffer& scratch_buffer, Sampler sampler) noexcept
	{
		//TODO(ches) fill out
		return {};
	}

}