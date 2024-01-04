#pragma once

namespace loquat {

	using Point1f = Vec1f;
	using Point1i = Vec1i;

	using Point2f = Vec2f;
	using Point2i = Vec2i;

	using Point3f = Vec3f;
	using Point3i = Vec3i;

	/// <summary>
	/// Checks if a type is one of our point types.
	/// </summary>
	template<typename T>
	concept is_point = 
		   std::same_as<T, Point1i> 
		|| std::same_as<T, Point1f>
		|| std::same_as<T, Point2i> 
		|| std::same_as<T, Point2f>
		|| std::same_as<T, Point3i>
		|| std::same_as<T, Point3f>;
}
