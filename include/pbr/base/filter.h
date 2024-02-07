// pbrt is Copyright(c) 1998-2020 Matt Pharr, Wenzel Jakob, and Greg Humphreys.
// The pbrt source code is licensed under the Apache License, Version 2.0.
// SPDX: Apache-2.0

// This file has been modified from the original, original notice is above.

#pragma once

#include "main/loquat.h"

#include <string_view>


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

		inline Vec2f radius() const noexcept;

		inline Float evaluate(Point2f point) const noexcept;

		inline Float integral() const noexcept;

		inline FilterSample sample(Point2f sample_2D) const noexcept;

		[[nodiscard]]
		std::string to_string() const noexcept;
	};

}