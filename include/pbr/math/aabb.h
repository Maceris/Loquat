#pragma once

namespace loquat
{
	template <unsigned int Dims, typename T>
	struct AABB;

	template <typename T>
	using AABB1 = AABB<1, T>;

	template <typename T>
	using AABB2 = AABB<2, T>;

	template <typename T>
	using AABB3 = AABB<3, T>;

	using AABB1f = AABB1<Float>;
	using AABB1i = AABB1<int>;

	using AABB2f = AABB2<Float>;
	using AABB2i = AABB2<int>;

	using AABB3f = AABB3<Float>;
	using AABB3i = AABB3<int>;


	template <unsigned int Dims, typename T>
	struct AABB
	{
	public:
		using PointType = Point<Dims, T>;

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
