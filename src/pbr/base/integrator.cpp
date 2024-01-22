// pbrt is Copyright(c) 1998-2020 Matt Pharr, Wenzel Jakob, and Greg Humphreys.
// The pbrt source code is licensed under the Apache License, Version 2.0.
// SPDX: Apache-2.0

// This file has been modified from the original, original notice is above.

#include "pbr/shapes.h"
#include "pbr/base/integrator.h"
#include "pbr/base/spectrum.h"
#include "pbr/math/ray.h"

namespace loquat
{
	[[nodiscard]]
	SampledSpectrum RandomWalkIntegrator::light_incoming_random_walk(
		RayDifferential ray,
		SampledWavelengths& lambda, Sampler sampler,
		ScratchBuffer& scratch_buffer, int depth) const noexcept
	{
		std::optional<ShapeIntersection> intersection = intersect(ray);

		if (!intersection)
		{
			SampledSpectrum result{ 0.0f };
			for (Light light : infinite_lights)
			{
				result += light.infinite_light_contribution(ray, lambda);
			}
			return result;
		}

		SurfaceInteraction& surface = intersection->interaction;

		Vec3f outgoing_direction = -ray.direction;
		SampledSpectrum emitted = surface.emitted_radiance(
			outgoing_direction, lambda);

		if (depth == max_depth)
		{
			return emitted;
		}

		BSDF bsdf = surface.get_BSDF(ray, lambda, camera, scratch_buffer,
			sampler);

		if (!bsdf)
		{
			return emitted;
		}

		Point2f u = sampler.get_2D();
		Vec3f wp = sample_uniform_sphere(u);

		SampledSpectrum fcos = bsdf.f(outgoing_direction, wp) *
			absolute_dot(wp, surface.shading.normal);

		if (!fcos)
		{
			return emitted;
		}

		ray = surface.spawn_ray(wp);
		return emitted + fcos * light_incoming_random_walk(ray, lambda,
			sampler, scratch_buffer, depth + 1) /
			(1 / (4 * PI));
	}
}