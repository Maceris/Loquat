// pbrt is Copyright(c) 1998-2020 Matt Pharr, Wenzel Jakob, and Greg Humphreys.
// The pbrt source code is licensed under the Apache License, Version 2.0.
// SPDX: Apache-2.0

// This file has been modified from the original, original notice is above.

#pragma once

#include "main/loquat.h"
#include "pbr/base/light.h"
#include "pbr/base/light_sampler.h"

namespace loquat
{

	class UniformLightSampler
	{

	};

	class PowerLightSampler
	{
	public:
		PowerLightSampler(std::span<const Light> lights, Allocator allocator)
			noexcept;
	};

	class BVHLightSampler
	{

	};

	class ExhaustiveLightSampler
	{

	};
}