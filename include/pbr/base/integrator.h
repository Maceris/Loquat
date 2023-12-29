#pragma once

#include <optional>
#include <string>
#include <vector>

#include "pbr/ray.h"
#include "pbr/base/light.h"
#include "pbr/base/primitive.h"
#include "pbr/base/shape.h"
#include "pbr/math/float.h"

/// <summary>
/// Responsible for rendering a scene, calculating the results of 
/// the rendering equation.
/// </summary>
class Integrator
{
public:
	virtual ~Integrator();

	virtual void render() = 0;
	virtual std::string to_string() const noexcept = 0;

	std::optional<ShapeIntersection> intersect(const Ray& ray, 
		Float t_max = FLOAT_INFINITY) const;

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