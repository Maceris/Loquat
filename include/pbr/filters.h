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

		LOQUAT_CPU_GPU
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

	class BoxFilter {
	public:
		BoxFilter(Vec2f radius = Vec2f(0.5, 0.5))
			: radius(radius)
		{}

		static BoxFilter* create(const ParameterDictionary& parameters,
			const FileLoc* loc, Allocator allocator);

		LOQUAT_CPU_GPU
		Vec2f get_radius() const
		{
			return radius;
		}

		std::string to_string() const;

		LOQUAT_CPU_GPU
		Float evaluate(Point2f p) const
		{
			return (std::abs(p.x) <= radius.x && std::abs(p.y) <= radius.y) ? 1 : 0;
		}

		LOQUAT_CPU_GPU
		FilterSample sample(Point2f u) const
		{
			Point2f p(lerp(u[0], -radius.x, radius.x), 
				lerp(u[1], -radius.y, radius.y));
			return { p, Float(1) };
		}

		LOQUAT_CPU_GPU
		Float Integral() const
		{
			return 2 * radius.x * 2 * radius.y;
		}

	private:
		Vec2f radius;
	};

	//TODO(ches) complete this
}