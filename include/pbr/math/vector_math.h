// pbrt is Copyright(c) 1998-2020 Matt Pharr, Wenzel Jakob, and Greg Humphreys.
// The pbrt source code is licensed under the Apache License, Version 2.0.
// SPDX: Apache-2.0

// This file has been modified from the original, original notice is above.

#pragma once

#include <format>
#include <limits>
#include <type_traits>

#include "main/loquat.h"
#include "pbr/math/float.h"
#include "pbr/math/math.h"
#include "pbr/math/vec.h"

namespace loquat
{

	class Point3fi : public Vec3<Interval>
	{
	public:
		using Vec3<Interval>::x;
		using Vec3<Interval>::y;
		using Vec3<Interval>::z;
		using Vec3<Interval>::operator*=;

		Point3fi() = default;
		LOQUAT_CPU_GPU
		Point3fi(Interval x, Interval y, Interval z)
			: Vec3<Interval>{ x, y, z }
		{}
		LOQUAT_CPU_GPU
		Point3fi(Float x, Float y, Float z)
			: Vec3<Interval>{ Interval(x), Interval(y), Interval(z) }
		{}
		LOQUAT_CPU_GPU
		Point3fi(const Point3f& point)
			: Vec3<Interval>{
			Interval(point.x), Interval(point.y), Interval(point.z) }
		{}
		LOQUAT_CPU_GPU
		Point3fi(Vec3<Interval> point)
			: Vec3<Interval>{ point }
		{}
		LOQUAT_CPU_GPU
		Point3fi(Point3f point, Vec3f error)
		: Vec3<Interval>{ Interval::from_value_and_error(point.x, error.x),
			Interval::from_value_and_error(point.y, error.y),
			Interval::from_value_and_error(point.z, error.z) }
		{}

		LOQUAT_CPU_GPU
		Vec3f error() const noexcept
		{
			return {
				x.width() / 2,
				y.width() / 2,
				z.width() / 2
			};
		}
		LOQUAT_CPU_GPU
		bool is_exact() const noexcept
		{
			return x.width() == 0 && y.width() == 0 && z.width() == 0;
		}

		[[nodiscard]]
		LOQUAT_CPU_GPU
		Vec3f to_vec() const noexcept
		{
			return Vec3f{ x.midpoint(), y.midpoint(), z.midpoint() };
		}
	};

	/// <summary>
	/// Construct a local orthonormal coordinate system given a single
	/// normalized vector.
	/// </summary>
	/// <typeparam name="T">The type of vectors.</typeparam>
	/// <param name="v1">The normalized vector to start with.</param>
	/// <param name="v2">An out parameter for the second vector.</param>
	/// <param name="v3">An out parameter for the third vector.</param>
	template <typename T>
	LOQUAT_CPU_GPU
	inline void coordinate_system(Vec3<T> v1, Vec3<T>* v2, Vec3<T>* v3)
		noexcept
	{
		const Float sign = std::copysign(Float(1), v1.z);
		const Float a = -1 / (sign + v1.z);
		const Float b = v1.x * v1.y * a;
		*v2 = Vec3<T>{ 1 + sign * square(v1.x) * a, sign * b, -sign * v1.x };
		*v3 = Vec3<T>{ b, sign + square(v1.y) * a, -v1.y };
	}
	
	class Frame
	{
	public:

		LOQUAT_CPU_GPU
		Frame()
			: x{ 1, 0, 0 }
			, y{ 0, 1, 0 }
			, z{ 0, 0, 1 }
		{}
		LOQUAT_CPU_GPU
		Frame(Vec3f x, Vec3f y, Vec3f z);

		LOQUAT_CPU_GPU
		static Frame from_xz(Vec3f x, Vec3f z) noexcept
		{
			return Frame{ x, glm::cross(z, x), z };
		}

		LOQUAT_CPU_GPU
		static Frame from_xy(Vec3f x, Vec3f y) noexcept
		{
			return Frame{ x, y, glm::cross(x, y) };
		}

		LOQUAT_CPU_GPU
		static Frame from_x(Vec3f x) noexcept
		{
			Vec3f y;
			Vec3f z;
			coordinate_system(x, &y, &z);
			return Frame{ x, y, z };
		}

		LOQUAT_CPU_GPU
		static Frame from_y(Vec3f y) noexcept
		{
			Vec3f x;
			Vec3f z;
			coordinate_system(y, &z, &x);
			return Frame{ x, y, z };
		}

		LOQUAT_CPU_GPU
		static Frame from_z(Vec3f z) noexcept
		{
			Vec3f x;
			Vec3f y;
			coordinate_system(z, &x, &z);
			return Frame{ x, y, z };
		}

		LOQUAT_CPU_GPU
		Vec3f to_local(Vec3f vector) const noexcept
		{
			return Vec3f{ glm::dot(vector, x), glm::dot(vector, y),
				glm::dot(vector, z) };
		}

		LOQUAT_CPU_GPU
		Vec3f from_local(Vec3f vector) const noexcept
		{
			return vector.x * x + vector.y * y + vector.z * z;
		}

		std::string to_string() const noexcept
		{
			return std::format("[ Frame x: %s, y: %s, z: %s ]",
				vector::to_string(x), vector::to_string(y), 
				vector::to_string(z));
		}

		Vec3f x;
		Vec3f y;
		Vec3f z;
	};

	template <typename T>
	[[nodiscard]]
	LOQUAT_CPU_GPU
	inline T absolute_dot(Vec2<T> v1, Vec2<T> v2)
	{
		LOG_ASSERT(!has_NaN(v1) && !has_NaN(v1) 
			&& "Calculating dot products of vectors with NaNs");
		return std::abs(glm::dot(v1, v2));
	}

	template <typename T>
	[[nodiscard]]
	LOQUAT_CPU_GPU
	inline T absolute_dot(Vec3<T> v1, Vec3<T> v2)
	{
		LOG_ASSERT(!vector::has_NaN(v1) && !vector::has_NaN(v1)
			&& "Calculating dot products of vectors with NaNs");
		return std::abs(glm::dot(v1, v2));
	}

	LOQUAT_CPU_GPU
	inline Float absolute_cos_theta(Vec3f w)
	{
		return std::abs(w.z);
	}


	LOQUAT_CPU_GPU
	inline Float spherical_theta(Vec3f v)
	{
		return safe_acos(v.z);
	}

	LOQUAT_CPU_GPU
	inline Float cos_theta(Vec3f w)
	{
		return w.z;
	}

	LOQUAT_CPU_GPU
	inline Float cos2_theta(Vec3f w)
	{
		return square(w.z);
	}

	LOQUAT_CPU_GPU
	inline Float abs_cos_theta(Vec3f w)
	{
		return std::abs(w.z);
	}

	LOQUAT_CPU_GPU
	inline Float sin2_theta(Vec3f w)
	{
		return std::max<Float>(0, 1 - cos2_theta(w));
	}
	LOQUAT_CPU_GPU
	inline Float sin_theta(Vec3f w)
	{
		return std::sqrt(sin2_theta(w));
	}

	LOQUAT_CPU_GPU
	inline Float tan_theta(Vec3f w)
	{
		return sin_theta(w) / cos_theta(w);
	}
	LOQUAT_CPU_GPU
	inline Float tan2_theta(Vec3f w) 
	{
		return sin2_theta(w) / cos2_theta(w);
	}

	LOQUAT_CPU_GPU
	inline Float cos_phi(Vec3f w)
	{
		Float sinTheta = sin_theta(w);
		return (sinTheta == 0) ? 1 : clamp(w.x / sinTheta, -1, 1);
	}
	LOQUAT_CPU_GPU
	inline Float sin_phi(Vec3f w)
	{
		Float sinTheta = sin_theta(w);
		return (sinTheta == 0) ? 0 : clamp(w.y / sinTheta, -1, 1);
	}

	LOQUAT_CPU_GPU
	inline Float cos_d_phi(Vec3f wa, Vec3f wb)
	{
		Float waxy = square(wa.x) + square(wa.y), wbxy = square(wb.x) + square(wb.y);
		if (waxy == 0 || wbxy == 0)
			return 1;
		return clamp((wa.x * wb.x + wa.y * wb.y) / std::sqrt(waxy * wbxy), -1, 1);
	}

	template <typename T>
	LOQUAT_CPU_GPU
	inline auto distance(Point2<T> p1, Point2<T> p2) -> typename glm::length_t 
	{
		return glm::length(p1 - p2);
	}

	template <typename T>
	LOQUAT_CPU_GPU
	inline auto distance(Point3<T> p1, Point3<T> p2) -> typename glm::length_t
	{
		return glm::length(p1 - p2);
	}

	template <typename T>
	LOQUAT_CPU_GPU
	inline auto distance_squared(Point2<T> p1, Point2<T> p2) ->
		typename glm::length_t
	{
		glm::length_t length = glm::length(p1 - p2);
		return length * length;
	}

	template <typename T>
	LOQUAT_CPU_GPU
	inline auto distance_squared(Point3<T> p1, Point3<T> p2) ->
		typename glm::length_t
	{
		glm::length_t length = glm::length(p1 - p2);
		return length * length;
	}

	template <typename T>
	[[nodiscard]]
	LOQUAT_CPU_GPU
	inline Normal3<T> face_forward(Normal3<T> normal, Vec3<T> vector)
	{
		return glm::dot(normal, vector) < 0.0f ? -normal : normal;
	}

	class DirectionCone
	{
	public:
		DirectionCone() = default;

		LOQUAT_CPU_GPU
		DirectionCone(Vec3f direction, Float cos_theta) noexcept
			: direction{ glm::normalize(direction) }
			, cos_theta{ cos_theta }
		{}

		LOQUAT_CPU_GPU
		explicit DirectionCone(Vec3f direction) noexcept
			: DirectionCone(direction, 1)
		{}

		LOQUAT_CPU_GPU
		bool is_empty() const noexcept
		{
			return cos_theta == FLOAT_INFINITY;
		}

		LOQUAT_CPU_GPU
		static DirectionCone entire_sphere()
		{
			return DirectionCone{ Vec3f(0,0,1), -1 };
		}

		[[nodiscard]]
		std::string to_string() const noexcept;

		LOQUAT_CPU_GPU
		Vec3f closest_vector_in_cone(Vec3f vec) const noexcept;

		Vec3f direction;
		Float cos_theta = FLOAT_INFINITY;
	};

	template <typename T>
	LOQUAT_CPU_GPU
	inline auto lerp(Float t, Vec3<T> t0, Vec3<T> t1) {
		return (1 - t) * t0 + t * t1;
	}

	template <typename T>
	LOQUAT_CPU_GPU
	inline auto lerp(Float t, Point3<T> t0, Point3<T> t1) {
		return (1 - t) * t0 + t * t1;
	}

	LOQUAT_CPU_GPU
	inline bool same_hemisphere(Vec3f w, Vec3f wp) {
		return w.z * wp.z > 0;
	}

	template <template<typename U> typename PointBase, typename T>
		requires is_point<PointBase<T>>
	struct AABB;

	using AABB1f = AABB<Point1, Float>;
	using AABB1i = AABB<Point1, int>;

	using AABB2f = AABB<Point2, Float>;
	using AABB2i = AABB<Point2, int>;

	using AABB3f = AABB<Point3, Float>;
	using AABB3i = AABB<Point3, int>;

	template <template<typename U> typename PointBase, typename T>
		requires is_point<PointBase<T>>
	struct AABB
	{
	public:
		using PointType = PointBase<T>;

		PointType min;
		PointType max;

		LOQUAT_CPU_GPU
		constexpr AABB()
			: min{ std::numeric_limits<T>::lowest() }
			, max{ std::numeric_limits<T>::max() }
		{}
		constexpr ~AABB() = default;
		LOQUAT_CPU_GPU
		constexpr AABB(const AABB& aabb)
			: min{ aabb.min }
			, max{ aabb.max }
		{
			if (aabb.is_empty())
			{
				*this = AABB();
			}
			else
			{
				min = PointType(aabb.min);
				max = PointType(aabb.max);
			}
		}

		LOQUAT_CPU_GPU
		constexpr AABB& operator=(const AABB& aabb)
		{
			this->min = aabb.min;
			this->max = aabb.max;
			return *this;
		}
		LOQUAT_CPU_GPU
		constexpr AABB(AABB&& aabb)
			: min{ std::move(aabb.min) }
			, max{ std::move(aabb.max) }
		{}
		LOQUAT_CPU_GPU
		constexpr AABB& operator=(AABB&& aabb)
		{
			this->min = std::move(aabb.min);
			this->max = std::move(aabb.max);
			return *this;
		}

		LOQUAT_CPU_GPU
		constexpr AABB(const PointType& min, const PointType& max)
			: min{ min }
			, max{ max }
		{}
		LOQUAT_CPU_GPU
		constexpr AABB(PointType&& min, PointType&& max)
			: min{ std::move(min) }
			, max{ std::move(max) }
		{}

		std::string to_string() const
		{
			return std::format("[ {} - {} ]", vector::to_string(min),
				vector::to_string(max));
		}

		LOQUAT_CPU_GPU
		T area() const
			requires requires (PointType p) { p.x; p.y; }
		{
			PointType diagonal = PointType(max - min);
			return diagonal.x * diagonal.y;
		}

		LOQUAT_CPU_GPU
		bool constexpr is_empty() const
		{
			for (int dim = 0; dim < PointType::length(); ++dim)
			{
				if (min[dim] >= max[dim])
				{
					return true;
				}
			}
			return false;
		}

		template<typename Base = T, std::enable_if_t<std::is_same<PointBase<Base>, Point1<Base>>::value>* = nullptr>
		LOQUAT_CPU_GPU
		auto offset(PointBase<T> p) const
		{
			Vec1<T> result = p - min;
			if (max.x > min.x)
			{
				result.x /= max.x - min.x;
			}
			return result;
		}

		template<typename Base = T, std::enable_if_t<std::is_same<PointBase<Base>, Point2<Base>>::value>* = nullptr>
		LOQUAT_CPU_GPU
		auto offset(PointBase<T> p) const
		{
			Vec2<T> result = p - min;
			if (max.x > min.x)
			{
				result.x /= max.x - min.x;
			}
			if (max.y > min.y)
			{
				result.y /= max.y - min.y;
			}
			return result;
		}

		template<typename Base = T, std::enable_if_t<std::is_same<PointBase<Base>, Point3<Base>>::value>* = nullptr>
		LOQUAT_CPU_GPU
		auto offset(PointBase<T> p) const
		{
			Vec3<T> result = p - min;
			if (max.x > min.x)
			{
				result.x /= max.x - min.x;
			}
			if (max.y > min.y)
			{
				result.y /= max.y - min.y;
			}
			if (max.z > min.z)
			{
				result.z /= max.z - min.z;
			}
			return result;
		}

		LOQUAT_CPU_GPU
		PointType operator[](int i) const
		{
			LOG_ASSERT(i == 0 || i == 1);
			return (i == 0) ? min : max;
		}

		LOQUAT_CPU_GPU
		PointType& operator[](int i)
		{
			LOG_ASSERT(i == 0 || i == 1);
			return (i == 0) ? min : max;
		}

		LOQUAT_CPU_GPU
		PointType corner(int choice) const
		{
			if constexpr (PointType::length() == 2)
			{
				LOG_ASSERT(choice >= 0 && choice < 4);
				return PointType(
					(*this)[(choice & 1)].x, 
					(*this)[(choice & 2) ? 1 : 0].y
				);
			}
			else if constexpr (PointType::length() == 3)
			{
				LOG_ASSERT(choice >= 0 && choice < 8);
				return PointType(
					(*this)[(choice & 1)].x, 
					(*this)[(choice & 2) ? 1 : 0].y,
					(*this)[(choice & 4) ? 1 : 0].z);
			}
			else
			{
				LOG_FATAL("Unexpected point type dimensions");
			}
		}

		LOQUAT_CPU_GPU
		Vec3<T> diagonal() const { return max - min; }

		LOQUAT_CPU_GPU
		T surface_area() const
		{
			Vec3<T> d = diagonal();
			return 2 * (d.x * d.y + d.x * d.z + d.y * d.z);
		}

		LOQUAT_CPU_GPU
		T volume() const
		{
			Vec3<T> d = diagonal();
			return d.x * d.y * d.z;
		}

		LOQUAT_CPU_GPU
		int max_dimension() const
		{
			Vec3<T> d = diagonal();
			if (d.x > d.y && d.x > d.z)
				return 0;
			else if (d.y > d.z)
				return 1;
			else
				return 2;
		}

		LOQUAT_CPU_GPU
		PointType lerp(PointType t) const
		{
			if constexpr (PointType::length() == 2)
			{
				return PointType(loquat::lerp(t.x, min.x, max.x), loquat::lerp(t.y, min.y, max.y));
			}
			else if constexpr (PointType::length() == 3)
			{
				return PointType(loquat::lerp(t.x, min.x, max.x), loquat::lerp(t.y, min.y, max.y),
					loquat::lerp(t.z, min.z, max.z));
			}
			else
			{
				LOG_FATAL("Unexpected point type dimensions");
			}
		}

		LOQUAT_CPU_GPU
		void bounding_sphere(PointType* center, Float* radius) const
		{
			*center = (min + max) / 2;
			*radius = inside(*center, *this) ? distance(*center, max) : 0;
		}

		LOQUAT_CPU_GPU
		bool is_degenerate() const
		{
			for (int dim = 0; dim < PointType::length(); ++dim)
			{
				if (min[dim] > max[dim])
				{
					return true;
				}
			}
			return false;
		}

		LOQUAT_CPU_GPU
		bool operator==(const AABB& b) const
		{
			return b.min == min && b.max == max;
		}

		LOQUAT_CPU_GPU
		bool operator!=(const AABB& b) const
		{
			return b.min != min || b.max != max;
		}

		LOQUAT_CPU_GPU
		bool has_intersection(Point3f origin, Vec3f direction, 
			Float t_max = Infinity,
			Float* hit_t0 = nullptr, Float* hit_t1 = nullptr) const;

		LOQUAT_CPU_GPU
		bool has_intersection(Point3f origin, Vec3f direction, 
			Float t_max, Vec3f inv_dir, const int dir_is_negative[3]) const;
	
	};

	template <template<typename U> typename PointBase, typename T>
		requires is_point<PointBase<T>>
	class BoundsIterator : public std::forward_iterator_tag {
	public:
		LOQUAT_CPU_GPU
		BoundsIterator(const AABB<PointBase, T>& b, const PointBase<T>& pt)
			: p(pt)
			, bounds(&b)
		{}

		LOQUAT_CPU_GPU
		BoundsIterator operator++()
		{
			advance();
			return *this;
		}

		LOQUAT_CPU_GPU
		BoundsIterator operator++(int)
		{
			BoundsIterator old = *this;
			advance();
			return old;
		}

		LOQUAT_CPU_GPU
		bool operator==(const BoundsIterator& bi) const
		{
			return p == bi.p && bounds == bi.bounds;
		}

		LOQUAT_CPU_GPU
		bool operator!=(const BoundsIterator& bi) const
		{
			return p != bi.p || bounds != bi.bounds;
		}

		LOQUAT_CPU_GPU
		PointBase<T> operator*() const { return p; }

	private:
		LOQUAT_CPU_GPU
		void advance()
		{
			++p.x;
			if (p.x == bounds->max.x)
			{
				p.x = bounds->min.x;
				++p.y;
			}
		}
		PointBase<T> p;
		const AABB<PointBase, T>* bounds;
	};

	template <template<typename U> typename PointBase, typename T>
		requires is_point<PointBase<T>>
	LOQUAT_CPU_GPU
	inline BoundsIterator<PointBase, T> begin(const AABB<PointBase, T>& b)
	{
		return BoundsIterator<PointBase, T>(b, b.min);
	}

	template <template<typename U> typename PointBase, typename T>
		requires is_point<PointBase<T>>
	LOQUAT_CPU_GPU
	inline BoundsIterator<PointBase, T> end(const AABB<PointBase, T>& b)
	{
		// Normally, the ending point is at the minimum x value and one past
		// the last valid y value.
		Point2i pEnd(b.min.x, b.max.y);
		// However, if the bounds are degenerate, override the end point to
		// equal the start point so that any attempt to iterate over the bounds
		// exits out immediately.
		if (b.min.x >= b.max.x || b.min.y >= b.max.y)
		{
			pEnd = b.min;
		}
		return BoundsIterator<PointBase, T>(b, pEnd);
	}

	LOQUAT_CPU_GPU
	inline Vec3f spherical_direction(Float sin_theta, Float cos_theta,
		Float phi) noexcept
	{
		LOG_ASSERT(sin_theta >= -1.0001 && sin_theta <= 1.0001);
		LOG_ASSERT(cos_theta >= -1.0001 && cos_theta <= 1.0001);

		return {
			clamp(sin_theta, -1, 1) * std::cos(phi),
			clamp(sin_theta, -1, 1) * std::sin(phi),
			clamp(cos_theta, -1, 1)
		};
	}

	template <typename T>
	LOQUAT_CPU_GPU
	inline AABB<Point2, T> bounds_union(const AABB<Point2, T>& b1,
		const AABB<Point2, T>& b2)
	{
		// Be careful to not run the two-point Bounds constructor.
		AABB<Point2, T> ret;
		ret.min = min(b1.min, b2.min);
		ret.max = min(b1.max, b2.max);
		return ret;
	}

	template <typename T>
	LOQUAT_CPU_GPU
	inline AABB<Point3, T> bounds_union(const AABB<Point3, T>& b, Point3<T> p)
	{
		AABB<Point3, T> ret;
		ret.min = Min(b.min, p);
		ret.max = Max(b.max, p);
		return ret;
	}

	template <typename T>
	LOQUAT_CPU_GPU
	inline AABB<Point3, T> bounds_union(const AABB<Point3, T>& b1,
		const AABB<Point3, T>& b2)
	{
		AABB<Point3, T> ret;
		ret.min = Min(b1.min, b2.min);
		ret.max = Max(b1.max, b2.max);
		return ret;
	}

	template <typename T>
	LOQUAT_CPU_GPU
	inline bool inside_exclusive(Point2<T> point,
		const AABB<Point2, T>& bounds)
	{
		return point.x >= bounds.min.x
			&& point.x < bounds.max.x
			&& point.y >= bounds.min.y
			&& point.y < bounds.min.y;
	}

	template <typename T>
	LOQUAT_CPU_GPU
	inline bool inside_exclusive(Point3<T> point,
		const AABB<Point3, T>& bounds)
	{
		return point.x >= bounds.min.x
			&& point.x < bounds.max.x
			&& point.y >= bounds.min.y
			&& point.y < bounds.min.y
			&& point.z >= bounds.min.z
			&& point.z < bounds.min.z;
	}

	/// <summary>
	/// Returns an angle in [0, 2*PI], adjusted from the results of std::atan2.
	/// </summary>
	/// <param name="v">The dimensional vector, whose x and y coordinates
	/// will be used to return the phi angle.</param>
	/// <returns>The spherical angle phi.</returns>
	LOQUAT_CPU_GPU
	inline Float spherical_phi(Vec3f v)
	{
		Float phi = std::atan2(v.y, v.x);
		return (phi < 0) ? (phi + 2 * PI) : phi;
	}

	template<typename T>
	LOQUAT_CPU_GPU
	inline AABB<Point2, T> union_bounds(const AABB<Point2, T>& b1, const AABB<Point2, T>& b2)
	{
		// Be careful to not run the two-point Bounds constructor.
		AABB<Point2, T> ret;
		ret.min = min(b1.min, b2.min);
		ret.max = max(b1.max, b2.max);
		return ret;
	}

	template<typename T>
	LOQUAT_CPU_GPU
		inline AABB<Point3, T> union_bounds(const AABB<Point3, T>& b1,
			const AABB<Point3, T>& b2)
	{
		// Be careful to not run the two-point Bounds constructor.
		AABB<Point3, T> ret;
		ret.min = min(b1.min, b2.min);
		ret.max = max(b1.max, b2.max);
		return ret;
	}

	LOQUAT_CPU_GPU
	DirectionCone bounds_union(const DirectionCone& a, const DirectionCone& b);

	//TODO(ches) finish this
}