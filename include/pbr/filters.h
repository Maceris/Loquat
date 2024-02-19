// pbrt is Copyright(c) 1998-2020 Matt Pharr, Wenzel Jakob, and Greg Humphreys.
// The pbrt source code is licensed under the Apache License, Version 2.0.
// SPDX: Apache-2.0

// This file has been modified from the original, original notice is above.

#pragma once

#include "main/loquat.h"

#include "pbr/base/filter.h"
#include "pbr/math/math.h"
#include "pbr/struct/containers.h"
#include "pbr/util/sampling.h"

namespace loquat
{
	struct FilterSample
	{
		Point2f p;
		Float weight;
	};

	class FilterSampler
	{
	public:

	private:
		AABB2f domain;
		Array2D<Float> f;
		PiecewiseConstant2D distribution;
	};

	//TODO(ches) complete this
}