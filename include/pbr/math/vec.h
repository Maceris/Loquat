#pragma once

namespace loquat
{
	template <typename T>
	using Vec1 = glm::vec<1, T, glm::defaultp>;

	template <typename T>
	using Vec2 = glm::vec<2, T, glm::defaultp>;

	template <typename T>
	using Vec3 = glm::vec<3, T, glm::defaultp>;

	using Vec1f = Vec1<FloatGLM>;
	using Vec1i = Vec1<int>;

	using Vec2f = Vec2<FloatGLM>;
	using Vec2i = Vec2<int>;
	
	using Vec3f = Vec3<FloatGLM>;
	using Vec3i = Vec3<int>;

	using Normal3f = Vec3f;

	/// <summary>
	/// Checks if a type is one of our vec types.
	/// </summary>
	template<typename T>
	concept is_vec =
		   std::same_as<T, Vec1f>
		|| std::same_as<T, Vec1i>
		|| std::same_as<T, Vec2f>
		|| std::same_as<T, Vec2i>
		|| std::same_as<T, Vec3f>
		|| std::same_as<T, Vec3i>;

	template<typename T>
		requires is_vec<T>
	bool has_NaN(T vector) noexcept
	{
		for (glm::length_t i = 0; i < vector.length(); ++i)
		{
			if (isnan(vector[i]))
			{
				return true;
			}
		}
		return false;
	}
}
