#pragma once

#include "pbr/math/float.h"
#include "pbr/math/qualifier.h"

namespace loquat
{
	template <unsigned int Dims, typename T, Qualifier Q = default_precision>
	struct point;

	template <typename T, Qualifier Q = default_precision>
	using point2 = point<2, T, Q>;

	template <typename T, Qualifier Q = default_precision>
	using point3 = point<3, T, Q>;

	template <typename T, Qualifier Q = default_precision>
	using point4 = point<4, T, Q>;

	using point2f = point2<Float>;
	using point2i = point2<int>;
	
	using point3f = point3<Float>;
	using point3i = point3<int>;
	
	using point3f = point3<Float>;
	using point3i = point3<int>;
}