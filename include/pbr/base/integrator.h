// pbrt is Copyright(c) 1998-2020 Matt Pharr, Wenzel Jakob, and Greg Humphreys.
// The pbrt source code is licensed under the Apache License, Version 2.0.
// SPDX: Apache-2.0

// This file has been modified from the original, original notice is above.

#pragma once

#include <format>
#include <optional>
#include <memory>
#include <string>
#include <vector>

#include "pbr/base/camera.h"
#include "pbr/base/film.h"
#include "pbr/base/light.h"
#include "pbr/light_samplers.h"
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
		bool has_intersection(const Ray& ray,
			Float t_max = FLOAT_INFINITY) const noexcept;
		
		[[nodiscard]]
		std::optional<ShapeIntersection> intersect(const Ray& ray,
			Float t_max = FLOAT_INFINITY) const noexcept;

		[[nodiscard]]
		bool unoccluded(const Interaction& p0, const Interaction& p1)
			const noexcept
		{
			return !has_intersection(p0.spawn_ray_to(p1), 1 - SHADOW_EPSILON);
		}

		[[nodiscard]]
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
			LOG_INFO(std::format("Scene bounds are {}", 
				scene_bounds.to_string()));
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

		[[nodiscard]]
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

		[[nodiscard]]
		static std::unique_ptr<RandomWalkIntegrator> create(
			const ParameterDictionary& parameters, Camera camera,
			Sampler sampler, Primitive aggregate, std::vector<Light> lights);

		[[nodiscard]]
		std::string to_string() const noexcept;

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
			ScratchBuffer& scratch_buffer, int depth) const noexcept;

		int max_depth;
	};

	class SimplePathIntegrator : public RayIntegrator
	{
	public:
		SimplePathIntegrator(int max_depth, bool sample_lights,
			bool sample_BSDF, Camera camera, Sampler sampler,
			Primitive aggregate, std::vector<Light> lights) noexcept;

		[[nodiscard]]
		SampledSpectrum light_incoming(RayDifferential ray,
			SampledWavelengths& lambda, Sampler sampler,
			ScratchBuffer& scratch_buffer, VisibleSurface* visible_surface)
			const noexcept;

		[[nodiscard]]
		static std::unique_ptr<SimplePathIntegrator> create(
			const ParameterDictionary& parameters, Camera camera,
			Sampler sampler, Primitive aggregate, std::vector<Light> lights)
			noexcept;

		[[nodiscard]]
		std::string to_string() const noexcept;

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
			const noexcept;

		[[nodiscard]]
		static std::unique_ptr<PathIntegrator> create(
			const ParameterDictionary& parameters, Camera camera,
			Sampler sampler, Primitive aggregate, std::vector<Light> lights)
			noexcept;

		[[nodiscard]]
		std::string to_string() const noexcept;

	private:
		[[nodiscard]]
		SampledSpectrum light_direct(const SurfaceInteraction& interaction,
			const BSDF* bsdf, SampledWavelengths& lambda, Sampler sampler)
			const noexcept;

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
			const noexcept;

		[[nodiscard]]
		static std::unique_ptr<SimpleVolumePathIntegrator> create(
			const ParameterDictionary& parameters, Camera camera,
			Sampler sampler, Primitive aggregate, std::vector<Light> lights)
			noexcept;

		[[nodiscard]]
		std::string to_string() const noexcept;

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
				light_sample_strategy, lights, *g_allocator) }
			, regularize{ regularize }
		{}

		[[nodiscard]]
		SampledSpectrum light_incoming(RayDifferential ray,
			SampledWavelengths& lambda, Sampler sampler,
			ScratchBuffer& scratch_buffer, VisibleSurface* visible_surface)
			const noexcept;

		[[nodiscard]]
		static std::unique_ptr<VolumePathIntegrator> create(
			const ParameterDictionary& parameters, Camera camera,
			Sampler sampler, Primitive aggregate, std::vector<Light> lights)
			noexcept;

		[[nodiscard]]
		std::string to_string() const noexcept;

	private:
		[[nodiscard]]
		SampledSpectrum sample_light_direct(
			const SurfaceInteraction& interaction, const BSDF* bsdf,
			SampledWavelengths& lambda, Sampler sampler,
			SampledSpectrum beta, SampledSpectrum inverse_wu)
			const noexcept;

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
			const noexcept;

		[[nodiscard]]
		static std::unique_ptr<AOIntegrator> create(
			const ParameterDictionary& parameters, Spectrum illuminant, 
			Camera camera, Sampler sampler, Primitive aggregate,
			std::vector<Light> lights)
			noexcept;

		[[nodiscard]]
		std::string to_string() const noexcept;

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
			Primitive aggregate, std::vector<Light> lights) noexcept;

		void evaluate_pixel_sample(Point2i pixel, int sample_index,
			Sampler sampler, ScratchBuffer& scratch_buffer) noexcept;

		static std::unique_ptr<LightPathIntegrator> create(
			const ParameterDictionary& parameters, Camera camera,
			Sampler sampler, Primitive aggregate, std::vector<Light> lights)
			noexcept;

		[[nodiscard]]
		std::string to_string() const noexcept;

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
			bool regularize = false) noexcept
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
			const noexcept;

		static std::unique_ptr<BDPTIntegrator> create(
			const ParameterDictionary& parameters, Camera camera,
			Sampler sampler, Primitive aggregate, std::vector<Light> lights)
			noexcept;

		[[nodiscard]]
		std::string to_string() const noexcept;

		void render() const noexcept;

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
			Float large_step_probability, bool regularize) noexcept
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

		void render() noexcept;

		static std::unique_ptr<MLTIntegrator> create(
			const ParameterDictionary& parameters, Camera camera,
			Primitive aggregate, std::vector<Light> lights) noexcept;

		[[nodiscard]]
		std::string to_string() const noexcept;

	private:
		static constexpr int camera_stream_index = 0;
		static constexpr int light_stream_index = 1;
		static constexpr int connection_stream_index = 2;
		static constexpr int sample_stream_count = 3;

		SampledSpectrum radiance(ScratchBuffer& scratch_buffer,
			MLTSampler& sampler, int depth, Point2f* raster,
			SampledWavelengths* wavelengths) noexcept;

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
			const RGBColorSpace* color_space) noexcept
			: Integrator{ aggregate, lights }
			, camera{ camera }
			, sampler_prototype{ sampler }
			, max_depth{ max_depth }
			, photons_per_iteration{ photons_per_iteration > 0 ?
			photons_per_iteration : camera.get_film().pixel_bounds().area() }
			, color_space{ color_space }
			, digit_permutations_seed{ seed }
		{}

		static std::unique_ptr<SPPMIntegrator> create(
			const ParameterDictionary& parameters, 
			const RGBColorSpace* color_space, Camera camera, Sampler sampler,
			Primitive aggregate, std::vector<Light> lights) noexcept;

		[[nodiscard]]
		std::string to_string() const noexcept;

		void render();

	private:

		SampledSpectrum sample_direct_light(
			const SurfaceInteraction& interaction, const BSDF& bsdf,
			SampledWavelengths& wavelengths, Sampler sampler,
			LightSampler light_sampler) const noexcept;

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

	};

}