#pragma once

namespace loquat {

    template <typename T>
    using Point1 = Vec1<T>;

    template <typename T>
    using Point2 = Vec2<T>;

    template <typename T>
    using Point3 = Vec3<T>;

	using Point1f = Point1<Float>;
	using Point1i = Point1<int>;

	using Point2f = Point2<Float>;
	using Point2i = Point2<int>;

	using Point3f = Point3<Float>;
	using Point3i = Point3<int>;

	class Point3fi : public Vec3<Interval>
	{
	public:
		using Vec3<Interval>::x;
		using Vec3<Interval>::y;
		using Vec3<Interval>::z;
		using Vec3<Interval>::operator*=;

        Point3fi() = default;
        Point3fi(Interval x, Interval y, Interval z)
            : Vec3<Interval>{ x, y, z }
        {}
        Point3fi(Float x, Float y, Float z)
            : Vec3<Interval>{ Interval(x), Interval(y), Interval(z) }
        {}
        Point3fi(const Point3f& point)
            : Vec3<Interval>{
            Interval(point.x), Interval(point.y), Interval(point.z)}
        {}
        Point3fi(Vec3<Interval> point)
            : Vec3<Interval>{ point }
        {}
        Point3fi(Point3f point, Vec3f error)
            : Vec3<Interval>{ Interval::from_value_and_error(point.x, error.x),
                Interval::from_value_and_error(point.y, error.y),
                Interval::from_value_and_error(point.z, error.z)}
        {}

        Vec3f error() const noexcept
        {
            return {
                x.width() / 2,
                y.width() / 2,
                z.width() / 2
            };
        }
        
        bool is_exact() const noexcept 
        {
            return x.width() == 0 && y.width() == 0 && z.width() == 0;
        }

        [[nodiscard]]
        Vec3f to_vec() const noexcept
        {
            return Vec3f{ x.midpoint(), y.midpoint(), z.midpoint() };
        }
	};

	/// <summary>
	/// Checks if a type is one of our point types.
	/// </summary>
	template<typename T>
	concept is_point = 
		   std::same_as<T, Point1i> 
		|| std::same_as<T, Point1f>
		|| std::same_as<T, Point2i> 
		|| std::same_as<T, Point2f>
		|| std::same_as<T, Point3i>
		|| std::same_as<T, Point3f>
		|| std::same_as<T, Point3fi>;
}
