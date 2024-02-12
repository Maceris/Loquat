// pbrt is Copyright(c) 1998-2020 Matt Pharr, Wenzel Jakob, and Greg Humphreys.
// The pbrt source code is licensed under the Apache License, Version 2.0.
// SPDX: Apache-2.0

// This file has been modified from the original, original notice is above.

#pragma once

#include "main/loquat.h"
#include "pbr/base/texture.h"
#include "pbr/math/vector_math.h"
#include "pbr/util/tagged_pointer.h"

#include <map>
#include <string>
#include <string_view>

namespace loquat
{
	class Triangle;
	class BilinearPatch;
	class Curve;
	class Sphere;
	class Cylinder;
	class Disk;

	struct ShapeSample;
	struct ShapeIntersection;
	struct ShapeSampleContext;

	class Shape : public TaggedPointer<Sphere, Cylinder, Disk, Triangle,
		BilinearPatch, Curve>
	{
	public:
		using TaggedPointer::TaggedPointer;

		static std::vector<Shape> create(std::string_view name,
			const Transform* render_from_object,
			const Transform* object_from_render, bool reverse_orientation,
			const ParameterDictionary& parameters,
			std::map<std::string, FloatTexture>& float_textures,
			Allocator allocator);

		[[nodiscard]]
		std::string to_string() const noexcept;

		inline AABB3f bounds() const noexcept;

		inline DirectionCone normal_bounds() const noexcept;

		inline std::optional<ShapeIntersection> intersect(const Ray& ray,
			Float t_max = FLOAT_INFINITY) const noexcept;

		inline bool has_intersection(const Ray& ray,
			Float t_max = FLOAT_INFINITY) const noexcept;

		inline Float area() const noexcept;

		inline std::optional<ShapeSample> sample(Point2f sample_2D)
			const noexcept;

		inline float PDF(const Interaction& interaction) const noexcept;

		std::optional<ShapeSample> sample(const ShapeSampleContext& context,
			Point2f sample_2D) const noexcept;

		inline Float PDf(const ShapeSampleContext& context,
			Vec3f incident_direction) const noexcept;

	};

}