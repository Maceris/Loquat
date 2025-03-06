// pbrt is Copyright(c) 1998-2020 Matt Pharr, Wenzel Jakob, and Greg Humphreys.
// The pbrt source code is licensed under the Apache License, Version 2.0.
// SPDX: Apache-2.0

// This file has been modified from the original, original notice is above.

#pragma once

#include <functional>
#include <format>
#include <optional>
#include <memory>
#include <string>
#include <vector>

#include "main/loquat.h"
#include "pbr/light_samplers.h"
#include "pbr/base/camera.h"
#include "pbr/base/film.h"
#include "pbr/base/light.h"
#include "pbr/base/sampler.h"
#include "pbr/cpu/primitive.h"
#include "pbr/math/math.h"
#include "pbr/math/sampling.h"
#include "pbr/struct/interaction.h"
#include "pbr/struct/parameter_dictionary.h"
#include "pbr/util/color_space.h"
#include "pbr/util/spectrum.h"

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
			std::vector<Light> lights, const RGBColorSpace* color_space,
			const FileLoc* loc
		);

		[[nodiscard]]
		bool has_intersection(const Ray& ray,
			Float t_max = FLOAT_INFINITY) const;
		
		[[nodiscard]]
		pstd::optional<ShapeIntersection> intersect(const Ray& ray,
			Float t_max = FLOAT_INFINITY) const;

		[[nodiscard]]
		bool unoccluded(const Interaction& p0, const Interaction& p1)
			const
		{
			return !has_intersection(p0.spawn_ray_to(p1), 1 - SHADOW_EPSILON);
		}

		[[nodiscard]]
		SampledSpectrum transmittance(const Interaction& p0,
			const Interaction& p1, const SampledWavelengths& lambda) 
			const;

		virtual void render() = 0;
		virtual std::string to_string() const = 0;

		Primitive aggregate;
		std::vector<Light> lights;
		std::vector<Light> infinite_lights;

	protected:
		Integrator(Primitive aggregate, std::vector<Light> lights)
			: aggregate{ aggregate }
			, lights{ lights }
		{
			AABB3f scene_bounds = aggregate ? aggregate.bounds() : AABB3f();
			LOG_INFO(std::format("Scene bounds are {}", 
				scene_bounds.to_string()));
			for (auto& light : lights)
			{
				light.preprocess(scene_bounds);
				if (light.get_type() == LightType::Infinite)
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

		virtual void evaluate_pixel_sample(Point2i pixel, int sample_index,
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
		
		void evaluate_pixel_sample(Point2i pixel, int sample_index,
			Sampler sampler, ScratchBuffer& scratch_buffer) final;

		[[nodiscard]]
		virtual SampledSpectrum light_incoming(RayDifferential ray,
			SampledWavelengths& lambda, Sampler sampler,
			ScratchBuffer& scratch_buffer, VisibleSurface* visible_surface)
			const = 0;
	};

	class RandomWalkIntegrator : public RayIntegrator
	{
	public:
		RandomWalkIntegrator(int max_depth, Camera camera, Sampler sampler, 
			Primitive aggregate, std::vector<Light> lights)
			: RayIntegrator{ camera, sampler, aggregate,lights }
			, max_depth{ max_depth }
		{}

		[[nodiscard]]
		static std::unique_ptr<RandomWalkIntegrator> create(
			const ParameterDictionary& parameters, Camera camera,
			Sampler sampler, Primitive aggregate, std::vector<Light> lights,
			const FileLoc* loc);

		[[nodiscard]]
		std::string to_string() const;

		[[nodiscard]]
		SampledSpectrum light_incoming(RayDifferential ray,
			SampledWavelengths& lambda, Sampler sampler,
			ScratchBuffer& scratch_buffer, VisibleSurface* visible_surface)
			const
		{
			return light_incoming_random_walk(ray, lambda, sampler,
				scratch_buffer, 0);
		}

	private:

		[[nodiscard]]
		SampledSpectrum light_incoming_random_walk(RayDifferential ray,
			SampledWavelengths& lambda, Sampler sampler,
			ScratchBuffer& scratch_buffer, int depth) const
		{
			pstd::optional<ShapeIntersection> intersection = intersect(ray);

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
	public:
		SimplePathIntegrator(int max_depth, bool sample_lights,
			bool sample_BSDF, Camera camera, Sampler sampler,
			Primitive aggregate, std::vector<Light> lights);

		[[nodiscard]]
		SampledSpectrum light_incoming(RayDifferential ray,
			SampledWavelengths& lambda, Sampler sampler,
			ScratchBuffer& scratch_buffer, VisibleSurface* visible_surface)
			const;

		[[nodiscard]]
		static std::unique_ptr<SimplePathIntegrator> create(
			const ParameterDictionary& parameters, Camera camera,
			Sampler sampler, Primitive aggregate, std::vector<Light> lights,
			const FileLoc* loc)
			;

		[[nodiscard]]
		std::string to_string() const;

	private:
		int max_depth;
		bool sample_lights;
		bool sample_BSDF;
		UniformLightSampler light_sampler;
	};

	class PathIntegrator : public RayIntegrator
	{
	public:
		PathIntegrator(int max_depth, Camera camera, Sampler sampler,
			Primitive aggregate, std::vector<Light> lights,
			std::string_view light_sample_strategy = "bvh",
			bool regularize = false);

		[[nodiscard]]
		SampledSpectrum light_incoming(RayDifferential ray,
			SampledWavelengths& lambda, Sampler sampler,
			ScratchBuffer& scratch_buffer, VisibleSurface* visible_surface)
			const;

		[[nodiscard]]
		static std::unique_ptr<PathIntegrator> create(
			const ParameterDictionary& parameters, Camera camera,
			Sampler sampler, Primitive aggregate, std::vector<Light> lights,
			const FileLoc* loc);

		[[nodiscard]]
		std::string to_string() const;

	private:
		[[nodiscard]]
		SampledSpectrum light_direct(const SurfaceInteraction& interaction,
			const BSDF* bsdf, SampledWavelengths& lambda, Sampler sampler)
			const;

		int max_depth;
		LightSampler light_sampler;
		bool regularize;
	};

	class SimpleVolumePathIntegrator : public RayIntegrator
	{
	public:
		SimpleVolumePathIntegrator(int max_depth, Camera camera,
			Sampler sampler, Primitive aggregate, std::vector<Light> lights);

		[[nodiscard]]
		SampledSpectrum light_incoming(RayDifferential ray,
			SampledWavelengths& lambda, Sampler sampler,
			ScratchBuffer& scratch_buffer, VisibleSurface* visible_surface)
			const;

		[[nodiscard]]
		static std::unique_ptr<SimpleVolumePathIntegrator> create(
			const ParameterDictionary& parameters, Camera camera,
			Sampler sampler, Primitive aggregate, std::vector<Light> lights,
			const FileLoc* loc);

		[[nodiscard]]
		std::string to_string() const;

	private:
		int max_depth;
	};

	/// <summary>
	/// Based on null-scattering path integral of Miller et al. 2019,
	/// https://cs.dartmouth.edu/~wjarosz/publications/miller19null.html
	/// </summary>
	class VolumePathIntegrator : public RayIntegrator
	{
	public:
		VolumePathIntegrator(int max_depth, Camera camera,
			Sampler sampler, Primitive aggregate, std::vector<Light> lights,
			std::string_view light_sample_strategy = "bvh",
			bool regularize = false)
			: RayIntegrator{ camera, sampler, aggregate, lights }
			, max_depth{ max_depth }
			, light_sampler{ LightSampler::create(
				light_sample_strategy, lights, Allocator()) }
			, regularize{ regularize }
		{}

		[[nodiscard]]
		SampledSpectrum light_incoming(RayDifferential ray,
			SampledWavelengths& lambda, Sampler sampler,
			ScratchBuffer& scratch_buffer, VisibleSurface* visible_surface)
			const;

		[[nodiscard]]
		static std::unique_ptr<VolumePathIntegrator> create(
			const ParameterDictionary& parameters, Camera camera,
			Sampler sampler, Primitive aggregate, std::vector<Light> lights,
			const FileLoc* loc);

		[[nodiscard]]
		std::string to_string() const;

	private:
		[[nodiscard]]
		SampledSpectrum sample_light_direct(
			const SurfaceInteraction& interaction, const BSDF* bsdf,
			SampledWavelengths& lambda, Sampler sampler,
			SampledSpectrum beta, SampledSpectrum inverse_wu)
			const;

		int max_depth;
		LightSampler light_sampler;
		bool regularize;
	};

	class AOIntegrator : public RayIntegrator
	{
	public:
		AOIntegrator(bool cos_sample, Float max_distance, Camera camera,
			Sampler sampler, Primitive aggregate, std::vector<Light> lights,
			Spectrum illuminant);

		[[nodiscard]]
		SampledSpectrum light_incoming(RayDifferential ray,
			SampledWavelengths& lambda, Sampler sampler,
			ScratchBuffer& scratch_buffer, VisibleSurface* visible_surface)
			const;

		[[nodiscard]]
		static std::unique_ptr<AOIntegrator> create(
			const ParameterDictionary& parameters, Spectrum illuminant, 
			Camera camera, Sampler sampler, Primitive aggregate,
			std::vector<Light> lights, const FileLoc* loc);

		[[nodiscard]]
		std::string to_string() const;

	private:
		/// <summary>
		/// Whether we use cosine-weighted sampling instead of uniform
		/// hemisphere sampling for ambient occlusion sample rays.
		/// </summary>
		bool cos_sample;
		int max_distance;
		Spectrum illuminant;
		Float illuminant_scale;
	};

	class LightPathIntegrator : public ImageTileIntegrator
	{
	public:

		LightPathIntegrator(int max_depth, Camera camera, Sampler sampler,
			Primitive aggregate, std::vector<Light> lights);

		void evaluate_pixel_sample(Point2i pixel, int sample_index,
			Sampler sampler, ScratchBuffer& scratch_buffer);

		[[nodiscard]]
		static std::unique_ptr<LightPathIntegrator> create(
			const ParameterDictionary& parameters, Camera camera,
			Sampler sampler, Primitive aggregate, std::vector<Light> lights,
			const FileLoc* loc);

		[[nodiscard]]
		std::string to_string() const;

	private:
		int max_depth;
		PowerLightSampler light_sampler;
	};

	struct Vertex;

	class BDPTIntegrator : public RayIntegrator
	{
	public:
		BDPTIntegrator(Camera camera, Sampler sampler, Primitive aggregate,
			std::vector<Light> lights, int max_depth,
			bool visualize_strategies, bool visualize_weights,
			bool regularize = false)
			:RayIntegrator(camera, sampler, aggregate, lights)
			, max_depth{ max_depth }
			, regularize{ regularize }
			, light_sampler{ new PowerLightSampler(lights, Allocator()) }
			, visualize_strategies{ visualize_strategies }
			, visualize_weights{ visualize_weights }
		{}

		[[nodiscard]]
		SampledSpectrum light_incoming(RayDifferential ray,
			SampledWavelengths& lambda, Sampler sampler,
			ScratchBuffer& scratch_buffer, VisibleSurface* visible_surface)
			const;

		[[nodiscard]]
		static std::unique_ptr<BDPTIntegrator> create(
			const ParameterDictionary& parameters, Camera camera,
			Sampler sampler, Primitive aggregate, std::vector<Light> lights,
			const FileLoc* loc);

		[[nodiscard]]
		std::string to_string() const;

		void render() const;

	private:
		int max_depth;
		bool regularize;
		LightSampler light_sampler;
		bool visualize_strategies;
		bool visualize_weights;
		mutable std::vector<Film> weight_films;
	};

	class MLTSampler;

	/// <summary>
	/// Metropolis Light Transport integrator.
	/// </summary>
	class MLTIntegrator : public Integrator
	{
	public:
		MLTIntegrator(Camera camera, Primitive aggregate,
			std::vector<Light> lights, int max_depth, int bootstrap_count,
			int chain_count, int mutations_per_pixel, Float sigma,
			Float large_step_probability, bool regularize)
			: Integrator{ aggregate, lights }
			, light_sampler{ new PowerLightSampler{lights, Allocator()} }
			, camera{ camera }
			, max_depth{ max_depth }
			, bootstrap_count{ bootstrap_count }
			, chain_count{ chain_count }
			, mutations_per_pixel{ mutations_per_pixel }
			, sigma{ sigma }
			, large_step_probability{ large_step_probability }
			, regularize{ regularize }
		{}

		void render();

		[[nodiscard]]
		static std::unique_ptr<MLTIntegrator> create(
			const ParameterDictionary& parameters, Camera camera,
			Primitive aggregate, std::vector<Light> lights);

		[[nodiscard]]
		std::string to_string() const;

	private:
		static constexpr int camera_stream_index = 0;
		static constexpr int light_stream_index = 1;
		static constexpr int connection_stream_index = 2;
		static constexpr int sample_stream_count = 3;

		[[nodiscard]]
		SampledSpectrum radiance(ScratchBuffer& scratch_buffer,
			MLTSampler& sampler, int depth, Point2f* raster,
			SampledWavelengths* wavelengths);

		static Float c(const SampledSpectrum& radiance,
			const SampledWavelengths& wavelengths)
		{
			return radiance.y(wavelengths);
		}

		int bootstrap_count;
		Camera camera;
		int chain_count;
		Float large_step_probability;
		LightSampler light_sampler;
		int max_depth;
		int mutations_per_pixel;
		bool regularize;
		Float sigma;
	};

	/// <summary>
	/// Stochastic Progressive Photon Mapping integrator.
	/// </summary>
	class SPPMIntegrator : public Integrator
	{
	public:
		SPPMIntegrator(Camera camera, Sampler sampler, Primitive aggregate,
			std::vector<Light> lights, int photons_per_iteration,
			int max_depth, Float initial_search_radius, int seed,
			const RGBColorSpace* color_space)
			: Integrator{ aggregate, lights }
			, camera{ camera }
			, sampler_prototype{ sampler }
			, max_depth{ max_depth }
			, photons_per_iteration{ photons_per_iteration > 0 ?
				photons_per_iteration : camera.get_film().get_pixel_bounds().area() }
			, color_space{ color_space }
			, digit_permutations_seed{ seed }
		{}

		[[nodiscard]]
		static std::unique_ptr<SPPMIntegrator> create(
			const ParameterDictionary& parameters, 
			const RGBColorSpace* color_space, Camera camera, Sampler sampler,
			Primitive aggregate, std::vector<Light> lights,
			const FileLoc* loc);

		[[nodiscard]]
		std::string to_string() const;

		void render();

	private:

		[[nodiscard]]
		SampledSpectrum sample_direct_light(
			const SurfaceInteraction& interaction, const BSDF& bsdf,
			SampledWavelengths& wavelengths, Sampler sampler,
			LightSampler light_sampler) const;

		Camera camera;
		Float initial_search_radius;
		Sampler sampler_prototype;
		int digit_permutations_seed;
		int max_depth;
		int photons_per_iteration;
		const RGBColorSpace* color_space;
	};

	class FunctionIntegrator : public Integrator
	{
	public:

		FunctionIntegrator(std::function<double(Point2f)> function,
			std::string_view output_filename, Camera camera,
			Sampler sampler, bool skip_bad, std::string_view image_filename)
			;

		[[nodiscard]]
		static std::unique_ptr<FunctionIntegrator> create(
			const ParameterDictionary& parameters, Camera camera,
			Sampler sampler, const FileLoc* loc);

		void render();
		[[nodiscard]]
		std::string to_string() const;


	private:
		std::function<double(Point2f)> function;
		std::string output_filename;
		Camera camera;
		Sampler base_sampler;
		bool skip_bad;
		std::string image_filename;
	};

}