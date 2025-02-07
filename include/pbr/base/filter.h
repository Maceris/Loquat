// pbrt is Copyright(c) 1998-2020 Matt Pharr, Wenzel Jakob, and Greg Humphreys.
// The pbrt source code is licensed under the Apache License, Version 2.0.
// SPDX: Apache-2.0

// This file has been modified from the original, original notice is above.

#pragma once

#include <string_view>

#include "main/loquat.h"

#include "pbr/util/tagged_pointer.h"

namespace loquat
{
	struct FilterSample;
	class BoxFilter;
	class GaussianFilter;
	class MitchellFilter;
	class LanczosSincFilter;
	class TriangleFilter;

	class Filter : public TaggedPointer<BoxFilter, GaussianFilter,
		MitchellFilter, LanczosSincFilter, TriangleFilter>
	{
	public:
		using TaggedPointer::TaggedPointer;

		static Filter create(std::string_view name,
			const ParameterDictionary& parameters, Allocator allocators)
			noexcept;

		LOQUAT_CPU_GPU
		inline Vec2f get_radius() const;

		LOQUAT_CPU_GPU
		inline Float evaluate(Point2f point) const;

		LOQUAT_CPU_GPU
		inline Float integral() const;

		LOQUAT_CPU_GPU
		inline FilterSample sample(Point2f sample_2D) const;

		[[nodiscard]]
		std::string to_string() const;
	};

}