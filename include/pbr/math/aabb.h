#pragma once

namespace loquat
{
	template <typename PointType>
		requires is_point<PointType>
	struct AABB;

	using AABB1f = AABB<Point1f>;
	using AABB1i = AABB<Point1i>;

	using AABB2f = AABB<Point2f>;
	using AABB2i = AABB<Point2i>;

	using AABB3f = AABB<Point3f>;
	using AABB3i = AABB<Point3i>;

	template <typename PointType>
		requires is_point<PointType>
	struct AABB
	{
	public:
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

	};
}
