#pragma once

#include "pbr/math/float.h"
#include "pbr/math/qualifier.h"

namespace loquat
{
	template <unsigned int Dims, typename T, Qualifier Q = default_precision>
	struct aabb;

	template <typename T, Qualifier Q = default_precision>
	using aabb2 = aabb<2, T, Q>;

	template <typename T, Qualifier Q = default_precision>
	using aabb3 = aabb<3, T, Q>;

	using aabb2f = aabb2<Float>;
	using aabb2i = aabb2<int>;

	using aabb3f = aabb3<Float>;
	using aabb3i = aabb3<int>;
}