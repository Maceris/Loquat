#pragma once

#if !defined(_VECTOR_PARENT_INCLUDE_)
#include "debug/logger.h"
#include "pbr/math/Qualifier.h"
#endif

namespace loquat
{
	template <typename T, Qualifier Q>
	struct vec<2, T, Q>
	{
		using value_type = T;
		using type = vec<2, T, Q>;
		union
		{
			struct { T x, y; };
			struct { T r, g; };
			struct { T s, t; };
		};

		static constexpr unsigned int length()
		{
			return 2;
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
			}
		}

		constexpr vec()
			: x{ 0 }
			, y{ 0 }
		{}
		constexpr vec(vec const& v)
			: x{ v.x }
			, y{ v.y }
		{}
		template <Qualifier R>
		constexpr vec(vec<2, T, R> const& v)
			: x{ v.x }
			, y{ v.y }
		{}

		constexpr explicit vec(T scalar)
			: x{ scalar }
			, y{ scalar }
		{}
		constexpr vec(T x, T y)
			: x{ x }
			, y{ y }
		{}

		template <typename A, typename B>
			requires std::convertible_to<A, T>&& std::convertible_to<B, T>
		constexpr vec(A x, B y)
			: x{ static_cast<T>(x) }
			, y{ static_cast<T>(y) }
		{}
		template <typename A, typename B>
			requires std::convertible_to<A, T>&& std::convertible_to<B, T>
		constexpr vec(vec<1, A, Q> const& v, B y)
			: x{ static_cast<T>(v.x) }
			, y{ static_cast<T>(y) }
		{}
		template <typename A, typename B>
			requires std::convertible_to<A, T>&& std::convertible_to<B, T>
		constexpr vec(A x, vec<1, B, Q> const& v)
			: x{ static_cast<T>(x) }
			, y{ static_cast<T>(v.x) }
		{}
		template <typename A, typename B>
			requires std::convertible_to<A, T>&& std::convertible_to<B, T>
		constexpr vec(vec<1, A, Q> const& v1, vec<1, B, Q> const& v2)
			: x{ static_cast<T>(v1.x) }
			, y{ static_cast<T>(v2.x) }
		{}

		template <typename A, Qualifier R>
			requires std::convertible_to<A, T>
		constexpr explicit vec(vec<2, A, R> const& v)
			: x{ static_cast<T>(v.x) }
			, y{ static_cast<T>(v.y) }
		{}
		template <typename A, Qualifier R>
			requires std::convertible_to<A, T>
		constexpr explicit vec(vec<3, A, R> const& v)
			: x{ static_cast<T>(v.x) }
			, y{ static_cast<T>(v.y) }
		{}

		constexpr vec<2, T, Q>& operator=(vec const& v)
		{
			this->x = v.x;
			this->y = v.y;
			return *this;
		}

		template<typename A>
			requires std::convertible_to<A, T>
		constexpr vec<2, T, Q>& operator=(vec<2, A, Q> const& v)
		{
			this->x = static_cast<T>(v.x);
			this->y = static_cast<T>(v.y);
			return *this;
		}
		template<typename A>
			requires std::convertible_to<A, T>
		constexpr vec<2, T, Q>& operator+=(A scalar)
		{
			this->x += static_cast<T>(scalar);
			this->y += static_cast<T>(scalar);
			return *this;
		}
		template<typename A>
			requires std::convertible_to<A, T>
		constexpr vec<2, T, Q>& operator+=(vec<1, A, Q> const& v)
		{
			this->x += static_cast<T>(v.x);
			this->y += static_cast<T>(v.x);
			return *this;
		}
		template<typename A>
			requires std::convertible_to<A, T>
		constexpr vec<2, T, Q>& operator+=(vec<2, A, Q> const& v)
		{
			this->x += static_cast<T>(v.x);
			this->y += static_cast<T>(v.y);
			return *this;
		}
		template<typename A>
			requires std::convertible_to<A, T>
		constexpr vec<2, T, Q>& operator-=(A scalar)
		{
			this->x -= static_cast<T>(scalar);
			this->y -= static_cast<T>(scalar);
			return *this;
		}
		template<typename A>
			requires std::convertible_to<A, T>
		constexpr vec<2, T, Q>& operator-=(vec<1, A, Q> const& v)
		{
			this->x -= static_cast<T>(v.x);
			this->y -= static_cast<T>(v.x);
			return *this;
		}
		template<typename A>
			requires std::convertible_to<A, T>
		constexpr vec<2, T, Q>& operator-=(vec<2, A, Q> const& v)
		{
			this->x -= static_cast<T>(v.x);
			this->y -= static_cast<T>(v.y);
			return *this;
		}
		template<typename A>
			requires std::convertible_to<A, T>
		constexpr vec<2, T, Q>& operator*=(A scalar)
		{
			this->x *= static_cast<T>(scalar);
			this->y *= static_cast<T>(scalar);
			return *this;
		}
		template<typename A>
			requires std::convertible_to<A, T>
		constexpr vec<2, T, Q>& operator*=(vec<1, A, Q> const& v)
		{
			this->x *= static_cast<T>(v.x);
			this->y *= static_cast<T>(v.x);
			return *this;
		}
		template<typename A>
			requires std::convertible_to<A, T>
		constexpr vec<2, T, Q>& operator*=(vec<2, A, Q> const& v)
		{
			this->x *= static_cast<T>(v.x);
			this->y *= static_cast<T>(v.y);
			return *this;
		}
		template<typename A>
			requires std::convertible_to<A, T>
		constexpr vec<2, T, Q>& operator/=(A scalar)
		{
			this->x /= static_cast<T>(scalar);
			this->y /= static_cast<T>(scalar);
			return *this;
		}
		template<typename A>
			requires std::convertible_to<A, T>
		constexpr vec<2, T, Q>& operator/=(vec<1, A, Q> const& v)
		{
			this->x /= static_cast<T>(v.x);
			this->y /= static_cast<T>(v.x);
			return *this;
		}
		template<typename A>
			requires std::convertible_to<A, T>
		constexpr vec<2, T, Q>& operator/=(vec<2, A, Q> const& v)
		{
			this->x /= static_cast<T>(v.x);
			this->y /= static_cast<T>(v.y);
			return *this;
		}

		constexpr vec<2, T, Q>& operator++()
		{
			++this->x;
			++this->y;
			return *this;
		}
		constexpr vec<2, T, Q>& operator--()
		{
			--this->x;
			--this->y;
			return *this;
		}
		constexpr vec<2, T, Q> operator++(int)
		{
			vec<2, T, Q> result(*this);
			++*this;
			return result;
		}
		constexpr vec<2, T, Q> operator--(int)
		{
			vec<2, T, Q> result(*this);
			--*this;
			return result;
		}

		template<typename A>
			requires std::convertible_to<A, T>
		constexpr vec<2, T, Q>& operator%=(A scalar)
		{
			this->x %= static_cast<T>(scalar);
			this->y %= static_cast<T>(scalar);
			return *this;
		}
		template<typename A>
			requires std::convertible_to<A, T>
		constexpr vec<2, T, Q>& operator%=(vec<1, A, Q> const& v)
		{
			this->x %= static_cast<T>(v.x);
			this->y %= static_cast<T>(v.x);
			return *this;
		}
		template<typename A>
			requires std::convertible_to<A, T>
		constexpr vec<2, T, Q>& operator%=(vec<2, A, Q> const& v)
		{
			this->x %= static_cast<T>(v.x);
			this->y %= static_cast<T>(v.y);
			return *this;
		}
		template<typename A>
			requires std::convertible_to<A, T>
		constexpr vec<2, T, Q>& operator&=(A scalar)
		{
			this->x &= static_cast<T>(scalar);
			this->y &= static_cast<T>(scalar);
			return *this;
		}
		template<typename A>
			requires std::convertible_to<A, T>
		constexpr vec<2, T, Q>& operator&=(vec<1, A, Q> const& v)
		{
			this->x &= static_cast<T>(v.x);
			this->y &= static_cast<T>(v.x);
			return *this;
		}
		template<typename A>
			requires std::convertible_to<A, T>
		constexpr vec<2, T, Q>& operator&=(vec<2, A, Q> const& v)
		{
			this->x &= static_cast<T>(v.x);
			this->y &= static_cast<T>(v.y);
			return *this;
		}
		template<typename A>
			requires std::convertible_to<A, T>
		constexpr vec<2, T, Q>& operator|=(A scalar)
		{
			this->x |= static_cast<T>(scalar);
			this->y |= static_cast<T>(scalar);
			return *this;
		}
		template<typename A>
			requires std::convertible_to<A, T>
		constexpr vec<2, T, Q>& operator|=(vec<1, A, Q> const& v)
		{
			this->x |= static_cast<T>(v.x);
			this->y |= static_cast<T>(v.x);
			return *this;
		}
		template<typename A>
			requires std::convertible_to<A, T>
		constexpr vec<2, T, Q>& operator|=(vec<2, A, Q> const& v)
		{
			this->x |= static_cast<T>(v.x);
			this->y |= static_cast<T>(v.y);
			return *this;
		}
		template<typename A>
			requires std::convertible_to<A, T>
		constexpr vec<2, T, Q>& operator^=(A scalar)
		{
			this->x ^= static_cast<T>(scalar);
			this->y ^= static_cast<T>(scalar);
			return *this;
		}
		template<typename A>
			requires std::convertible_to<A, T>
		constexpr vec<2, T, Q>& operator^=(vec<1, A, Q> const& v)
		{
			this->x ^= static_cast<T>(v.x);
			this->y ^= static_cast<T>(v.x);
			return *this;
		}
		template<typename A>
			requires std::convertible_to<A, T>
		constexpr vec<2, T, Q>& operator^=(vec<2, A, Q> const& v)
		{
			this->x ^= static_cast<T>(v.x);
			this->y ^= static_cast<T>(v.y);
			return *this;
		}
		template<typename A>
			requires std::convertible_to<A, T>
		constexpr vec<2, T, Q>& operator<<=(A scalar)
		{
			this->x <<= static_cast<T>(scalar);
			this->y <<= static_cast<T>(scalar);
			return *this;
		}
		template<typename A>
			requires std::convertible_to<A, T>
		constexpr vec<2, T, Q>& operator<<=(vec<1, A, Q> const& v)
		{
			this->x <<= static_cast<T>(v.x);
			this->y <<= static_cast<T>(v.x);
			return *this;
		}
		template<typename A>
			requires std::convertible_to<A, T>
		constexpr vec<2, T, Q>& operator<<=(vec<2, A, Q> const& v)
		{
			this->x <<= static_cast<T>(v.x);
			this->y <<= static_cast<T>(v.y);
			return *this;
		}
		template<typename A>
			requires std::convertible_to<A, T>
		constexpr vec<2, T, Q>& operator>>=(A scalar)
		{
			this->x >>= static_cast<T>(scalar);
			this->y >>= static_cast<T>(scalar);
			return *this;
		}
		template<typename A>
			requires std::convertible_to<A, T>
		constexpr vec<2, T, Q>& operator>>=(vec<1, A, Q> const& v)
		{
			this->x >>= static_cast<T>(v.x);
			this->y >>= static_cast<T>(v.x);
			return *this;
		}
		template<typename A>
			requires std::convertible_to<A, T>
		constexpr vec<2, T, Q>& operator>>=(vec<2, A, Q> const& v)
		{
			this->x >>= static_cast<T>(v.x);
			this->y >>= static_cast<T>(v.y);
			return *this;
		}
	};

	template<typename T, Qualifier Q>
	constexpr vec<2, T, Q> operator+(vec<2, T, Q> const& v)
	{
		return v;
	}

	template<typename T, Qualifier Q>
	constexpr vec<2, T, Q> operator-(vec<2, T, Q> const& v)
	{
		return vec<2, T, Q>(-v.x, -v.y);
	}

	template<typename T, Qualifier Q>
	constexpr vec<2, T, Q> operator+(vec<2, T, Q> const& v, T scalar)
	{
		return vec<2, T, Q>(v.x + scalar, v.y + scalar);
	}

	template<typename T, Qualifier Q>
	constexpr vec<2, T, Q> operator+(vec<2, T, Q> const& v1,
		vec<1, T, Q> const& v2)
	{
		return vec<2, T, Q>(v1.x + v2.x, v1.y + v2.x);
	}

	template<typename T, Qualifier Q>
	constexpr vec<2, T, Q> operator+(T scalar, vec<2, T, Q> const& v)
	{
		return vec<2, T, Q>(scalar + v.x, scalar + v.y);
	}

	template<typename T, Qualifier Q>
	constexpr vec<2, T, Q> operator+(vec<1, T, Q> const& v1,
		vec<2, T, Q> const& v2)
	{
		return vec<2, T, Q>(v1.x + v2.x, v1.x + v2.y);
	}

	template<typename T, Qualifier Q>
	constexpr vec<2, T, Q> operator+(vec<2, T, Q> const& v1,
		vec<2, T, Q> const& v2)
	{
		return vec<2, T, Q>(v1.x + v2.x, v1.y + v2.y);
	}

	template<typename T, Qualifier Q>
	constexpr vec<2, T, Q> operator-(vec<2, T, Q> const& v, T scalar)
	{
		return vec<2, T, Q>(v.x - scalar, v.y - scalar);
	}

	template<typename T, Qualifier Q>
	constexpr vec<2, T, Q> operator-(vec<2, T, Q> const& v1,
		vec<1, T, Q> const& v2)
	{
		return vec<2, T, Q>(v1.x - v2.x, v1.y - v2.x);
	}

	template<typename T, Qualifier Q>
	constexpr vec<2, T, Q> operator-(T scalar, vec<2, T, Q> const& v)
	{
		return vec<2, T, Q>(scalar - v.x, scalar - v.y);
	}

	template<typename T, Qualifier Q>
	constexpr vec<2, T, Q> operator-(vec<1, T, Q> const& v1,
		vec<2, T, Q> const& v2)
	{
		return vec<2, T, Q>(v1.x - v2.x, v1.x - v2.y);
	}

	template<typename T, Qualifier Q>
	constexpr vec<2, T, Q> operator-(vec<2, T, Q> const& v1,
		vec<2, T, Q> const& v2)
	{
		return vec<2, T, Q>(v1.x - v2.x, v1.y - v2.y);
	}

	template<typename T, Qualifier Q>
	constexpr vec<2, T, Q> operator*(vec<2, T, Q> const& v, T scalar)
	{
		return vec<2, T, Q>(v.x * scalar, v.y * scalar);
	}

	template<typename T, Qualifier Q>
	constexpr vec<2, T, Q> operator*(vec<2, T, Q> const& v1,
		vec<1, T, Q> const& v2)
	{
		return vec<2, T, Q>(v1.x * v2.x, v1.y * v2.x);
	}

	template<typename T, Qualifier Q>
	constexpr vec<2, T, Q> operator*(T scalar, vec<2, T, Q> const& v)
	{
		return vec<2, T, Q>(scalar * v.x, scalar * v.y);
	}

	template<typename T, Qualifier Q>
	constexpr vec<2, T, Q> operator*(vec<1, T, Q> const& v1,
		vec<2, T, Q> const& v2)
	{
		return vec<2, T, Q>(v1.x * v2.x, v1.x * v2.y);
	}

	template<typename T, Qualifier Q>
	constexpr vec<2, T, Q> operator*(vec<2, T, Q> const& v1,
		vec<2, T, Q> const& v2)
	{
		return vec<2, T, Q>(v1.x * v2.x, v1.y * v2.y);
	}

	template<typename T, Qualifier Q>
	constexpr vec<2, T, Q> operator/(vec<2, T, Q> const& v, T scalar)
	{
		return vec<2, T, Q>(v.x / scalar, v.y / scalar);
	}

	template<typename T, Qualifier Q>
	constexpr vec<2, T, Q> operator/(vec<2, T, Q> const& v1,
		vec<1, T, Q> const& v2)
	{
		return vec<2, T, Q>(v1.x / v2.x, v1.y / v2.x);
	}

	template<typename T, Qualifier Q>
	constexpr vec<2, T, Q> operator/(T scalar, vec<2, T, Q> const& v)
	{
		return vec<2, T, Q>(scalar / v.x, scalar / v.y);
	}

	template<typename T, Qualifier Q>
	constexpr vec<2, T, Q> operator/(vec<1, T, Q> const& v1,
		vec<2, T, Q> const& v2)
	{
		return vec<2, T, Q>(v1.x / v2.x, v1.x / v2.y);
	}

	template<typename T, Qualifier Q>
	constexpr vec<2, T, Q> operator/(vec<2, T, Q> const& v1,
		vec<2, T, Q> const& v2)
	{
		return vec<2, T, Q>(v1.x / v2.x, v1.y / v2.y);
	}

	template<typename T, Qualifier Q>
	constexpr vec<2, T, Q> operator%(vec<2, T, Q> const& v, T scalar)
	{
		return vec<2, T, Q>(v.x % scalar, v.y % scalar);
	}

	template<typename T, Qualifier Q>
	constexpr vec<2, T, Q> operator%(vec<2, T, Q> const& v1,
		vec<1, T, Q> const& v2)
	{
		return vec<2, T, Q>(v1.x % v2.x, v1.y % v2.x);
	}

	template<typename T, Qualifier Q>
	constexpr vec<2, T, Q> operator%(T scalar, vec<2, T, Q> const& v)
	{
		return vec<2, T, Q>(scalar % v.x, scalar % v.y);
	}

	template<typename T, Qualifier Q>
	constexpr vec<2, T, Q> operator%(vec<1, T, Q> const& v1,
		vec<2, T, Q> const& v2)
	{
		return vec<2, T, Q>(v1.x % v2.x, v1.x % v2.y);
	}

	template<typename T, Qualifier Q>
	constexpr vec<2, T, Q> operator%(vec<2, T, Q> const& v1,
		vec<2, T, Q> const& v2)
	{
		return vec<2, T, Q>(v1.x % v2.x, v1.y % v2.y);
	}

	template<typename T, Qualifier Q>
	constexpr vec<2, T, Q> operator&(vec<2, T, Q> const& v, T scalar)
	{
		return vec<2, T, Q>(v.x & scalar, v.y & scalar);
	}

	template<typename T, Qualifier Q>
	constexpr vec<2, T, Q> operator&(vec<2, T, Q> const& v1,
		vec<1, T, Q> const& v2)
	{
		return vec<2, T, Q>(v1.x & v2.x, v1.y & v2.x);
	}

	template<typename T, Qualifier Q>
	constexpr vec<2, T, Q> operator&(T scalar, vec<2, T, Q> const& v)
	{
		return vec<2, T, Q>(scalar & v.x, scalar & v.y);
	}

	template<typename T, Qualifier Q>
	constexpr vec<2, T, Q> operator&(vec<1, T, Q> const& v1,
		vec<2, T, Q> const& v2)
	{
		return vec<2, T, Q>(v1.x & v2.x, v1.x & v2.y);
	}

	template<typename T, Qualifier Q>
	constexpr vec<2, T, Q> operator&(vec<2, T, Q> const& v1,
		vec<2, T, Q> const& v2)
	{
		return vec<2, T, Q>(v1.x & v2.x, v1.y & v2.y);
	}

	template<typename T, Qualifier Q>
	constexpr vec<2, T, Q> operator|(vec<2, T, Q> const& v, T scalar)
	{
		return vec<2, T, Q>(v.x | scalar, v.y | scalar);
	}

	template<typename T, Qualifier Q>
	constexpr vec<2, T, Q> operator|(vec<2, T, Q> const& v1,
		vec<1, T, Q> const& v2)
	{
		return vec<2, T, Q>(v1.x | v2.x, v1.y | v2.x);
	}

	template<typename T, Qualifier Q>
	constexpr vec<2, T, Q> operator|(T scalar, vec<2, T, Q> const& v)
	{
		return vec<2, T, Q>(scalar | v.x, scalar | v.y);
	}

	template<typename T, Qualifier Q>
	constexpr vec<2, T, Q> operator|(vec<1, T, Q> const& v1,
		vec<2, T, Q> const& v2)
	{
		return vec<2, T, Q>(v1.x | v2.x, v1.x | v2.y);
	}

	template<typename T, Qualifier Q>
	constexpr vec<2, T, Q> operator|(vec<2, T, Q> const& v1,
		vec<2, T, Q> const& v2)
	{
		return vec<2, T, Q>(v1.x | v2.x, v1.y | v2.y);
	}

	template<typename T, Qualifier Q>
	constexpr vec<2, T, Q> operator^(vec<2, T, Q> const& v, T scalar)
	{
		return vec<2, T, Q>(v.x ^ scalar, v.y ^ scalar);
	}

	template<typename T, Qualifier Q>
	constexpr vec<2, T, Q> operator^(vec<2, T, Q> const& v1,
		vec<1, T, Q> const& v2)
	{
		return vec<2, T, Q>(v1.x ^ v2.x, v1.y ^ v2.x);
	}

	template<typename T, Qualifier Q>
	constexpr vec<2, T, Q> operator^(T scalar, vec<2, T, Q> const& v)
	{
		return vec<2, T, Q>(scalar ^ v.x, scalar ^ v.y);
	}

	template<typename T, Qualifier Q>
	constexpr vec<2, T, Q> operator^(vec<1, T, Q> const& v1,
		vec<2, T, Q> const& v2)
	{
		return vec<2, T, Q>(v1.x ^ v2.x, v1.x ^ v2.y);
	}

	template<typename T, Qualifier Q>
	constexpr vec<2, T, Q> operator^(vec<2, T, Q> const& v1,
		vec<2, T, Q> const& v2)
	{
		return vec<2, T, Q>(v1.x ^ v2.x, v1.y ^ v2.y);
	}

	template<typename T, Qualifier Q>
	constexpr vec<2, T, Q> operator<<(vec<2, T, Q> const& v, T scalar)
	{
		return vec<2, T, Q>(static_cast<T>(v.x << scalar), 
			static_cast<T>(v.y << scalar));
	}

	template<typename T, Qualifier Q>
	constexpr vec<2, T, Q> operator<<(vec<2, T, Q> const& v1,
		vec<1, T, Q> const& v2)
	{
		return vec<2, T, Q>(static_cast<T>(v1.x << v2.x),
			static_cast<T>(v1.y << v2.x));
	}

	template<typename T, Qualifier Q>
	constexpr vec<2, T, Q> operator<<(T scalar, vec<2, T, Q> const& v)
	{
		return vec<2, T, Q>(static_cast<T>(scalar << v.x),
			static_cast<T>(scalar << v.y));
	}

	template<typename T, Qualifier Q>
	constexpr vec<2, T, Q> operator<<(vec<1, T, Q> const& v1,
		vec<2, T, Q> const& v2)
	{
		return vec<2, T, Q>(static_cast<T>(v1.x << v2.x),
			static_cast<T>(v1.x << v2.y));
	}

	template<typename T, Qualifier Q>
	constexpr vec<2, T, Q> operator<<(vec<2, T, Q> const& v1,
		vec<2, T, Q> const& v2)
	{
		return vec<2, T, Q>(static_cast<T>(v1.x << v2.x),
			static_cast<T>(v1.y << v2.y));
	}

	template<typename T, Qualifier Q>
	constexpr vec<2, T, Q> operator>>(vec<2, T, Q> const& v, T scalar)
	{
		return vec<2, T, Q>(static_cast<T>(v.x >> scalar),
			static_cast<T>(v.y >> scalar));
	}

	template<typename T, Qualifier Q>
	constexpr vec<2, T, Q> operator>>(vec<2, T, Q> const& v1,
		vec<1, T, Q> const& v2)
	{
		return vec<2, T, Q>(static_cast<T>(v1.x >> v2.x),
			static_cast<T>(v1.y >> v2.x));
	}

	template<typename T, Qualifier Q>
	constexpr vec<2, T, Q> operator>>(T scalar, vec<2, T, Q> const& v)
	{
		return vec<2, T, Q>(static_cast<T>(scalar >> v.x),
			static_cast<T>(scalar >> v.y));
	}

	template<typename T, Qualifier Q>
	constexpr vec<2, T, Q> operator>>(vec<1, T, Q> const& v1,
		vec<2, T, Q> const& v2)
	{
		return vec<2, T, Q>(static_cast<T>(v1.x >> v2.x),
			static_cast<T>(v1.x >> v2.y));
	}

	template<typename T, Qualifier Q>
	constexpr vec<2, T, Q> operator>>(vec<2, T, Q> const& v1,
		vec<2, T, Q> const& v2)
	{
		return vec<2, T, Q>(static_cast<T>(v1.x >> v2.x),
			static_cast<T>(v1.y >> v2.y));
	}

	template<typename T, Qualifier Q>
	constexpr vec<2, T, Q> operator~(vec<2, T, Q> const& v)
	{
		return vec<2, T, Q>(~v.x, ~v.y);
	}

	template<typename T, Qualifier Q>
	constexpr bool operator==(vec<2, T, Q> const& v1, vec<2, T, Q> const& v2)
	{
		return v1.x == v2.x && v1.y == v2.y;
	}

	template<typename T, Qualifier Q>
	constexpr bool operator!=(vec<2, T, Q> const& v1, vec<2, T, Q> const& v2)
	{
		return !(v1 == v2);
	}

	template<Qualifier Q>
	constexpr vec<2, bool, Q> operator&&(vec<2, bool, Q> const& v1,
		vec<2, bool, Q> const& v2)
	{
		return vec<2, bool, Q>(v1.x && v2.x, v1.y && v2.y);
	}

	template<Qualifier Q>
	constexpr vec<2, bool, Q> operator||(vec<2, bool, Q> const& v1,
		vec<2, bool, Q> const& v2)
	{
		return vec<2, bool, Q>(v1.x || v2.x, v1.y || v2.y);
	}
}