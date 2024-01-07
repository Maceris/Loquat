#pragma once

#include <optional>
#include <string>
#include <vector>

#include "main/loquat.h"
#include "pbr/struct/parameter_dictionary.h"
#include "pbr/base/camera.h"
#include "pbr/base/light.h"
#include "pbr/base/primitive.h"
#include "pbr/base/sampler.h"
#include "pbr/base/shape.h"
#include "pbr/math/ray.h"
#include "pbr/struct/interaction.h"
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
		bool unoccluded(const Interaction& p0, const Interaction& p1) const
		{
			return !has_intersection(p0.spawn_ray_to(p1), 1 - ShadowEpsilon);
		}

		SampledSpectrum Tr(const Interaction& p0, const Interaction& p1,
			const SampledWavelengths& lambda)const;

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
			//TODO(ches) fill this out.
		}
	};
}