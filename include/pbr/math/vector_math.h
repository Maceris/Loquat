// pbrt is Copyright(c) 1998-2020 Matt Pharr, Wenzel Jakob, and Greg Humphreys.
// The pbrt source code is licensed under the Apache License, Version 2.0.
// SPDX: Apache-2.0

// This file has been modified from the original, original notice is above.

#pragma once

#include <format>

#include "main/loquat.h"
#include "pbr/math/float.h"
#include "pbr/math/vec.h"

namespace loquat
{
	/// <summary>
	/// Construct a local orthonormal coordinate system given a single
	/// normalized vector.
	/// </summary>
	/// <typeparam name="T">The type of vectors.</typeparam>
	/// <param name="v1">The normalized vector to start with.</param>
	/// <param name="v2">An out parameter for the second vector.</param>
	/// <param name="v3">An out parameter for the third vector.</param>
	template <typename T>
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

		Frame()
			: x{ 1, 0, 0 }
			, y{ 0, 1, 0 }
			, z{ 0, 0, 1 }
		{}
		Frame(Vec3f x, Vec3f y, Vec3f z);

		static Frame from_XZ(Vec3f x, Vec3f z) noexcept
		{
			return Frame{ x, glm::cross(z, x), z };
		}

		static Frame from_XY(Vec3f x, Vec3f y) noexcept
		{
			return Frame{ x, y, glm::cross(x, y) };
		}

		static Frame from_X(Vec3f x) noexcept
		{
			Vec3f y;
			Vec3f z;
			coordinate_system(x, &y, &z);
			return Frame{ x, y, z };
		}

		static Frame from_Y(Vec3f y) noexcept
		{
			Vec3f x;
			Vec3f z;
			coordinate_system(y, &z, &x);
			return Frame{ x, y, z };
		}

		static Frame from_Z(Vec3f z) noexcept
		{
			Vec3f x;
			Vec3f y;
			coordinate_system(z, &x, &z);
			return Frame{ x, y, z };
		}

		Vec3f to_local(Vec3f vector) const noexcept
		{
			return Vec3f{ glm::dot(vector, x), glm::dot(vector, y),
				glm::dot(vector, z) };
		}

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
	inline T absolute_dot(Vec2<T> v1, Vec2<T> v2)
	{
		LOG_ASSERT(!has_NaN(v1) && !has_NaN(v1) 
			&& "Calculating dot products of vectors with NaNs");
		return std::abs(glm::dot(v1, v2));
	}

	template <typename T>
	[[nodiscard]]
	inline T absolute_dot(Vec3<T> v1, Vec3<T> v2)
	{
		LOG_ASSERT(!vector::has_NaN(v1) && !vector::has_NaN(v1)
			&& "Calculating dot products of vectors with NaNs");
		return std::abs(glm::dot(v1, v2));
	}

	template <typename T>
	[[nodiscard]]
	inline Normal3<T> face_forward(Normal3<T> normal, Vec3<T> vector)
	{
		return glm::dot(normal, vector) < 0.0f ? -normal : normal;
	}

	class DirectionCone
	{
	public:
		DirectionCone() = default;

		DirectionCone(Vec3f direction, Float cos_theta) noexcept
			: direction{ glm::normalize(direction) }
			, cos_theta{ cos_theta }
		{}

		explicit DirectionCone(Vec3f direction) noexcept
			: DirectionCone(direction, 1)
		{}

		bool is_empty() const noexcept
		{
			return cos_theta == FLOAT_INFINITY;
		}

		static DirectionCone entire_sphere()
		{
			return DirectionCone{ Vec3f(0,0,1), -1 };
		}

		[[nodiscard]]
		std::string to_string() const noexcept;

		Vec3f closest_vector_in_cone(Vec3f vec) const noexcept;

		Vec3f direction;
		Float cos_theta = FLOAT_INFINITY;
	};
}