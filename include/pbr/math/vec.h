#pragma once

#include <concepts>

#include "pbr/math/float.h"
#include "pbr/math/qualifier.h"

namespace loquat
{
	template <unsigned int Dims, typename T, Qualifier Q = default_precision>
	struct vec;

	template <typename T, Qualifier Q = default_precision>
	using vec1 = vec<1, T, Q>;

	template <typename T, Qualifier Q = default_precision>
	using vec2 = vec<2, T, Q>;

	template <typename T, Qualifier Q = default_precision>
	using vec3 = vec<3, T, Q>;

	using vec1f = vec1<Float>;
	using vec1i = vec1<int>;

	using vec2f = vec2<Float>;
	using vec2i = vec2<int>;
	
	using vec3f = vec3<Float>;
	using vec3i = vec3<int>;
}

#include "pbr/math/vec1.h"
#include "pbr/math/vec2.h"
#include "pbr/math/vec3.h"
