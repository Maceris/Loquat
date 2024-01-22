// pbrt is Copyright(c) 1998-2020 Matt Pharr, Wenzel Jakob, and Greg Humphreys.
// The pbrt source code is licensed under the Apache License, Version 2.0.
// SPDX: Apache-2.0

// This file has been modified from the original, original notice is above.

#pragma once

#include <optional>
#include <string>
#include <vector>

#include "pbr/base/camera.h"
#include "pbr/base/film.h"
#include "pbr/base/light.h"
#include "pbr/base/primitive.h"
#include "pbr/base/sampler.h"
#include "pbr/base/spectrum.h"
#include "pbr/math/math.h"
#include "pbr/math/sampling.h"
#include "pbr/struct/interaction.h"
#include "pbr/struct/parameter_dictionary.h"
#include "pbr/util/color_space.h"

namespace loquat
{
	/// <summary>
	/// Responsible for rendering a scene, calculating the results of 
	/// the rendering equation.
	/// </summary>
	class Integrator
	{
	public:
		virtual ~Integrator();

		static std::unique_ptr<Integrator> create(
		std::string_view name, const ParameterDictionary& parameters,
			Camera camera, Sampler sampler, Primitive aggregate,
			std::vector<Light> lights, const RGBColorSpace* color_space
		);

		[[nodiscard]]
		bool has_intersection(const Ray3f& ray,
			Float t_max = FLOAT_INFINITY) const noexcept;
		
		[[nodiscard]]
		std::optional<ShapeIntersection> intersect(const Ray3f& ray,
			Float t_max = FLOAT_INFINITY) const noexcept;

		[[nodiscard]]
		bool unoccluded(const Interaction& p0, const Interaction& p1)
			const noexcept
		{
			return !has_intersection(p0.spawn_ray_to(p1), 1 - SHADOW_EPSILON);
		}

		SampledSpectrum transmittance(const Interaction& p0,
			const Interaction& p1, const SampledWavelengths& lambda) 
			const noexcept;

		virtual void render() = 0;
		virtual std::string to_string() const noexcept = 0;

		Primitive aggregate;
		std::vector<Light> lights;
		std::vector<Light> infinite_lights;

	protected:
		Integrator(Primitive aggregate, std::vector<Light> lights)
			: aggregate{ aggregate }
			, lights{ lights }
		{
			AABB3f scene_bounds = aggregate ? aggregate.bounds() : AABB3f();
			LOG_INFO(std::format("Scene bounds are {}", scene_bounds));
			for (auto& light : lights)
			{
				light.preprocess(scene_bounds);
				if (light.type() == LightType::Infinite)
				{
					infinite_lights.push_back(light);
				}
			}
		}
	};

	class ImageTileIntegrator : public Integrator
	{
	public:
		ImageTileIntegrator(Camera camera, Sampler sampler,
			Primitive aggregate, std::vector<Light> lights)
			: Integrator{ aggregate, lights }
			, camera{ camera }
			, sampler_prototype{ sampler }
		{}

		void render();

		virtual void evaulate_pixel_sample(Point2i pixel, int sample_index,
			Sampler sampler, ScratchBuffer& scratch_buffer) = 0;

	protected:
		Camera camera;
		Sampler sampler_prototype;
	};

	class RayIntegrator : public ImageTileIntegrator
	{
	public:
		RayIntegrator(Camera camera, Sampler sampler, Primitive aggregate,
			std::vector<Light> lights)
			: ImageTileIntegrator{ camera, sampler, aggregate, lights }
		{}
		
		void evaulate_pixel_sample(Point2i pixel, int sample_index,
			Sampler sampler, ScratchBuffer& scratch_buffer) final;

		virtual SampledSpectrum light_incoming(RayDifferential ray,
			SampledWavelengths& lambda, Sampler sampler,
			ScratchBuffer& scratch_buffer, VisibleSurface* visible_surface)
			const = 0;
	};

	class RandomWalkIntegrator : public RayIntegrator
	{
		RandomWalkIntegrator(int max_depth, Camera camera, Sampler sampler, 
			Primitive aggregate, std::vector<Light> lights)
			: RayIntegrator{ camera, sampler, aggregate,lights }
			, max_depth{ max_depth }
		{}

		static std::unique_ptr<RandomWalkIntegrator> create(
			const ParameterDictionary& parameters, Camera camera,
			Sampler sampler, Primitive aggregate, std::vector<Light> lights);

		std::string to_string();

		SampledSpectrum light_incoming(RayDifferential ray,
			SampledWavelengths& lambda, Sampler sampler,
			ScratchBuffer& scratch_buffer, VisibleSurface* visible_surface)
			const
		{
			return light_incoming_random_walk(ray, lambda, sampler,
				scratch_buffer, 0);
		}

	private:

		SampledSpectrum light_incoming_random_walk(RayDifferential ray,
			SampledWavelengths& lambda, Sampler sampler,
			ScratchBuffer& scratch_buffer, int depth) const
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
		int max_depth;
	};

	class SimplePathIntegrator : public RayIntegrator
	{

	};

	class PathIntegrator : public RayIntegrator
	{

	};

	class SimpleVolumePathIntegrator : public RayIntegrator
	{

	};

	/// <summary>
	/// Based on null-scattering path integral of Miller et al. 2019,
	/// https://cs.dartmouth.edu/~wjarosz/publications/miller19null.html
	/// </summary>
	class VolumePathIntegrator : public RayIntegrator
	{

	};

	class AOIntegrator : public RayIntegrator
	{

	};

	class LightPathIntegrator : public ImageTileIntegrator
	{

	};

	struct Vertex;

	class BDPTIntegrator : public RayIntegrator
	{

	};

	class MLTSampler;

	class MLTIntegrator : public Integrator
	{

	};

	class SPPMIntegrator : public Integrator
	{

	};

	class FunctionIntegrator : public Integrator
	{

	};

}