#pragma once

namespace loquat
{
	template <typename PointType, typename DirType, typename BaseType>
		requires is_point<PointType>&& is_vec<DirType>
	struct Ray
	{
	public:
		PointType origin;
		DirType direction;
		Float time;
		Medium medium;

		Ray()
			: origin{ 0 }
			, direction{ 0 }
			, time{ 0.0f }
			, medium{ nullptr }
		{}
		Ray(const PointType& origin, const DirType& direction,
			Float time = 0.0f, Medium medium = nullptr)
			: origin{ origin }
			, direction{ direction }
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

		[[nodiscard]]
		bool has_NaN() const noexcept
		{
			return vector::has_NaN(origin) || vector::has_NaN(direction);
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

	using Ray2i = Ray<Point2i, Vec2i, int>;
	using Ray2f = Ray<Point2f, Vec2f, Float>;
	
	using Ray3i = Ray<Point3i, Vec3i, int>;
	using Ray3f = Ray<Point3f, Vec3f, Float>;

	[[nodiscard]]
	inline Point3f offset_ray_origin(Point3fi point_interval, Normal3f normal,
		Vec3f direction)
	{
		Float d = glm::dot(glm::abs(normal), point_interval.error());
		Vec3f offset = d * Vec3f(normal);
		if (glm::dot(direction, normal) < 0)
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
	inline Ray3f spawn_ray(Point3fi point_interval, Normal3f normal, Float time,
		Vec3f direction) noexcept
	{
		return Ray3f(offset_ray_origin(point_interval, direction, normal),
			direction, time);
	}
	
	[[nodiscard]]
	inline Ray3f spawn_ray_to(Point3fi from, Normal3f normal, Float time, 
		Point3f to)
	{
		Vec3f direction = to - from.to_vec();
		return spawn_ray(from, normal, time, direction);
	}

	[[nodiscard]]
	inline Ray3f spawn_ray_to(Point3fi from, Normal3f normal_from, Float time,
		Point3fi to, Normal3f normal_to)
	{
		Point3f point_from = offset_ray_origin(from, normal_from, 
			to.to_vec() - from.to_vec());
		Point3f point_to = offset_ray_origin(to, normal_to, 
			point_from - to.to_vec());
		return Ray3f(point_from, point_to - point_from, time);
	}

	class RayDifferential : public Ray3f
	{
	public:
		RayDifferential() = default;
		RayDifferential(Point3f origin, Vec3f direction, Float time = 0.0f,
			Medium medium = nullptr)
			: Ray3f(origin, direction, time, medium)
		{}
		explicit RayDifferential(const Ray3f& ray)
			: Ray3f(ray)
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
		bool has_NaN() const noexcept
		{
			return Ray3f::has_NaN() || (has_differentials &&
				(vector::has_NaN(x_offset_origin)
					|| vector::has_NaN(y_offset_origin)
					|| vector::has_NaN(x_offset_direction)
					|| vector::has_NaN(y_offset_direction)));
		}
		std::string to_string() const noexcept;

		bool has_differentials = false;
		Point3f x_offset_origin;
		Point3f y_offset_origin;
		Vec3f x_offset_direction;
		Vec3f y_offset_direction;
	};
}
