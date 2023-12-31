#pragma once

class Medium;

namespace loquat
{
	template <unsigned int Dims, typename T>
	struct Ray;

	template <typename T>
	using Ray2 = Ray<2, T>;

	template <typename T>
	using Ray3 = Ray<3, T>;

	template <unsigned int Dims, typename T>
	struct Ray
	{
		using PointType = Point<Dims, T>;
		using DirType = Vec<Dims, T>;

		PointType origin;
		DirType direction;
		Float time;
		Medium* medium;

		Ray()
			: origin{ 0 }
			, direction{ 0 }
			, time{ 0.0f }
			, medium { nullptr }
		{}
		Ray(PointType&& origin, DirType&& direction, Float time = 0.0f,
			Medium* medium = nullptr)
			: origin{ std::move(origin) }
			, direction{ std::move(direction) }
			, time{ time }
			, medium{ medium }
		{}
		Ray(const Ray& ray)
			: origin{ ray.origin }
			, direction{ ray.direction }
			, time{ ray.time }
			, medium{ ray.medium }
		{}
		Ray(Ray&& ray)
			: origin{ std::move(ray.origin) }
			, direction{ std::move(ray.direction) }
			, time{ std::move(ray.time) }
			, medium{ std::move(ray.medium) }
		{}
		Ray& operator=(const Ray& ray)
		{
			this->origin = ray.origin;
			this->direction = ray.direction;
			this->time = ray.time;
			this->medium = ray.medium;
			return *this;
		}
		Ray& operator=(Ray&& ray)
		{
			this->origin = std::move(ray.origin);
			this->direction = std::move(ray.direction);
			this->time = std::move(ray.time);
			this->medium = std::move(ray.medium);
			return *this;
		}
	};
}
