// pbrt is Copyright(c) 1998-2020 Matt Pharr, Wenzel Jakob, and Greg Humphreys.
// The pbrt source code is licensed under the Apache License, Version 2.0.
// SPDX: Apache-2.0

// This file has been modified from the original, original notice is above.

#pragma once

#include <format>

namespace loquat
{
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
		constexpr AABB() noexcept
			: min{ 0 }
			, max{ 0 }
		{}
		constexpr ~AABB() noexcept = default;
		LOQUAT_CPU_GPU
		constexpr AABB(const AABB& aabb) noexcept
			: min{ aabb.min }
			, max{ aabb.max }
		{}
		LOQUAT_CPU_GPU
		constexpr AABB& operator=(const AABB& aabb) noexcept
		{
			this->min = aabb.min;
			this->max = aabb.max;
			return *this;
		}
		LOQUAT_CPU_GPU
		constexpr AABB(AABB&& aabb) noexcept
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
		constexpr AABB(const PointType& min, const PointType& max) noexcept
			: min{ min }
			, max{ max }
		{}
		LOQUAT_CPU_GPU
		constexpr AABB(PointType&& min, PointType&& max) noexcept
			: min{ std::move(min) }
			, max{ std::move(max) }
		{}

		std::string to_string() const noexcept
		{
			return std::format("[ %s - %s ]", vector::to_string(min),
				vector::to_string(max));
		}

		LOQUAT_CPU_GPU
		T area() const noexcept
			requires requires (PointType p) { p.x; p.y; }
		{
			PointType diagonal = PointType(max - min);
			return diagonal.x * diagonal.y;
		}

		LOQUAT_CPU_GPU
		bool is_empty() const
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

		LOQUAT_CPU_GPU
		Vec2<T> offset(Point2<T> p) const noexcept
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
		//TODO(ches) add more functions
	};
}
