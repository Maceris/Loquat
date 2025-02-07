// pbrt is Copyright(c) 1998-2020 Matt Pharr, Wenzel Jakob, and Greg Humphreys.
// The pbrt source code is licensed under the Apache License, Version 2.0.
// SPDX: Apache-2.0

// This file has been modified from the original, original notice is above.

#pragma once

#include "main/loquat.h"
#include "pbr/bsdf.h"
#include "pbr/base/camera.h"
#include "pbr/base/light.h"
#include "pbr/base/material.h"
#include "pbr/base/medium.h"
#include "pbr/base/sampler.h"
#include "pbr/math/ray.h"
#include "pbr/math/vector_math.h"

namespace loquat
{
	class Interaction
	{
	public:
		Interaction() = default;

		LOQUAT_CPU_GPU
		Interaction(Point3fi point, Normal3f normal, Point2f uv,
			Vec3f outgoing, Float time)
			: point{ point }
			, normal{ normal }
			, uv{ uv }
			, outgoing{ normalize(outgoing) }
			, time{ time }
		{}

		[[nodiscard]]
		LOQUAT_CPU_GPU
		Point3f p() const
		{
			return point.to_vec();
		}

		[[nodiscard]]
		LOQUAT_CPU_GPU
		bool is_surface_interaction() const
		{
			return normal != Normal3f{ 0, 0, 0 };
		}

		[[nodiscard]]
		LOQUAT_CPU_GPU
		bool is_medium_interaction() const
		{
			return !is_surface_interaction();
		}

		[[nodiscard]]
		LOQUAT_CPU_GPU
		const SurfaceInteraction& as_surface() const
		{
			LOG_ASSERT(is_surface_interaction()
				&& "Trying to cast a non-surface interaction to surface");
			return (const SurfaceInteraction&) *this;
		}
		
		[[nodiscard]]
		LOQUAT_CPU_GPU
		SurfaceInteraction& as_surface()
		{
			LOG_ASSERT(is_surface_interaction()
				&& "Trying to cast a non-surface interaction to surface");
			return (SurfaceInteraction&)*this;
		}

		LOQUAT_CPU_GPU
		Interaction(Point3f point, Vec3f outgoing, Float time, Medium medium)
			: point{ point }
			, time{ time }
			, outgoing{ outgoing }
			, medium{ medium }
		{}

		LOQUAT_CPU_GPU
		Interaction(Point3f point, Point2f uv)
			: point{ point }
			, uv{ uv }
		{}

		LOQUAT_CPU_GPU
		Interaction(const Point3fi& point, Normal3f normal, Float time = 0,
			Point2f uv = {})
			: point{ point }
			, normal{ normal }
			, uv{ uv }
			, time{ time }
		{}

		LOQUAT_CPU_GPU
		Interaction(const Point3fi& point, Normal3f normal, Point2f uv)
			: point{ point }
			, time{ time }
			, uv{ uv }
		{}

		LOQUAT_CPU_GPU
		Interaction(Point3f point, Float time, Medium medium)
			: point{ point }
			, time{ time }
			, medium{ medium }
		{}

		LOQUAT_CPU_GPU
		Interaction(Point3f point, const MediumInterface* medium_interface)
			: point{ point }
			, medium_interface{ medium_interface }
		{}

		[[nodiscard]]
		LOQUAT_CPU_GPU
		const MediumInteraction& as_medium() const
		{
			LOG_ASSERT(is_medium_interaction()
				&& "Trying to cast a non-medium interaction to medium");
			return (const MediumInteraction&)*this;
		}

		[[nodiscard]]
		LOQUAT_CPU_GPU
		MediumInteraction& as_medium()
		{
			LOG_ASSERT(is_medium_interaction()
				&& "Trying to cast a non-medium interaction to medium");
			return (MediumInteraction&)*this;
		}

		[[nodiscard]]
		std::string to_string() const;

		[[nodiscard]]
		LOQUAT_CPU_GPU
		Point3f offset_ray_origin(Vec3f direction) const
		{
			return loquat::offset_ray_origin(point, normal, direction);
		}

		[[nodiscard]]
		LOQUAT_CPU_GPU
		Point3f offset_ray_origin_dest(Point3f destination) const
		{
			return offset_ray_origin(Vec3f(destination) - p());
		}

		[[nodiscard]]
		LOQUAT_CPU_GPU
		RayDifferential spawn_ray(Vec3f direction) const
		{
			return RayDifferential(offset_ray_origin(direction), direction, 
				time, get_medium(direction));
		}

		[[nodiscard]]
		LOQUAT_CPU_GPU
		Ray spawn_ray_to(Point3f destination) const
		{
			Ray ray = loquat::spawn_ray_to(point, normal, time, destination);
			ray.medium = get_medium(ray.direction);
			return ray;
		}
		
		[[nodiscard]]
		LOQUAT_CPU_GPU
		Ray spawn_ray_to(const Interaction& target) const
		{
			Ray ray = loquat::spawn_ray_to(point, normal, time, 
				target.point, target.normal);
			ray.medium = get_medium(ray.direction);
			return ray;
		}
		
		[[nodiscard]]
		LOQUAT_CPU_GPU
		Medium get_medium(Vec3f direction) const
		{
			if (medium_interface)
			{
				if (dot(direction, normal) > 0)
				{
					return medium_interface->outside;
				}
				return medium_interface->inside;
			}
			return medium;
		}

		[[nodiscard]]
		LOQUAT_CPU_GPU
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

	class SurfaceInteraction : public Interaction
	{
	public:
		SurfaceInteraction() = default;

		LOQUAT_CPU_GPU
		SurfaceInteraction(Point3fi point, Point2f uv, Vec3f outgoing,
			Vec3f dpdu, Vec3f dpdv, Normal3f dndu, Normal3f dndv, Float time,
			bool flip_normal)
			: Interaction{ point,
				Normal3f(glm::normalize(cross(dpdu, dpdv))), uv, outgoing,
				time }
			, dpdu{ dpdu }
			, dpdv{ dpdv }
			, dndu{ dndu }
			, dndv{ dndv }
		{
			shading.normal = normal;
			shading.dpdu = dpdu;
			shading.dpdv = dpdv;
			shading.dndu = dndu;
			shading.dndv = dndv;

			if (flip_normal)
			{
				normal = -normal;
				shading.normal = -shading.normal;
			}
		}

		LOQUAT_CPU_GPU
		SurfaceInteraction(Point3fi point, Point2f uv, Vec3f outgoing,
			Vec3f dpdu, Vec3f dpdv, Normal3f dndu, Normal3f dndv, Float time,
			bool flip_normal, int face_index)
			: SurfaceInteraction{ point, uv, outgoing, dpdu, dpdv, dndu, dndv,
				time, flip_normal }
		{
			this->face_index = face_index;
		}

		LOQUAT_CPU_GPU
		void set_shading_geometry(Normal3f normal, Vec3f dpdu, Vec3f dpdv,
			Normal3f dndu, Normal3f dndv, bool is_orientation_authoritative)
			noexcept
		{
			shading.normal = normal;
			LOG_ASSERT(shading.normal != Normal3f{ 0 }
				&& "Shading normal zero");
			if (is_orientation_authoritative)
			{
				this->normal = face_forward(normal, shading.normal);
			}
			else
			{
				shading.normal = face_forward(shading.normal, normal);
			}

			shading.dpdu = dpdu;
			shading.dpdv = dpdv;
			shading.dndu = dndu;
			shading.dndv = dndv;
			while (length_squared(shading.dpdu) > 1e16f
				|| length_squared(shading.dpdv) > 1e16f)
			{
				shading.dpdu /= 1e8f;
				shading.dpdv /= 1e8f;
			}
		}

		[[nodiscard]]
		std::string to_string() const;

		void set_intersection_properties(Material material, Light area,
			const MediumInterface* primary_medium_interface, Medium ray_medium)
		{
			this->material = material;
			area_light = area;
			LOG_ASSERT(dot(normal, shading.normal) >= 0
				&& "Shading normal perpendicular to normal");
			if (primary_medium_interface 
				&& primary_medium_interface->is_medium_transition())
			{
				medium_interface = primary_medium_interface;
			}
			else
			{
				medium = ray_medium;
			}
		}

		LOQUAT_CPU_GPU
		void compute_differentials(const RayDifferential& ray, Camera camera,
			int samples_per_pixel);

		LOQUAT_CPU_GPU
		void skip_intersection(RayDifferential* ray, Float t) const;

		using Interaction::spawn_ray;

		LOQUAT_CPU_GPU
		RayDifferential spawn_ray(const RayDifferential& ray,
			const BSDF& bsdf, Vec3f incoming, int flags, Float eta)
			const;

		BSDF get_BSDF(const RayDifferential& ray, SampledWavelengths& lambda,
			Camera camera, ScratchBuffer& scratch_buffer, Sampler sampler);

		BSSRDF get_BSSRDF(const RayDifferential& ray,
			SampledWavelengths& lambda, Camera camera,
			ScratchBuffer& scratch_buffer, Sampler sampler);

		LOQUAT_CPU_GPU
		SampledSpectrum emitted_radiance(Vec3f direction, 
			const SampledWavelengths& lambda) const;

		Vec3f dpdu;
		Vec3f dpdv;
		Normal3f dndu;
		Normal3f dndv;

		struct {
			Normal3f normal;
			Vec3f dpdu;
			Vec3f dpdv;
			Normal3f dndu;
			Normal3f dndv;
		} shading;

		int face_index = 0;
		Material material;
		Light area_light;
		Vec3f dpdx;
		Vec3f dpdy;
		Float dudx = 0;
		Float dvdx = 0;
		Float dudy = 0;
		Float dvdy = 0;
	};

	class MediumInteraction : public Interaction
	{
	public:
		LOQUAT_CPU_GPU
		MediumInteraction()
			: phase{ nullptr }
		{}

		LOQUAT_CPU_GPU
		MediumInteraction(Point3f point, Vec3f outgoing, Float time,
			Medium medium, PhaseFunction phase)
			: Interaction{ point, outgoing, time, medium }
			, phase{ phase }
		{}

		[[nodiscard]]
		std::string to_string() const;

		PhaseFunction phase;
	};

}