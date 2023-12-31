#pragma once

namespace loquat {

	template <unsigned int Dims, typename T>
	using Point = Vec<Dims, T>;

	using Point2f = Vec2f;
	using Point2i = Vec2i;

	using Point3f = Vec3f;
	using Point3i = Vec3i;
}
