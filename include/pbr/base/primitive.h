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