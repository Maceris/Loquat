// pbrt is Copyright(c) 1998-2020 Matt Pharr, Wenzel Jakob, and Greg Humphreys.
// The pbrt source code is licensed under the Apache License, Version 2.0.
// SPDX: Apache-2.0

// This file has been modified from the original, original notice is above.

#pragma once

#include <string>

#include "main/loquat.h"

#include "pbr/base/medium.h"
#include "pbr/math/vector_math.h"

namespace loquat
{
	class Ray
	{
	public:
		Point3f origin;
		Vec3f direction;
		Float time;
		Medium medium;

		Ray()
			: origin{ 0 }
			, direction{ 0 }
			, time{ 0.0f }
			, medium{ nullptr }
		{}
		LOQUAT_CPU_GPU
		Ray(const Point3f& origin, const Vec3f& direction,
			Float time = 0.0f, Medium medium = nullptr)
			: origin{ origin }
			, direction{ direction }
			, time{ time }
			, medium{ medium }
		{}
		LOQUAT_CPU_GPU
		Ray(const Ray& ray)
			: origin{ ray.origin }
			, direction{ ray.direction }
			, time{ ray.time }
			, medium{ ray.medium }
		{}
		LOQUAT_CPU_GPU
		Ray(Ray&& ray)
			: origin{ std::move(ray.origin) }
			, direction{ std::move(ray.direction) }
			, time{ std::move(ray.time) }
			, medium{ std::move(ray.medium) }
		{}

		LOQUAT_CPU_GPU
		Ray& operator=(const Ray& ray)
		{
			this->origin = ray.origin;
			this->direction = ray.direction;
			this->time = ray.time;
			this->medium = ray.medium;
			return *this;
		}
		LOQUAT_CPU_GPU
		Ray& operator=(Ray&& ray)
		{
			this->origin = std::move(ray.origin);
			this->direction = std::move(ray.direction);
			this->time = std::move(ray.time);
			this->medium = std::move(ray.medium);
			return *this;
		}

		LOQUAT_CPU_GPU
		Point3f operator()(Float t) const
		{
			return origin + direction * t;
		}

		[[nodiscard]]
		LOQUAT_CPU_GPU
		bool has_NaN() const noexcept
		{
			return vector::has_NaN(origin) || vector::has_NaN(direction);
		}

		template<typename A>
			requires std::convertible_to<A, Float>
		LOQUAT_CPU_GPU
		constexpr Ray& operator+=(A scalar)
		{
			this->origin += static_cast<Float>(scalar);
			return *this;
		}
		template<typename A>
			requires std::convertible_to<A, Float>
		LOQUAT_CPU_GPU
		constexpr Ray& operator-=(A scalar)
		{
			this->origin -= static_cast<Float>(scalar);
			return *this;
		}
		template<typename A>
			requires std::convertible_to<A, Float>
		LOQUAT_CPU_GPU
		constexpr Ray& operator*=(A scalar)
		{
			this->direction *= static_cast<Float>(scalar);
			return *this;
		}
		template<typename A>
			requires std::convertible_to<A, Float>
		LOQUAT_CPU_GPU
		constexpr Ray& operator/=(A scalar)
		{
			this->direction /= static_cast<Float>(scalar);
			return *this;
		}
	};

	[[nodiscard]]
	LOQUAT_CPU_GPU
	inline Point3f offset_ray_origin(Point3fi point_interval, Normal3f normal,
		Vec3f direction)
	{
		Float d = dot(loquat::abs(normal), point_interval.error());
		Vec3f offset = d * Vec3f(normal);
		if (dot(direction, normal) < 0)
		{
			offset = -offset;
		}
		Point3f new_origin = point_interval.to_vec() + offset;

		for (int i = 0; i < 3; ++i)
		{
			if (offset[i] > 0)
			{
				new_origin[i] = next_float_up(new_origin[i]);
			}
			else if (offset[i] < 0)
			{
				new_origin[i] = next_float_down(new_origin[i]);
			}
		}
		return new_origin;
	}

	[[nodiscard]]
	LOQUAT_CPU_GPU
	inline Ray spawn_ray(Point3fi point_interval, Normal3f normal, Float time,
		Vec3f direction) noexcept
	{
		return Ray(offset_ray_origin(point_interval, direction, normal),
			direction, time);
	}
	
	[[nodiscard]]
	LOQUAT_CPU_GPU
	inline Ray spawn_ray_to(Point3fi from, Normal3f normal, Float time, 
		Point3f to)
	{
		Vec3f direction = to - from.to_vec();
		return spawn_ray(from, normal, time, direction);
	}

	[[nodiscard]]
	LOQUAT_CPU_GPU
	inline Ray spawn_ray_to(Point3fi from, Normal3f normal_from, Float time,
		Point3fi to, Normal3f normal_to)
	{
		Point3f point_from = offset_ray_origin(from, normal_from, 
			to.to_vec() - from.to_vec());
		Point3f point_to = offset_ray_origin(to, normal_to, 
			point_from - to.to_vec());
		return Ray(point_from, point_to - point_from, time);
	}

	class RayDifferential : public Ray
	{
	public:
		RayDifferential() = default;
		LOQUAT_CPU_GPU
		RayDifferential(Point3f origin, Vec3f direction, Float time = 0.0f,
			Medium medium = nullptr)
			: Ray(origin, direction, time, medium)
		{}
		LOQUAT_CPU_GPU
		explicit RayDifferential(const Ray& ray)
			: Ray(ray)
		{}

		void scale_differentials(Float scale)
		{
			x_offset_origin = origin + (x_offset_origin - origin) * scale;
			y_offset_origin = origin + (y_offset_origin - origin) * scale;
			x_offset_direction = 
				direction + (x_offset_direction - direction) * scale;
			y_offset_direction = 
				direction + (y_offset_direction - direction) * scale;
		}

		[[nodiscard]]
		LOQUAT_CPU_GPU
		bool has_NaN() const noexcept
		{
			return Ray::has_NaN() || (has_differentials &&
				(vector::has_NaN(x_offset_origin)
					|| vector::has_NaN(y_offset_origin)
					|| vector::has_NaN(x_offset_direction)
					|| vector::has_NaN(y_offset_direction)));
		}

		[[nodiscard]]
		std::string to_string() const noexcept;

		bool has_differentials = false;
		Point3f x_offset_origin;
		Point3f y_offset_origin;
		Vec3f x_offset_direction;
		Vec3f y_offset_direction;
	};
}
