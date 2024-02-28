// pbrt is Copyright(c) 1998-2020 Matt Pharr, Wenzel Jakob, and Greg Humphreys.
// The pbrt source code is licensed under the Apache License, Version 2.0.
// SPDX: Apache-2.0

// This file has been modified from the original, original notice is above.

#pragma once

#include <format>

#include "pbr/math/float.h"

namespace loquat
{
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

	template <typename T>
	using Normal3 = Vec3<T>;
	
	using Normal3f = Normal3<FloatGLM>;
	using Normal3i = Normal3<int>;

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

	namespace vector
	{
		template<typename T>
			requires is_vec<T>
		bool has_NaN(T vector) noexcept
		{
			for (glm::length_t i = 0; i < vector.length(); ++i)
			{
				if (is_NaN(vector[i]))
				{
					return true;
				}
			}
			return false;
		}

		template <typename T>
		inline T length_squared(Vec3<T> vector)
		{
			return square(vector.x) + square(vector.y) + square(vector.z);
		}

		template <typename T>
		std::string to_string(Vec1<T> vector)
		{
			return std::format("[ %s ]", vector.x);
		}

		template <typename T>
		std::string to_string(Vec2<T> vector)
		{
			return std::format("[ %s, %s ]", vector.x, vector.y);
		}

		template <typename T>
		std::string to_string(Vec3<T> vector)
		{
			return std::format("[ %s, %s, %s ]", vector.x, vector.y, vector.z);
		}
	}
	
}
