#pragma once

namespace loquat
{
	template <unsigned int Dims, typename T>
	struct Vec;

	template <typename T>
	using Vec1 = Vec<1, T>;

	template <typename T>
	using Vec2 = Vec<2, T>;

	template <typename T>
	using Vec3 = Vec<3, T>;

	using Vec1f = Vec1<Float>;
	using Vec1i = Vec1<int>;

	using Vec2f = Vec2<Float>;
	using Vec2i = Vec2<int>;
	
	using Vec3f = Vec3<Float>;
	using Vec3i = Vec3<int>;
}

#include "pbr/math/vec1.h"
#include "pbr/math/vec2.h"
#include "pbr/math/vec3.h"
