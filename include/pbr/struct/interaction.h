// pbrt is Copyright(c) 1998-2020 Matt Pharr, Wenzel Jakob, and Greg Humphreys.
// The pbrt source code is licensed under the Apache License, Version 2.0.
// SPDX: Apache-2.0

// This file has been modified from the original, original notice is above.

#pragma once

#include "main/loquat.h"
#include "pbr/base/medium.h"

namespace loquat
{
	class SurfaceInteraction;
	class MediumInteraction;

	class Interaction
	{
	public:
		Interaction() = default;
		Interaction(Point3fi point, Normal3f normal, Point2f uv,
			Vec3f outgoing, Float time)
			: point{ point }
			, normal{ normal }
			, uv{ uv }
			, outgoing{ glm::normalize(outgoing) }
			, time{ time }
		{}

		[[nodiscard]]
		Point3f p() const noexcept
		{
			return point.to_vec();
		}

		[[nodiscard]]
		bool is_surface_interaction() const noexcept
		{
			return normal != Normal3f{ 0, 0, 0 };
		}

		[[nodiscard]]
		bool is_medium_interaction() const noexcept
		{
			return !is_surface_interaction();
		}

		[[nodiscard]]
		const SurfaceInteraction& as_surface() const noexcept
		{
			LOG_ASSERT(is_surface_interaction()
				&& "Trying to cast a non-surface interaction to surface");
			return (const SurfaceInteraction&) *this;
		}

		[[nodiscard]]
		const MediumInteraction& as_medium() const noexcept
		{
			LOG_ASSERT(is_medium_interaction()
				&& "Trying to cast a non-medium interaction to medium");
			return (const MediumInteraction&)*this;
		}

		std::string to_string() const noexcept;

		[[nodiscard]]
		Point3f offset_ray_origin(Vec3f direction) const noexcept
		{
			return loquat::offset_ray_origin(point, normal, direction);
		}

		[[nodiscard]]
		Point3f offset_ray_origin_dest(Point3f destination) const noexcept
		{
			return offset_ray_origin(Vec3f(destination) - p());
		}

		[[nodiscard]]
		RayDifferential spawn_ray(Vec3f direction) const noexcept
		{
			return RayDifferential(offset_ray_origin(direction), direction, 
				time, get_medium(direction));
		}

		[[nodiscard]]
		Ray3f spawn_ray_to(Point3f destination) const noexcept
		{
			Ray3f ray = loquat::spawn_ray_to(point, normal, time, destination);
			ray.medium = get_medium(ray.direction);
			return ray;
		}
		
		[[nodiscard]]
		Ray3f spawn_ray_to(const Interaction& target) const noexcept
		{
			Ray3f ray = loquat::spawn_ray_to(point, normal, time, 
				target.point, target.normal);
			ray.medium = get_medium(ray.direction);
			return ray;
		}
		
		[[nodiscard]]
		Medium get_medium(Vec3f direction) const noexcept
		{
			if (medium_interface)
			{
				if (glm::dot(direction, normal) > 0)
				{
					return medium_interface->outside;
				}
				return medium_interface->inside;
			}
			return medium;
		}

		[[nodiscard]]
		Medium get_medium() const {
			if (medium_interface)
			{
				LOG_ASSERT(medium_interface->inside == 
					medium_interface->outside);
				return medium_interface->inside;
			}
			return medium;
		}

		Point3fi point;
		Float time = 0;
		Vec3f outgoing;
		Normal3f normal;
		Point2f uv;
		const MediumInterface* medium_interface = nullptr;
		Medium medium = nullptr;
	};

	class SurfaceInteraction : Interaction
	{

	};

	class MediumInteraction : Interaction
	{

	};

}