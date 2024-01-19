#pragma once

#include <optional>
#include <string>
#include <vector>

#include "main/loquat.h"
#include "pbr/base/camera.h"
#include "pbr/base/film.h"
#include "pbr/base/light.h"
#include "pbr/base/primitive.h"
#include "pbr/base/sampler.h"
#include "pbr/base/shape.h"
#include "pbr/base/spectrum.h"
#include "pbr/math/math.h"
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
		bool unoccluded(const Interaction& p0, const Interaction& p1) const noexcept
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
			: ImageTileIntegrator{camera, sampler, aggregate, lights}
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