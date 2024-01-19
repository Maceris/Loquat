// pbrt is Copyright(c) 1998-2020 Matt Pharr, Wenzel Jakob, and Greg Humphreys.
// The pbrt source code is licensed under the Apache License, Version 2.0.
// SPDX: Apache-2.0

// This file has been modified from the original, original notice is above.

#pragma once

#include "main/loquat.h"

namespace loquat
{
	class SimplePrimitive;
	class GeometricPrimitive;
	class TransformedPrimitive;
	class AnimatedPrimitive;

	/// <summary>
	/// Bounding Volume Hiearchy aggregate.
	/// </summary>
	class BVHAggregate;
	class KdTreeAggregate;

	class Primitive : public TaggedPointer<SimplePrimitive, GeometricPrimitive,
		TransformedPrimitive, AnimatedPrimitive, BVHAggregate, KdTreeAggregate>
	{
	public:
		// Primitive Interface
		using TaggedPointer::TaggedPointer;

		[[nodiscard]]
		AABB3f bounds() const noexcept;

		[[nodiscard]]
		std::optional<ShapeIntersection> intersection(const Ray3f& r,
			Float tMax = FLOAT_INFINITY) const noexcept;

		[[nodiscard]]
		bool has_intersection(const Ray3f& r, Float tMax = FLOAT_INFINITY)
			const noexcept;
	};
}