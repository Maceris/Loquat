// pbrt is Copyright(c) 1998-2020 Matt Pharr, Wenzel Jakob, and Greg Humphreys.
// The pbrt source code is licensed under the Apache License, Version 2.0.
// SPDX: Apache-2.0

// This file has been modified from the original, original notice is above.

#pragma once

#include <concepts>
#include <format>
#include <ostream>
#include <string>

namespace loquat
{
	namespace internal {

		template <typename T>
		std::string to_string_1(T x);
		template <typename T>
		std::string to_string_2(T x, T y);
		template <typename T>
		std::string to_string_3(T x, T y, T z);

	}

	extern template std::string internal::to_string_1(float);
	extern template std::string internal::to_string_1(double);
	extern template std::string internal::to_string_2(float, float);
	extern template std::string internal::to_string_2(double, double);
	extern template std::string internal::to_string_2(int, int);
	extern template std::string internal::to_string_3(float, float, float);
	extern template std::string internal::to_string_3(double, double, double);
	extern template std::string internal::to_string_3(int, int, int);

	template <typename T>
	using Vec1 = glm::vec<1, T, glm::defaultp>;

	template <typename T>
	using Vec2 = glm::vec<2, T, glm::defaultp>;

	template <typename T>
	using Vec3 = glm::vec<3, T, glm::defaultp>;
	
	template <typename T>
	using Vec4 = glm::vec<4, T, glm::defaultp>;

	using Vec1f = Vec1<FloatGLM>;
	using Vec1i = Vec1<int>;

	using Vec2f = Vec2<FloatGLM>;
	using Vec2i = Vec2<int>;
	
	using Vec3f = Vec3<FloatGLM>;
	using Vec3i = Vec3<int>;
	class Interval;
	using Vec3fi = Vec3<Interval>;
	
	using Vec4f = Vec4<FloatGLM>;
	using Vec4i = Vec4<int>;

	/// <summary>
	/// A Vec4f full of NaN values.
	/// </summary>
	constexpr Vec4f Vec4_NaN = Vec4f{ NaN, NaN, NaN, NaN };

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
		|| std::same_as<T, Vec3i>
		|| std::same_as<T, Vec3fi>;

	template <typename T>
	std::string to_string(const Vec1<T>& vector)
	{
		return internal::to_string_1(vector.x);
	}

	template <typename T>
	std::string to_string(const Vec2<T>& vector)
	{
		return internal::to_string_2(vector.x, vector.y);
	}

	template <typename T>
	std::string to_string(const Vec3<T>& vector)
	{
		return internal::to_string_3(vector.x, vector.y, vector.z);
	}

	namespace vector
	{
		template<typename T>
			requires is_vec<T>
		LOQUAT_CPU_GPU
		bool has_NaN(T vector) noexcept
		{
			for (glm::length_t i = 0; i < vector.length(); ++i)
			{
				bool NaN;
#ifdef LOQUAT_IS_GPU_CODE
				NaN = isnan(vector[i]);
#else
				NaN = std::isnan(vector[i]);
#endif
				if (NaN)
				{
					return true;
				}
			}
			return false;
		}
	}

	template <typename T>
		requires std::integral<T> || std::floating_point<T>
	LOQUAT_CPU_GPU
	inline T length_squared(Vec2<T> vector)
	{
		return vector.x * vector.x
			+ vector.y * vector.y;
	}

	template <typename T>
		requires std::integral<T> || std::floating_point<T>
	LOQUAT_CPU_GPU
	inline T length_squared(Vec3<T> vector)
	{
		return vector.x * vector.x
			+ vector.y * vector.y
			+ vector.z * vector.z;
	}

	template <typename T>
	struct Normal3 : public Vec3<T>
	{
		using loquat::Vec3<T>::x;
		using loquat::Vec3<T>::y;
		using loquat::Vec3<T>::z;
		using loquat::Vec3<T>::operator[];

		LOQUAT_CPU_GPU
			Normal3()
			: Vec3<T>()
		{}

		LOQUAT_CPU_GPU
			Normal3(T x)
			: Vec3<T>(x)
		{}

		LOQUAT_CPU_GPU
			Normal3(T x, T y, T z)
			: Vec3<T>(x, y, z)
		{}

		LOQUAT_CPU_GPU
			Normal3(glm::vec3 v)
			: Vec3<T>(v)
		{}

		template <typename U>
		LOQUAT_CPU_GPU
			explicit Normal3(Normal3<U> v)
			: Vec3<T>(T(v.x), T(v.y), T(v.z))
		{}

	};

	using Normal3f = Normal3<FloatGLM>;
	using Normal3i = Normal3<int>;

	/// <summary>
	/// Checks if a type is one of our normal types.
	/// </summary>
	template<typename T>
	concept is_normal =
		std::same_as<T, Normal3f>
		|| std::same_as<T, Normal3i>;

	namespace vector
	{
		template<typename T>
			requires is_normal<T>
		LOQUAT_CPU_GPU
		bool has_NaN(T normal) noexcept
		{
			for (glm::length_t i = 0; i < normal.length(); ++i)
			{
				bool NaN;
#ifdef LOQUAT_IS_GPU_CODE
				NaN = isnan(normal[i]);
#else
				NaN = std::isnan(normal[i]);
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

namespace std
{
	static std::ostream& operator<<(std::ostream& os, const loquat::Vec1f& v)
	{
		return os << loquat::to_string(v);
	}

	static std::ostream& operator<<(std::ostream& os, const loquat::Vec2f& v)
	{
		return os << loquat::to_string(v);
	}

	static std::ostream& operator<<(std::ostream& os, const loquat::Vec3f& v)
	{
		return os << loquat::to_string(v);
	}
}
