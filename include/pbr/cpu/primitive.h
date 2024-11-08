// pbrt is Copyright(c) 1998-2020 Matt Pharr, Wenzel Jakob, and Greg Humphreys.
// The pbrt source code is licensed under the Apache License, Version 2.0.
// SPDX: Apache-2.0

// This file has been modified from the original, original notice is above.

#pragma once

#include "main/loquat.h"

#include "pbr/base/light.h"
#include "pbr/base/shape.h"
#include "pbr/base/material.h"
#include "pbr/math/transform.h"

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
		std::optional<ShapeIntersection> intersection(const Ray& ray,
			Float t_max = FLOAT_INFINITY) const noexcept;

		[[nodiscard]]
		bool has_intersection(const Ray& r, Float t_max = FLOAT_INFINITY)
			const noexcept;
	};

	class GeometricPrimitive
	{
	public:
		GeometricPrimitive(Shape shape, Material material, Light area_light,
			const MediumInterface& medium_interface,
			FloatTexture alpha = nullptr) noexcept;

		[[nodiscard]]
		AABB3f bounds() const noexcept;

		[[nodiscard]]
		std::optional<ShapeIntersection> intersect(const Ray& ray, Float t_max)
			const noexcept;

		[[nodiscard]]
		bool has_intersection(const Ray& r, Float tMax) const noexcept;

	private:
		Shape shape;
		Material material;
		Light area_light;
		MediumInterface medium_interface;
		FloatTexture alpha;
	};

	class SimplePrimitive
	{
	public:

		SimplePrimitive(Shape shape, Material material) noexcept;

		[[nodiscard]]
		AABB3f bounds() const noexcept;

		[[nodiscard]]
		std::optional<ShapeIntersection> intersect(const Ray& ray, Float t_max)
			const noexcept;

		[[nodiscard]]
		bool has_intersection(const Ray& r, Float tMax) const noexcept;

	private:
		Shape shape;
		Material material;
	};

	class TransformedPrimitive
	{
	public:
		TransformedPrimitive(Primitive primitive,
			const Transform* render_from_primitive) noexcept
			: primitive{ primitive }
			, render_from_primitive{ render_from_primitive }
		{}


		[[nodiscard]]
		AABB3f bounds() const noexcept
		{
			return (*render_from_primitive)(primitive.bounds());
		}

		[[nodiscard]]
		std::optional<ShapeIntersection> intersect(const Ray& ray, Float t_max)
			const noexcept;

		[[nodiscard]]
		bool has_intersection(const Ray& r, Float tMax) const noexcept;


	private:
		Primitive primitive;
		const Transform* render_from_primitive;
	};

	class AnimatedPrimitive
	{
	public:

		AnimatedPrimitive(Primitive primitive,
			const AnimatedTransform& render_from_primitive) noexcept;

		[[nodiscard]]
		AABB3f bounds() const noexcept
		{
			return render_from_primitive.motion_bounds(primitive.bounds());
		}

		[[nodiscard]]
		std::optional<ShapeIntersection> intersect(const Ray& ray, Float t_max)
			const noexcept;

		[[nodiscard]]
		bool has_intersection(const Ray& r, Float tMax) const noexcept;

	private:
		Primitive primitive;
		AnimatedTransform render_from_primitive;
	};
}