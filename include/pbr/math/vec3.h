#pragma once

#if !defined(_VECTOR_PARENT_INCLUDE_)
#include "debug/logger.h"
#include "pbr/math/Qualifier.h"
#endif

namespace loquat
{
	template <typename T, Qualifier Q>
	struct vec<3, T, Q>
	{
		using value_type = T;
		using type = vec<3, T, Q>;
		union
		{
			struct { T x, y, z; };
			struct { T r, g, b; };
			struct { T s, t, p; };
		};

		static constexpr unsigned int length()
		{
			return 3;
		}

		constexpr T& operator[](unsigned int i)
		{
			LOG_ASSERT(i >= 0 && i < this->length()
				&& "Index out of bounds");
			switch (i)
			{
			case 0:
			default:
				return x;
			case 1:
				return y;
			case 2:
				return z;
			}
		}
		constexpr T const& operator[](unsigned int i) const
		{
			LOG_ASSERT(i >= 0 && i < this->length()
				&& "Index out of bounds");
			switch (i)
			{
			case 0:
			default:
				return x;
			case 1:
				return y;
			case 2:
				return z;
			}
		}

		constexpr vec()
			: x{ 0 }
			, y{ 0 }
			, z{ 0 }
		{}
		constexpr vec(vec const& v)
			: x{ v.x }
			, y{ v.y }
			, z{ v.z }
		{}
		template <Qualifier R>
		constexpr vec(vec<3, T, R> const& v)
			: x{ v.x }
			, y{ v.y }
			, z{ v.z }
		{}

		constexpr explicit vec(T scalar)
			: x{ scalar }
			, y{ scalar }
			, z{ scalar }
		{}
		constexpr vec(T x, T y, T z)
			: x{ x }
			, y{ y }
			, z{ z }
		{}

		template<typename A, Qualifier Q>
			requires std::convertible_to<A, T>
		constexpr explicit vec(vec<1, A, Q> const& v)
			: x{ v.x }
			, y{ v.x }
			, z{ v.x }
		{}

		template <typename A, typename B, typename C>
			requires std::convertible_to<A, T>&& std::convertible_to<B, T>&& std::convertible_to<C, T>
		constexpr vec(A x, B y, C z)
			: x{ static_cast<T>(x) }
			, y{ static_cast<T>(y) }
			, z{ static_cast<T>(z) }
		{}

		template <typename A, typename B, typename C>
			requires std::convertible_to<A, T>&& std::convertible_to<B, T>&& std::convertible_to<C, T>
		constexpr vec(vec<1, A, Q> const& v, B y, C z)
			: x{ static_cast<T>(v.x) }
			, y{ static_cast<T>(y) }
			, z{ static_cast<T>(z) }
		{}

		template <typename A, typename B, typename C>
			requires std::convertible_to<A, T>&& std::convertible_to<B, T>&& std::convertible_to<C, T>
		constexpr vec(A x, vec<1, B, Q> const& v, C z)
			: x{ static_cast<T>(x) }
			, y{ static_cast<T>(v.x) }
			, z{ static_cast<T>(z) }
		{}

		template <typename A, typename B, typename C>
			requires std::convertible_to<A, T>&& std::convertible_to<B, T>&& std::convertible_to<C, T>
		constexpr vec(vec<1, A, Q> const& v1, vec<1, B, Q> const& v2, C z)
			: x{ static_cast<T>(v1.x) }
			, y{ static_cast<T>(v2.x) }
			, z{ static_cast<T>(z) }
		{}

		template <typename A, typename B, typename C>
			requires std::convertible_to<A, T>&& std::convertible_to<B, T>&& std::convertible_to<C, T>
		constexpr vec(A x, B y, vec<1, C, Q> const& v)
			: x{ static_cast<T>(x) }
			, y{ static_cast<T>(y) }
			, z{ static_cast<T>(v.x) }
		{}

		template <typename A, typename B, typename C>
			requires std::convertible_to<A, T>&& std::convertible_to<B, T>&& std::convertible_to<C, T>
		constexpr vec(vec<1, A, Q> const& v1, B y, vec<1, C, Q> const& v2)
			: x{ static_cast<T>(v1.x) }
			, y{ static_cast<T>(y) }
			, z{ static_cast<T>(v2.x) }
		{}

		template <typename A, typename B, typename C>
			requires std::convertible_to<A, T>&& std::convertible_to<B, T>&& std::convertible_to<C, T>
		constexpr vec(A x, vec<1, B, Q> const& v1, vec<1, C, Q> const& v2)
			: x{ static_cast<T>(x) }
			, y{ static_cast<T>(v1.x) }
			, z{ static_cast<T>(v2.x) }
		{}

		template <typename A, typename B, typename C>
			requires std::convertible_to<A, T>&& std::convertible_to<B, T>&& std::convertible_to<C, T>
		constexpr vec(vec<1, A, Q> const& v1, vec<1, B, Q> const& v2, 
			vec<1, C, Q> const& v3)
			: x{ static_cast<T>(v1.x) }
			, y{ static_cast<T>(v2.x) }
			, z{ static_cast<T>(v3.x) }
		{}


		template <typename A, typename B>
			requires std::convertible_to<A, T>&& std::convertible_to<B, T>
		constexpr vec(vec<2, A, Q> const& xy, B z)
			: x{ static_cast<T>(xy.x) }
			, y{ static_cast<T>(xy.y) }
			, z{ static_cast<T>(z) }
		{}

		template <typename A, typename B>
			requires std::convertible_to<A, T>&& std::convertible_to<B, T>
		constexpr vec(vec<2, A, Q> const& xy, vec<1, B, Q> const& z)
			: x{ static_cast<T>(xy.x) }
			, y{ static_cast<T>(xy.y) }
			, z{ static_cast<T>(z.x) }
		{}

		template <typename A, typename B>
			requires std::convertible_to<A, T>&& std::convertible_to<B, T>
		constexpr vec(A x, vec<2, B, Q> const& yz)
			: x{ static_cast<T>(x) }
			, y{ static_cast<T>(yz.x) }
			, z{ static_cast<T>(yz.y) }
		{}

		template <typename A, typename B>
			requires std::convertible_to<A, T>&& std::convertible_to<B, T>
		constexpr vec(vec<1, A, Q> const& x, vec<2, B, Q> const& yz)
			: x{ static_cast<T>(x.x) }
			, y{ static_cast<T>(yz.x) }
			, z{ static_cast<T>(yz.y) }
		{}

		template <typename A, Qualifier R>
			requires std::convertible_to<A, T>
		constexpr explicit vec(vec<3, A, R> const& v)
			: x{ static_cast<T>(v.x) }
			, y{ static_cast<T>(v.y) }
			, z{ static_cast<T>(v.z) }
		{}

		constexpr vec<3, T, Q>& operator=(vec<3, T, Q> const& v)
		{
			this->x = v.x;
			this->y = v.y;
			this->z = v.z;
			return *this;
		}

		template<typename A>
			requires std::convertible_to<A, T>
		constexpr vec<3, T, Q>& operator=(vec<3, A, Q> const& v)
		{
			this->x = static_cast<T>(v.x);
			this->y = static_cast<T>(v.y);
			this->z = static_cast<T>(v.z);
			return *this;
		}
		template<typename A>
			requires std::convertible_to<A, T>
		constexpr vec<3, T, Q>& operator+=(A scalar)
		{
			this->x += static_cast<T>(scalar);
			this->y += static_cast<T>(scalar);
			this->z += static_cast<T>(scalar);
			return *this;
		}
		template<typename A>
			requires std::convertible_to<A, T>
		constexpr vec<3, T, Q>& operator+=(vec<1, A, Q> const& v)
		{
			this->x += static_cast<T>(v.x);
			this->y += static_cast<T>(v.x);
			this->z += static_cast<T>(v.x);
			return *this;
		}
		template<typename A>
			requires std::convertible_to<A, T>
		constexpr vec<3, T, Q>& operator+=(vec<3, A, Q> const& v)
		{
			this->x += static_cast<T>(v.x);
			this->y += static_cast<T>(v.y);
			this->z += static_cast<T>(v.z);
			return *this;
		}
		template<typename A>
			requires std::convertible_to<A, T>
		constexpr vec<3, T, Q>& operator-=(A scalar)
		{
			this->x -= static_cast<T>(scalar);
			this->y -= static_cast<T>(scalar);
			this->z -= static_cast<T>(scalar);
			return *this;
		}
		template<typename A>
			requires std::convertible_to<A, T>
		constexpr vec<3, T, Q>& operator-=(vec<1, A, Q> const& v)
		{
			this->x -= static_cast<T>(v.x);
			this->y -= static_cast<T>(v.x);
			this->z -= static_cast<T>(v.x);
			return *this;
		}
		template<typename A>
			requires std::convertible_to<A, T>
		constexpr vec<3, T, Q>& operator-=(vec<3, A, Q> const& v)
		{
			this->x -= static_cast<T>(v.x);
			this->y -= static_cast<T>(v.y);
			this->z -= static_cast<T>(v.z);
			return *this;
		}
		template<typename A>
			requires std::convertible_to<A, T>
		constexpr vec<3, T, Q>& operator*=(A scalar)
		{
			this->x *= static_cast<T>(scalar);
			this->y *= static_cast<T>(scalar);
			this->z *= static_cast<T>(scalar);
			return *this;
		}
		template<typename A>
			requires std::convertible_to<A, T>
		constexpr vec<3, T, Q>& operator*=(vec<1, A, Q> const& v)
		{
			this->x *= static_cast<T>(v.x);
			this->y *= static_cast<T>(v.x);
			this->z *= static_cast<T>(v.x);
			return *this;
		}
		template<typename A>
			requires std::convertible_to<A, T>
		constexpr vec<3, T, Q>& operator*=(vec<3, A, Q> const& v)
		{
			this->x *= static_cast<T>(v.x);
			this->y *= static_cast<T>(v.y);
			this->z *= static_cast<T>(v.z);
			return *this;
		}
		template<typename A>
			requires std::convertible_to<A, T>
		constexpr vec<3, T, Q>& operator/=(A scalar)
		{
			this->x /= static_cast<T>(scalar);
			this->y /= static_cast<T>(scalar);
			this->z /= static_cast<T>(scalar);
			return *this;
		}
		template<typename A>
			requires std::convertible_to<A, T>
		constexpr vec<3, T, Q>& operator/=(vec<1, A, Q> const& v)
		{
			this->x /= static_cast<T>(v.x);
			this->y /= static_cast<T>(v.x);
			this->z /= static_cast<T>(v.x);
			return *this;
		}
		template<typename A>
			requires std::convertible_to<A, T>
		constexpr vec<3, T, Q>& operator/=(vec<3, A, Q> const& v)
		{
			this->x /= static_cast<T>(v.x);
			this->y /= static_cast<T>(v.y);
			this->z /= static_cast<T>(v.z);
			return *this;
		}

		constexpr vec<3, T, Q>& operator++()
		{
			++this->x;
			++this->y;
			++this->z;
			return *this;
		}
		constexpr vec<3, T, Q>& operator--()
		{
			--this->x;
			--this->y;
			--this->z;
			return *this;
		}
		constexpr vec<3, T, Q> operator++(int)
		{
			vec<3, T, Q> result(*this);
			++*this;
			return result;
		}
		constexpr vec<3, T, Q> operator--(int)
		{
			vec<3, T, Q> result(*this);
			--*this;
			return result;
		}

		template<typename A>
			requires std::convertible_to<A, T>
		constexpr vec<3, T, Q>& operator%=(A scalar)
		{
			this->x %= static_cast<T>(scalar);
			this->y %= static_cast<T>(scalar);
			this->z %= static_cast<T>(scalar);
			return *this;
		}
		template<typename A>
			requires std::convertible_to<A, T>
		constexpr vec<3, T, Q>& operator%=(vec<1, A, Q> const& v)
		{
			this->x %= static_cast<T>(v.x);
			this->y %= static_cast<T>(v.x);
			this->z %= static_cast<T>(v.x);
			return *this;
		}
		template<typename A>
			requires std::convertible_to<A, T>
		constexpr vec<3, T, Q>& operator%=(vec<3, A, Q> const& v)
		{
			this->x %= static_cast<T>(v.x);
			this->y %= static_cast<T>(v.y);
			this->z %= static_cast<T>(v.z);
			return *this;
		}
		template<typename A>
			requires std::convertible_to<A, T>
		constexpr vec<3, T, Q>& operator&=(A scalar)
		{
			this->x &= static_cast<T>(scalar);
			this->y &= static_cast<T>(scalar);
			this->z &= static_cast<T>(scalar);
			return *this;
		}
		template<typename A>
			requires std::convertible_to<A, T>
		constexpr vec<3, T, Q>& operator&=(vec<1, A, Q> const& v)
		{
			this->x &= static_cast<T>(v.x);
			this->y &= static_cast<T>(v.x);
			this->z &= static_cast<T>(v.x);
			return *this;
		}
		template<typename A>
			requires std::convertible_to<A, T>
		constexpr vec<3, T, Q>& operator&=(vec<3, A, Q> const& v)
		{
			this->x &= static_cast<T>(v.x);
			this->y &= static_cast<T>(v.y);
			this->z &= static_cast<T>(v.z);
			return *this;
		}
		template<typename A>
			requires std::convertible_to<A, T>
		constexpr vec<3, T, Q>& operator|=(A scalar)
		{
			this->x |= static_cast<T>(scalar);
			this->y |= static_cast<T>(scalar);
			this->z |= static_cast<T>(scalar);
			return *this;
		}
		template<typename A>
			requires std::convertible_to<A, T>
		constexpr vec<3, T, Q>& operator|=(vec<1, A, Q> const& v)
		{
			this->x |= static_cast<T>(v.x);
			this->y |= static_cast<T>(v.x);
			this->z |= static_cast<T>(v.x);
			return *this;
		}
		template<typename A>
			requires std::convertible_to<A, T>
		constexpr vec<3, T, Q>& operator|=(vec<3, A, Q> const& v)
		{
			this->x |= static_cast<T>(v.x);
			this->y |= static_cast<T>(v.y);
			this->z |= static_cast<T>(v.z);
			return *this;
		}
		template<typename A>
			requires std::convertible_to<A, T>
		constexpr vec<3, T, Q>& operator^=(A scalar)
		{
			this->x ^= static_cast<T>(scalar);
			this->y ^= static_cast<T>(scalar);
			this->z ^= static_cast<T>(scalar);
			return *this;
		}
		template<typename A>
			requires std::convertible_to<A, T>
		constexpr vec<3, T, Q>& operator^=(vec<1, A, Q> const& v)
		{
			this->x ^= static_cast<T>(v.x);
			this->y ^= static_cast<T>(v.x);
			this->z ^= static_cast<T>(v.x);
			return *this;
		}
		template<typename A>
			requires std::convertible_to<A, T>
		constexpr vec<3, T, Q>& operator^=(vec<3, A, Q> const& v)
		{
			this->x ^= static_cast<T>(v.x);
			this->y ^= static_cast<T>(v.y);
			this->z ^= static_cast<T>(v.z);
			return *this;
		}
		template<typename A>
			requires std::convertible_to<A, T>
		constexpr vec<3, T, Q>& operator<<=(A scalar)
		{
			this->x <<= static_cast<T>(scalar);
			this->y <<= static_cast<T>(scalar);
			this->z <<= static_cast<T>(scalar);
			return *this;
		}
		template<typename A>
			requires std::convertible_to<A, T>
		constexpr vec<3, T, Q>& operator<<=(vec<1, A, Q> const& v)
		{
			this->x <<= static_cast<T>(v.x);
			this->y <<= static_cast<T>(v.x);
			this->z <<= static_cast<T>(v.x);
			return *this;
		}
		template<typename A>
			requires std::convertible_to<A, T>
		constexpr vec<3, T, Q>& operator<<=(vec<3, A, Q> const& v)
		{
			this->x <<= static_cast<T>(v.x);
			this->y <<= static_cast<T>(v.y);
			this->z <<= static_cast<T>(v.z);
			return *this;
		}
		template<typename A>
			requires std::convertible_to<A, T>
		constexpr vec<3, T, Q>& operator>>=(A scalar)
		{
			this->x >>= static_cast<T>(scalar);
			this->y >>= static_cast<T>(scalar);
			this->z >>= static_cast<T>(scalar);
			return *this;
		}
		template<typename A>
			requires std::convertible_to<A, T>
		constexpr vec<3, T, Q>& operator>>=(vec<1, A, Q> const& v)
		{
			this->x >>= static_cast<T>(v.x);
			this->y >>= static_cast<T>(v.x);
			this->z >>= static_cast<T>(v.x);
			return *this;
		}
		template<typename A>
			requires std::convertible_to<A, T>
		constexpr vec<3, T, Q>& operator>>=(vec<3, A, Q> const& v)
		{
			this->x >>= static_cast<T>(v.x);
			this->y >>= static_cast<T>(v.y);
			this->z >>= static_cast<T>(v.z);
			return *this;
		}
	};

	template<typename T, Qualifier Q>
	constexpr vec<3, T, Q> operator+(vec<3, T, Q> const& v)
	{
		return v;
	}

	template<typename T, Qualifier Q>
	constexpr vec<3, T, Q> operator-(vec<3, T, Q> const& v)
	{
		return vec<3, T, Q>(-v.x, -v.y, -v.z);
	}

	template<typename T, Qualifier Q>
	constexpr vec<3, T, Q> operator+(vec<3, T, Q> const& v, T scalar)
	{
		return vec<3, T, Q>(
			v.x + scalar,
			v.y + scalar,
			v.z + scalar
		);
	}

	template<typename T, Qualifier Q>
	constexpr vec<3, T, Q> operator+(vec<3, T, Q> const& v1,
		vec<1, T, Q> const& v2)
	{
		return vec<3, T, Q>(
			v1.x + v2.x,
			v1.y + v2.x,
			v1.z + v2.x
		);
	}

	template<typename T, Qualifier Q>
	constexpr vec<3, T, Q> operator+(T scalar, vec<3, T, Q> const& v)
	{
		return vec<3, T, Q>(
			scalar + v.x,
			scalar + v.y,
			scalar + v.z
		);
	}

	template<typename T, Qualifier Q>
	constexpr vec<3, T, Q> operator+(vec<1, T, Q> const& v1,
		vec<3, T, Q> const& v2)
	{
		return vec<3, T, Q>(
			v1.x + v2.x,
			v1.x + v2.y,
			v1.x + v2.z
		);
	}

	template<typename T, Qualifier Q>
	constexpr vec<3, T, Q> operator+(vec<3, T, Q> const& v1,
		vec<3, T, Q> const& v2)
	{
		return vec<3, T, Q>(
			v1.x + v2.x,
			v1.y + v2.y,
			v1.z + v2.z
		);
	}

	template<typename T, Qualifier Q>
	constexpr vec<3, T, Q> operator-(vec<3, T, Q> const& v, T scalar)
	{
		return vec<3, T, Q>(
			v.x - scalar,
			v.y - scalar,
			v.z - scalar
		);
	}

	template<typename T, Qualifier Q>
	constexpr vec<3, T, Q> operator-(vec<3, T, Q> const& v1,
		vec<1, T, Q> const& v2)
	{
		return vec<3, T, Q>(
			v1.x - v2.x,
			v1.y - v2.x,
			v1.z - v2.x
		);
	}

	template<typename T, Qualifier Q>
	constexpr vec<3, T, Q> operator-(T scalar, vec<3, T, Q> const& v)
	{
		return vec<3, T, Q>(
			scalar - v.x,
			scalar - v.y,
			scalar - v.z
		);
	}

	template<typename T, Qualifier Q>
	constexpr vec<3, T, Q> operator-(vec<1, T, Q> const& v1,
		vec<3, T, Q> const& v2)
	{
		return vec<3, T, Q>(
			v1.x - v2.x,
			v1.x - v2.y,
			v1.x - v2.z
		);
	}

	template<typename T, Qualifier Q>
	constexpr vec<3, T, Q> operator-(vec<3, T, Q> const& v1,
		vec<3, T, Q> const& v2)
	{
		return vec<3, T, Q>(
			v1.x - v2.x,
			v1.y - v2.y,
			v1.z - v2.z
		);
	}

	template<typename T, Qualifier Q>
	constexpr vec<3, T, Q> operator*(vec<3, T, Q> const& v, T scalar)
	{
		return vec<3, T, Q>(
			v.x * scalar,
			v.y * scalar,
			v.z * scalar
		);
	}

	template<typename T, Qualifier Q>
	constexpr vec<3, T, Q> operator*(vec<3, T, Q> const& v1,
		vec<1, T, Q> const& v2)
	{
		return vec<3, T, Q>(
			v1.x * v2.x,
			v1.y * v2.x,
			v1.z * v2.x
		);
	}

	template<typename T, Qualifier Q>
	constexpr vec<3, T, Q> operator*(T scalar, vec<3, T, Q> const& v)
	{
		return vec<3, T, Q>(
			scalar * v.x,
			scalar * v.y,
			scalar * v.z
		);
	}

	template<typename T, Qualifier Q>
	constexpr vec<3, T, Q> operator*(vec<1, T, Q> const& v1,
		vec<3, T, Q> const& v2)
	{
		return vec<3, T, Q>(
			v1.x * v2.x,
			v1.x * v2.y,
			v1.x * v2.z
		);
	}

	template<typename T, Qualifier Q>
	constexpr vec<3, T, Q> operator*(vec<3, T, Q> const& v1,
		vec<3, T, Q> const& v2)
	{
		return vec<3, T, Q>(
			v1.x * v2.x,
			v1.y * v2.y,
			v1.z * v2.z
		);
	}

	template<typename T, Qualifier Q>
	constexpr vec<3, T, Q> operator/(vec<3, T, Q> const& v, T scalar)
	{
		return vec<3, T, Q>(
			v.x / scalar,
			v.y / scalar,
			v.z / scalar
		);
	}

	template<typename T, Qualifier Q>
	constexpr vec<3, T, Q> operator/(vec<3, T, Q> const& v1,
		vec<1, T, Q> const& v2)
	{
		return vec<3, T, Q>(
			v1.x / v2.x,
			v1.y / v2.x,
			v1.z / v2.x
		);
	}

	template<typename T, Qualifier Q>
	constexpr vec<3, T, Q> operator/(T scalar, vec<3, T, Q> const& v)
	{
		return vec<3, T, Q>(
			scalar / v.x,
			scalar / v.y,
			scalar / v.z
		);
	}

	template<typename T, Qualifier Q>
	constexpr vec<3, T, Q> operator/(vec<1, T, Q> const& v1,
		vec<3, T, Q> const& v2)
	{
		return vec<3, T, Q>(
			v1.x / v2.x,
			v1.x / v2.y,
			v1.x / v2.z
		);
	}

	template<typename T, Qualifier Q>
	constexpr vec<3, T, Q> operator/(vec<3, T, Q> const& v1,
		vec<3, T, Q> const& v2)
	{
		return vec<3, T, Q>(
			v1.x / v2.x,
			v1.y / v2.y,
			v1.z / v2.z
		);
	}

	template<typename T, Qualifier Q>
	constexpr vec<3, T, Q> operator%(vec<3, T, Q> const& v, T scalar)
	{
		return vec<3, T, Q>(
			v.x % scalar,
			v.y % scalar,
			v.z % scalar
		);
	}

	template<typename T, Qualifier Q>
	constexpr vec<3, T, Q> operator%(vec<3, T, Q> const& v1,
		vec<1, T, Q> const& v2)
	{
		return vec<3, T, Q>(
			v1.x % v2.x,
			v1.y % v2.x,
			v1.z % v2.x
		);
	}

	template<typename T, Qualifier Q>
	constexpr vec<3, T, Q> operator%(T scalar, vec<3, T, Q> const& v)
	{
		return vec<3, T, Q>(
			scalar % v.x,
			scalar % v.y,
			scalar % v.z
		);
	}

	template<typename T, Qualifier Q>
	constexpr vec<3, T, Q> operator%(vec<1, T, Q> const& v1,
		vec<3, T, Q> const& v2)
	{
		return vec<3, T, Q>(
			v1.x % v2.x,
			v1.x % v2.y,
			v1.x % v2.z
		);
	}

	template<typename T, Qualifier Q>
	constexpr vec<3, T, Q> operator%(vec<3, T, Q> const& v1,
		vec<3, T, Q> const& v2)
	{
		return vec<3, T, Q>(
			v1.x % v2.x,
			v1.y % v2.y,
			v1.z % v2.z
		);
	}

	template<typename T, Qualifier Q>
	constexpr vec<3, T, Q> operator&(vec<3, T, Q> const& v, T scalar)
	{
		return vec<3, T, Q>(
			v.x & scalar,
			v.y & scalar,
			v.z & scalar
		);
	}

	template<typename T, Qualifier Q>
	constexpr vec<3, T, Q> operator&(vec<3, T, Q> const& v1,
		vec<1, T, Q> const& v2)
	{
		return vec<3, T, Q>(
			v1.x & v2.x,
			v1.y & v2.x,
			v1.z & v2.x
		);
	}

	template<typename T, Qualifier Q>
	constexpr vec<3, T, Q> operator&(T scalar, vec<3, T, Q> const& v)
	{
		return vec<3, T, Q>(
			scalar & v.x,
			scalar & v.y,
			scalar & v.z
		);
	}

	template<typename T, Qualifier Q>
	constexpr vec<3, T, Q> operator&(vec<1, T, Q> const& v1,
		vec<3, T, Q> const& v2)
	{
		return vec<3, T, Q>(
			v1.x & v2.x,
			v1.x & v2.y,
			v1.x & v2.z
		);
	}

	template<typename T, Qualifier Q>
	constexpr vec<3, T, Q> operator&(vec<3, T, Q> const& v1,
		vec<3, T, Q> const& v2)
	{
		return vec<3, T, Q>(
			v1.x & v2.x,
			v1.y & v2.y,
			v1.z & v2.z
		);
	}

	template<typename T, Qualifier Q>
	constexpr vec<3, T, Q> operator|(vec<3, T, Q> const& v, T scalar)
	{
		return vec<3, T, Q>(
			v.x | scalar,
			v.y | scalar,
			v.z | scalar
		);
	}

	template<typename T, Qualifier Q>
	constexpr vec<3, T, Q> operator|(vec<3, T, Q> const& v1,
		vec<1, T, Q> const& v2)
	{
		return vec<3, T, Q>(
			v1.x | v2.x,
			v1.y | v2.x,
			v1.z | v2.x
		);
	}

	template<typename T, Qualifier Q>
	constexpr vec<3, T, Q> operator|(T scalar, vec<3, T, Q> const& v)
	{
		return vec<3, T, Q>(
			scalar | v.x,
			scalar | v.y,
			scalar | v.z
		);
	}

	template<typename T, Qualifier Q>
	constexpr vec<3, T, Q> operator|(vec<1, T, Q> const& v1,
		vec<3, T, Q> const& v2)
	{
		return vec<3, T, Q>(
			v1.x | v2.x,
			v1.x | v2.y,
			v1.x | v2.z
		);
	}

	template<typename T, Qualifier Q>
	constexpr vec<3, T, Q> operator|(vec<3, T, Q> const& v1,
		vec<3, T, Q> const& v2)
	{
		return vec<3, T, Q>(
			v1.x | v2.x,
			v1.y | v2.y,
			v1.z | v2.z
		);
	}

	template<typename T, Qualifier Q>
	constexpr vec<3, T, Q> operator^(vec<3, T, Q> const& v, T scalar)
	{
		return vec<3, T, Q>(
			v.x ^ scalar,
			v.y ^ scalar,
			v.z ^ scalar
		);
	}

	template<typename T, Qualifier Q>
	constexpr vec<3, T, Q> operator^(vec<3, T, Q> const& v1,
		vec<1, T, Q> const& v2)
	{
		return vec<3, T, Q>(
			v1.x ^ v2.x,
			v1.y ^ v2.x,
			v1.z ^ v2.x
		);
	}

	template<typename T, Qualifier Q>
	constexpr vec<3, T, Q> operator^(T scalar, vec<3, T, Q> const& v)
	{
		return vec<3, T, Q>(
			scalar ^ v.x,
			scalar ^ v.y,
			scalar ^ v.z
		);
	}

	template<typename T, Qualifier Q>
	constexpr vec<3, T, Q> operator^(vec<1, T, Q> const& v1,
		vec<3, T, Q> const& v2)
	{
		return vec<3, T, Q>(
			v1.x ^ v2.x,
			v1.x ^ v2.y,
			v1.x ^ v2.z
		);
	}

	template<typename T, Qualifier Q>
	constexpr vec<3, T, Q> operator^(vec<3, T, Q> const& v1,
		vec<3, T, Q> const& v2)
	{
		return vec<3, T, Q>(
			v1.x ^ v2.x,
			v1.y ^ v2.y,
			v1.z ^ v2.z
		);
	}

	template<typename T, Qualifier Q>
	constexpr vec<3, T, Q> operator<<(vec<3, T, Q> const& v, T scalar)
	{
		return vec<3, T, Q>(
			static_cast<T>(v.x << scalar),
			static_cast<T>(v.y << scalar),
			static_cast<T>(v.z << scalar)
		);
	}

	template<typename T, Qualifier Q>
	constexpr vec<3, T, Q> operator<<(vec<3, T, Q> const& v1,
		vec<1, T, Q> const& v2)
	{
		return vec<3, T, Q>(
			static_cast<T>(v1.x << v2.x),
			static_cast<T>(v1.y << v2.x),
			static_cast<T>(v1.z << v2.x)
		);
	}

	template<typename T, Qualifier Q>
	constexpr vec<3, T, Q> operator<<(T scalar, vec<3, T, Q> const& v)
	{
		return vec<3, T, Q>(
			static_cast<T>(scalar << v.x),
			static_cast<T>(scalar << v.y),
			static_cast<T>(scalar << v.z)
		);
	}

	template<typename T, Qualifier Q>
	constexpr vec<3, T, Q> operator<<(vec<1, T, Q> const& v1,
		vec<3, T, Q> const& v2)
	{
		return vec<3, T, Q>(
			static_cast<T>(v1.x << v2.x),
			static_cast<T>(v1.x << v2.y),
			static_cast<T>(v1.x << v2.z)
		);
	}

	template<typename T, Qualifier Q>
	constexpr vec<3, T, Q> operator<<(vec<3, T, Q> const& v1,
		vec<3, T, Q> const& v2)
	{
		return vec<3, T, Q>(
			static_cast<T>(v1.x << v2.x),
			static_cast<T>(v1.y << v2.y),
			static_cast<T>(v1.z << v2.z)
		);
	}

	template<typename T, Qualifier Q>
	constexpr vec<3, T, Q> operator>>(vec<3, T, Q> const& v, T scalar)
	{
		return vec<3, T, Q>(
			static_cast<T>(v.x >> scalar),
			static_cast<T>(v.y >> scalar),
			static_cast<T>(v.z >> scalar)
		);
	}

	template<typename T, Qualifier Q>
	constexpr vec<3, T, Q> operator>>(vec<3, T, Q> const& v1,
		vec<1, T, Q> const& v2)
	{
		return vec<3, T, Q>(
			static_cast<T>(v1.x >> v2.x),
			static_cast<T>(v1.y >> v2.x),
			static_cast<T>(v1.z >> v2.x)
		);
	}

	template<typename T, Qualifier Q>
	constexpr vec<3, T, Q> operator>>(T scalar, vec<3, T, Q> const& v)
	{
		return vec<3, T, Q>(
			static_cast<T>(scalar >> v.x),
			static_cast<T>(scalar >> v.y),
			static_cast<T>(scalar >> v.z)
		);
	}

	template<typename T, Qualifier Q>
	constexpr vec<3, T, Q> operator>>(vec<1, T, Q> const& v1,
		vec<3, T, Q> const& v2)
	{
		return vec<3, T, Q>(
			static_cast<T>(v1.x >> v2.x),
			static_cast<T>(v1.x >> v2.y),
			static_cast<T>(v1.x >> v2.z)
		);
	}

	template<typename T, Qualifier Q>
	constexpr vec<3, T, Q> operator>>(vec<3, T, Q> const& v1,
		vec<3, T, Q> const& v2)
	{
		return vec<3, T, Q>(
			static_cast<T>(v1.x >> v2.x),
			static_cast<T>(v1.y >> v2.y),
			static_cast<T>(v1.z >> v2.z)
		);
	}

	template<typename T, Qualifier Q>
	constexpr vec<3, T, Q> operator~(vec<3, T, Q> const& v)
	{
		return vec<3, T, Q>(~v.x, ~v.y, ~v.z);
	}

	template<typename T, Qualifier Q>
	constexpr bool operator==(vec<3, T, Q> const& v1, vec<3, T, Q> const& v2)
	{
		return v1.x == v2.x && v1.y == v2.y && v1.z == v2.z;
	}

	template<typename T, Qualifier Q>
	constexpr bool operator!=(vec<3, T, Q> const& v1, vec<3, T, Q> const& v2)
	{
		return !(v1 == v2);
	}

	template<Qualifier Q>
	constexpr vec<3, bool, Q> operator&&(vec<3, bool, Q> const& v1,
		vec<3, bool, Q> const& v2)
	{
		return vec<3, bool, Q>(v1.x && v2.x, v1.y && v2.y, v1.z && v2.z);
	}

	template<Qualifier Q>
	constexpr vec<3, bool, Q> operator||(vec<3, bool, Q> const& v1,
		vec<3, bool, Q> const& v2)
	{
		return vec<3, bool, Q>(v1.x || v2.x, v1.y || v2.y, v1.z || v2.z);
	}
}