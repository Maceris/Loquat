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

		constexpr AABB() noexcept
			: min{ 0 }
			, max{ 0 }
		{}
		constexpr ~AABB() noexcept = default;
		constexpr AABB(const AABB& aabb) noexcept
			: min{ aabb.min }
			, max{ aabb.max }
		{}
		constexpr AABB& operator=(const AABB& aabb) noexcept
		{
			this->min = aabb.min;
			this->max = aabb.max;
			return *this;
		}
		constexpr AABB(AABB&& aabb) noexcept
			: min{ std::move(aabb.min) }
			, max{ std::move(aabb.max) }
		{}
		constexpr AABB& operator=(AABB&& aabb)
		{
			this->min = std::move(aabb.min);
			this->max = std::move(aabb.max);
			return *this;
		}

		constexpr AABB(const PointType& min, const PointType& max) noexcept
			: min{ min }
			, max{ max }
		{}
		constexpr AABB(PointType&& min, PointType&& max) noexcept
			: min{ std::move(min) }
			, max{ std::move(max) }
		{}

		std::string to_string() const noexcept
		{
			return std::format("[ %s - %s ]", vector::to_string(min),
				vector::to_string(max));
		}

		T area() const noexcept
			requires requires (PointType p) { p.x; p.y; }
		{
			PointType diagonal = max - min;
			return diagonal.x * diagonal.y;
		}
	};
}
