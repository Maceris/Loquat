#pragma once

class Medium;

namespace loquat
{
	template <typename PointType, typename DirType, typename BaseType>
		requires is_point<PointType> && is_vec<DirType>
	struct Ray;

	using Ray2i = Ray<Point2i, Vec2i, int>;
	using Ray2f = Ray<Point2f, Vec2f, Float>;
	
	using Ray3i = Ray<Point3i, Vec3i, int>;
	using Ray3f = Ray<Point3f, Vec3f, Float>;

	template <typename PointType, typename DirType, typename BaseType>
		requires is_point<PointType> && is_vec<DirType>
	struct Ray
	{
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
		Ray(const Ray<PointType, DirType, BaseType>& ray)
			: origin{ ray.origin }
			, direction{ ray.direction }
			, time{ ray.time }
			, medium{ ray.medium }
		{}
		Ray(Ray<PointType, DirType, BaseType>&& ray)
			: origin{ std::move(ray.origin) }
			, direction{ std::move(ray.direction) }
			, time{ std::move(ray.time) }
			, medium{ std::move(ray.medium) }
		{}
		Ray<PointType, DirType, BaseType>& operator=(
			const Ray<PointType, DirType, BaseType>& ray)
		{
			this->origin = ray.origin;
			this->direction = ray.direction;
			this->time = ray.time;
			this->medium = ray.medium;
			return *this;
		}
		Ray<PointType, DirType, BaseType>& operator=(
			Ray<PointType, DirType, BaseType>&& ray)
		{
			this->origin = std::move(ray.origin);
			this->direction = std::move(ray.direction);
			this->time = std::move(ray.time);
			this->medium = std::move(ray.medium);
			return *this;
		}

		template<typename A>
			requires std::convertible_to<A, BaseType>
		constexpr Ray<PointType, DirType, BaseType>& operator+=(A scalar)
		{
			this->origin += static_cast<BaseType>(scalar);
			return *this;
		}
		template<typename A>
			requires std::convertible_to<A, BaseType>
		constexpr Ray<PointType, DirType, BaseType>& operator-=(A scalar)
		{
			this->origin -= static_cast<BaseType>(scalar);
			return *this;
		}
		template<typename A>
			requires std::convertible_to<A, BaseType>
		constexpr Ray<PointType, DirType, BaseType>& operator*=(A scalar)
		{
			this->direction *= static_cast<BaseType>(scalar);
			return *this;
		}
		template<typename A>
			requires std::convertible_to<A, BaseType>
		constexpr Ray<PointType, DirType, BaseType>& operator/=(A scalar)
		{
			this->direction /= static_cast<BaseType>(scalar);
			return *this;
		}
	};
}
