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
		Point2f sample;
		Float weight;
	};

	class FilterSampler
	{
	public:
		FilterSampler(Filter filter, Allocator alloc = {});

		[[nodiscard]]
		std::string to_string() const noexcept;

		FilterSample sample(Point2f sample_2d) const noexcept
		{
			Float pdf;
			Point2i intersection;
			Point2f sample = distribution.sample(sample_2d, &pdf, &intersection);
			return FilterSample{ sample, filter_function_values[intersection] / pdf };
		}

	private:
		AABB2f domain;
		Array2D<Float> filter_function_values;
		PiecewiseConstant2D distribution;
	};

	//TODO(ches) complete this
}