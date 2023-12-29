#pragma once

#include "pbr/math/float.h"
#include "pbr/math/qualifier.h"

namespace loquat
{
	template <unsigned int Dims, typename T, Qualifier Q = default_precision>
	struct vec;

	template <typename T, Qualifier Q = default_precision>
	using vec2 = vec<2, T, Q>;

	template <typename T, Qualifier Q = default_precision>
	using vec3 = vec<3, T, Q>;

	template <typename T, Qualifier Q = default_precision>
	using vec4 = vec<4, T, Q>;

	using vec2f = vec2<Float>;
	using vec2i = vec2<int>;
	
	using vec3f = vec3<Float>;
	using vec3i = vec3<int>;
	
	using vec4f = vec4<Float>;
	using vec4i = vec4<int>;
}