// pbrt is Copyright(c) 1998-2020 Matt Pharr, Wenzel Jakob, and Greg Humphreys.
// The pbrt source code is licensed under the Apache License, Version 2.0.
// SPDX: Apache-2.0

// This file has been modified from the original, original notice is above.

#pragma once

namespace loquat {
	
	template <typename T>
	struct Point1 : public Vec1<T>
	{
		using loquat::Vec1<T>::x;
		using loquat::Vec1<T>::operator[];

		LOQUAT_CPU_GPU
		Point1()
			: Vec1<T>()
		{}

		LOQUAT_CPU_GPU
		Point1(T x)
			: Vec1<T>(x)
		{}

		LOQUAT_CPU_GPU
		Point1(glm::vec1 v)
			: Vec1<T>(v)
		{}

		template <typename U>
		LOQUAT_CPU_GPU
		explicit Point1(Point1<U> v)
			: Vec2<T>(T(v.x))
		{}
	};

	template <typename T>
	struct Point2 : public Vec2<T>
	{
		using loquat::Vec2<T>::x;
		using loquat::Vec2<T>::y;
		using loquat::Vec2<T>::operator[];

		LOQUAT_CPU_GPU
		Point2()
			: Vec2<T>()
		{}

		LOQUAT_CPU_GPU
		Point2(T x)
			: Vec2<T>(x)
		{}

		LOQUAT_CPU_GPU
		Point2(T x, T y)
			: Vec2<T>(x, y)
		{}

		LOQUAT_CPU_GPU
		Point2(glm::vec2 v)
			: Vec2<T>(v)
		{}

		template <typename U>
		LOQUAT_CPU_GPU
		explicit Point2(Point2<U> v)
			: Vec2<T>(T(v.x), T(v.y))
		{}
	};

	template <typename T>
	struct Point3 : public Vec3<T>
	{
		using loquat::Vec3<T>::x;
		using loquat::Vec3<T>::y;
		using loquat::Vec3<T>::z;
		using loquat::Vec3<T>::operator[];

		LOQUAT_CPU_GPU
		Point3()
			: Vec3<T>()
		{}

		LOQUAT_CPU_GPU
		Point3(T x)
			: Vec3<T>(x)
		{}

		LOQUAT_CPU_GPU
		Point3(T x, T y, T z) 
			: Vec3<T>(x, y, z)
		{}

		LOQUAT_CPU_GPU
		Point3(glm::vec3 v)
			: Vec3<T>(v)
		{}

		template <typename U>
		LOQUAT_CPU_GPU
		explicit Point3(Point3<U> v)
			: Vec3<T>(T(v.x), T(v.y), T(v.z))
		{}
		
	};

	using Point1f = Point1<Float>;
	using Point1i = Point1<int>;

	using Point2f = Point2<Float>;
	using Point2i = Point2<int>;

	using Point3f = Point3<Float>;
	using Point3i = Point3<int>;

    class Point3fi;

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
		|| std::same_as<T, Point3f>
		|| std::same_as<T, Point3fi>;

	namespace vector
	{
		template<typename T>
			requires is_point<T>
		LOQUAT_CPU_GPU
			bool has_NaN(T point) noexcept
		{
			for (glm::length_t i = 0; i < point.length(); ++i)
			{
				bool NaN;
#ifdef LOQUAT_IS_GPU_CODE
				NaN = isnan(point[i]);
#else
				NaN = std::isnan(point[i]);
#endif
				if (NaN)
				{
					return true;
				}
			}
			return false;
		}
	}
}
