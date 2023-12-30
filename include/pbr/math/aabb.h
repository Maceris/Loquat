#pragma once

#include "pbr/math/float.h"
#include "pbr/math/qualifier.h"
#include "pbr/math/vec.h"

namespace loquat
{
	template <unsigned int Dims, typename T, Qualifier Q = Qualifier::default_precision>
	struct AABB;

	template <typename T, Qualifier Q = Qualifier::default_precision>
	using AABB1 = AABB<1, T, Q>;

	template <typename T, Qualifier Q = Qualifier::default_precision>
	using AABB2 = AABB<2, T, Q>;

	template <typename T, Qualifier Q = Qualifier::default_precision>
	using AABB3 = AABB<3, T, Q>;

	using AABB1f = AABB1<Float>;
	using AABB1i = AABB1<int>;

	using AABB2f = AABB2<Float>;
	using AABB2i = AABB2<int>;

	using AABB3f = AABB3<Float>;
	using AABB3i = AABB3<int>;


	template <unsigned int Dims, typename T, Qualifier Q>
	struct AABB
	{
	public:
		vec<Dims, T, Q> min;
		vec<Dims, T, Q> max;

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
	};
}
