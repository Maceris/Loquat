#pragma once

#if !defined(_VECTOR_PARENT_INCLUDE_)
#include "debug/logger.h"
#include "pbr/math/Qualifier.h"
#endif

namespace loquat
{
	template <typename T, Qualifier Q>
	struct vec<1, T, Q>
	{
		union
		{
			T x;
			T r;
			T s;
		};

		static constexpr unsigned int length()
		{
			return 1;
		}

		constexpr T& operator[](unsigned int i)
		{
			LOG_ASSERT(i >= 0 && i < this->length()
				&& "Index out of bounds");
			return x;
		}

		constexpr T const& operator[](unsigned int i) const
		{
			LOG_ASSERT(i >= 0 && i < this->length()
				&& "Index out of bounds");
			return x;
		}

		constexpr vec()
			: x{ 0 }
		{}
		constexpr vec(vec const& v)
			: x{ v.x }
		{}
		template <Qualifier R>
		constexpr vec(vec<1, T, R> const& v)
			: x{ v.x }
		{}

		constexpr explicit vec(T scalar)
			: x{ scalar }
		{}

		template <typename A>
			requires std::convertible_to<A, T>
		constexpr vec(A x)
			: x{ static_cast<T>(x) }
		{}

		template <typename A, Qualifier R>
			requires std::convertible_to<A, T>
		constexpr vec(vec<2, A, R> const& v)
			: x{ static_cast<T>(v.x) }
		{}
		template <typename A, Qualifier R>
			requires std::convertible_to<A, T>
		constexpr vec(vec<3, A, R> const& v)
			: x{ static_cast<T>(v.x) }
		{}

		constexpr vec<1, T, Q>& operator=(vec const& v)
		{
			this->x = v.x;
			return *this;
		}

		template<typename A>
			requires std::convertible_to<A, T>
		constexpr vec<1, T, Q>& operator=(vec<1, A, Q> const& v)
		{
			this->x = static_cast<T>(v.x);
			return *this;
		}
		template<typename A>
			requires std::convertible_to<A, T>
		constexpr vec<1, T, Q>& operator+=(A scalar)
		{
			this->x += static_cast<T>(scalar);
			return *this;
		}
		template<typename A>
			requires std::convertible_to<A, T>
		constexpr vec<1, T, Q>& operator+=(vec<1, A, Q> const& v)
		{
			this->x += static_cast<T>(v.x);
			return *this;
		}
		template<typename A>
			requires std::convertible_to<A, T>
		constexpr vec<1, T, Q>& operator-=(A scalar)
		{
			this->x -= static_cast<T>(scalar);
			return *this;
		}
		template<typename A>
			requires std::convertible_to<A, T>
		constexpr vec<1, T, Q>& operator-=(vec<1, A, Q> const& v)
		{
			this->x -= static_cast<T>(v.x);
			return *this;
		}
		template<typename A>
			requires std::convertible_to<A, T>
		constexpr vec<1, T, Q>& operator*=(A scalar)
		{
			this->x *= static_cast<T>(scalar);
			return *this;
		}
		template<typename A>
			requires std::convertible_to<A, T>
		constexpr vec<1, T, Q>& operator*=(vec<1, A, Q> const& v)
		{
			this->x *= static_cast<T>(v.x);
			return *this;
		}
		template<typename A>
			requires std::convertible_to<A, T>
		constexpr vec<1, T, Q>& operator/=(A scalar)
		{
			this->x /= static_cast<T>(scalar);
			return *this;
		}
		template<typename A>
			requires std::convertible_to<A, T>
		constexpr vec<1, T, Q>& operator/=(vec<1, A, Q> const& v)
		{
			this->x /= static_cast<T>(v.x);
			return *this;
		}

		constexpr vec<1, T, Q>& operator++()
		{
			++this->x;
			return *this;
		}
		constexpr vec<1, T, Q>& operator--()
		{
			--this->x;
			return *this;
		}
		constexpr vec<1, T, Q> operator++(int)
		{
			vec<1, T, Q> result(*this);
			++*this;
			return result;
		}
		constexpr vec<1, T, Q> operator--(int)
		{
			vec<1, T, Q> result(*this);
			--*this;
			return result;
		}

		template<typename A>
			requires std::convertible_to<A, T>
		constexpr vec<1, T, Q>& operator%=(A scalar)
		{
			this->x %= static_cast<T>(scalar);
			return *this;
		}
		template<typename A>
			requires std::convertible_to<A, T>
		constexpr vec<1, T, Q>& operator%=(vec<1, A, Q> const& v)
		{
			this->x %= static_cast<T>(v.x);
			return *this;
		}
		template<typename A>
			requires std::convertible_to<A, T>
		constexpr vec<1, T, Q>& operator&=(A scalar)
		{
			this->x &= static_cast<T>(scalar);
			return *this;
		}
		template<typename A>
			requires std::convertible_to<A, T>
		constexpr vec<1, T, Q>& operator&=(vec<1, A, Q> const& v)
		{
			this->x &= static_cast<T>(v.x);
			return *this;
		}
		template<typename A>
			requires std::convertible_to<A, T>
		constexpr vec<1, T, Q>& operator|=(A scalar)
		{
			this->x |= static_cast<T>(scalar);
			return *this;
		}
		template<typename A>
			requires std::convertible_to<A, T>
		constexpr vec<1, T, Q>& operator|=(vec<1, A, Q> const& v)
		{
			this->x |= static_cast<T>(v.x);
			return *this;
		}
		template<typename A>
			requires std::convertible_to<A, T>
		constexpr vec<1, T, Q>& operator^=(A scalar)
		{
			this->x ^= static_cast<T>(scalar);
			return *this;
		}
		template<typename A>
			requires std::convertible_to<A, T>
		constexpr vec<1, T, Q>& operator^=(vec<1, A, Q> const& v)
		{
			this->x ^= static_cast<T>(v.x);
			return *this;
		}
		template<typename A>
			requires std::convertible_to<A, T>
		constexpr vec<1, T, Q>& operator<<=(A scalar)
		{
			this->x <<= static_cast<T>(scalar);
			return *this;
		}
		template<typename A>
			requires std::convertible_to<A, T>
		constexpr vec<1, T, Q>& operator<<=(vec<1, A, Q> const& v)
		{
			this->x <<= static_cast<T>(v.x);
			return *this;
		}
		template<typename A>
			requires std::convertible_to<A, T>
		constexpr vec<1, T, Q>& operator>>=(A scalar)
		{
			this->x >>= static_cast<T>(scalar);
			return *this;
		}
		template<typename A>
			requires std::convertible_to<A, T>
		constexpr vec<1, T, Q>& operator>>=(vec<1, A, Q> const& v)
		{
			this->x >>= static_cast<T>(v.x);
			return *this;
		}
	};

	template<typename T, Qualifier Q>
	constexpr vec<1, T, Q> operator+(vec<1, T, Q> const& v)
	{
		return v;
	}

	template<typename T, Qualifier Q>
	constexpr vec<1, T, Q> operator-(vec<1, T, Q> const& v)
	{
		return vec<1, T, Q>(-v.x);
	}

	template<typename T, Qualifier Q>
	constexpr vec<1, T, Q> operator+(vec<1, T, Q> const& v, T scalar)
	{
		return vec<1, T, Q>(v.x + scalar);
	}

	template<typename T, Qualifier Q>
	constexpr vec<1, T, Q> operator+(T scalar, vec<1, T, Q> const& v)
	{
		return vec<1, T, Q>(scalar + v.x);
	}

	template<typename T, Qualifier Q>
	constexpr vec<1, T, Q> operator+(vec<1, T, Q> const& v1,
		vec<1, T, Q> const& v2)
	{
		return vec<1, T, Q>(v1.x + v2.x);
	}

	template<typename T, Qualifier Q>
	constexpr vec<1, T, Q> operator-(vec<1, T, Q> const& v, T scalar)
	{
		return vec<1, T, Q>(v.x - scalar);
	}

	template<typename T, Qualifier Q>
	constexpr vec<1, T, Q> operator-(T scalar, vec<1, T, Q> const& v)
	{
		return vec<1, T, Q>(scalar - v.x);
	}

	template<typename T, Qualifier Q>
	constexpr vec<1, T, Q> operator-(vec<1, T, Q> const& v1,
		vec<1, T, Q> const& v2)
	{
		return vec<1, T, Q>(v1.x - v2.x);
	}

	template<typename T, Qualifier Q>
	constexpr vec<1, T, Q> operator*(vec<1, T, Q> const& v, T scalar)
	{
		return vec<1, T, Q>(v.x * scalar);
	}

	template<typename T, Qualifier Q>
	constexpr vec<1, T, Q> operator*(T scalar, vec<1, T, Q> const& v)
	{
		return vec<1, T, Q>(scalar * v.x);
	}

	template<typename T, Qualifier Q>
	constexpr vec<1, T, Q> operator*(vec<1, T, Q> const& v1,
		vec<1, T, Q> const& v2)
	{
		return vec<1, T, Q>(v1.x * v2.x);
	}

	template<typename T, Qualifier Q>
	constexpr vec<1, T, Q> operator/(vec<1, T, Q> const& v, T scalar)
	{
		return vec<1, T, Q>(v.x / scalar);
	}

	template<typename T, Qualifier Q>
	constexpr vec<1, T, Q> operator/(T scalar, vec<1, T, Q> const& v)
	{
		return vec<1, T, Q>(scalar / v.x);
	}

	template<typename T, Qualifier Q>
	constexpr vec<1, T, Q> operator/(vec<1, T, Q> const& v1,
		vec<1, T, Q> const& v2)
	{
		return vec<1, T, Q>(v1.x / v2.x);
	}

	template<typename T, Qualifier Q>
	constexpr vec<1, T, Q> operator%(vec<1, T, Q> const& v, T scalar)
	{
		return vec<1, T, Q>(v.x % scalar);
	}

	template<typename T, Qualifier Q>
	constexpr vec<1, T, Q> operator%(T scalar, vec<1, T, Q> const& v)
	{
		return vec<1, T, Q>(scalar % v.x);
	}

	template<typename T, Qualifier Q>
	constexpr vec<1, T, Q> operator%(vec<1, T, Q> const& v1,
		vec<1, T, Q> const& v2)
	{
		return vec<1, T, Q>(v1.x % v2.x);
	}

	template<typename T, Qualifier Q>
	constexpr vec<1, T, Q> operator&(vec<1, T, Q> const& v, T scalar)
	{
		return vec<1, T, Q>(v.x & scalar);
	}

	template<typename T, Qualifier Q>
	constexpr vec<1, T, Q> operator&(T scalar, vec<1, T, Q> const& v)
	{
		return vec<1, T, Q>(scalar & v.x);
	}

	template<typename T, Qualifier Q>
	constexpr vec<1, T, Q> operator&(vec<1, T, Q> const& v1,
		vec<1, T, Q> const& v2)
	{
		return vec<1, T, Q>(v1.x & v2.x);
	}

	template<typename T, Qualifier Q>
	constexpr vec<1, T, Q> operator|(vec<1, T, Q> const& v, T scalar)
	{
		return vec<1, T, Q>(v.x | scalar);
	}

	template<typename T, Qualifier Q>
	constexpr vec<1, T, Q> operator|(T scalar, vec<1, T, Q> const& v)
	{
		return vec<1, T, Q>(scalar | v.x);
	}

	template<typename T, Qualifier Q>
	constexpr vec<1, T, Q> operator|(vec<1, T, Q> const& v1,
		vec<1, T, Q> const& v2)
	{
		return vec<1, T, Q>(v1.x | v2.x);
	}

	template<typename T, Qualifier Q>
	constexpr vec<1, T, Q> operator^(vec<1, T, Q> const& v, T scalar)
	{
		return vec<1, T, Q>(v.x ^ scalar);
	}

	template<typename T, Qualifier Q>
	constexpr vec<1, T, Q> operator^(T scalar, vec<1, T, Q> const& v)
	{
		return vec<1, T, Q>(scalar ^ v.x);
	}

	template<typename T, Qualifier Q>
	constexpr vec<1, T, Q> operator^(vec<1, T, Q> const& v1,
		vec<1, T, Q> const& v2)
	{
		return vec<1, T, Q>(v1.x ^ v2.x);
	}

	template<typename T, Qualifier Q>
	constexpr vec<1, T, Q> operator<<(vec<1, T, Q> const& v, T scalar)
	{
		return vec<1, T, Q>(static_cast<T>(v.x << scalar));
	}

	template<typename T, Qualifier Q>
	constexpr vec<1, T, Q> operator<<(T scalar, vec<1, T, Q> const& v)
	{
		return vec<1, T, Q>(static_cast<T>(scalar << v.x));
	}

	template<typename T, Qualifier Q>
	constexpr vec<1, T, Q> operator<<(vec<1, T, Q> const& v1,
		vec<1, T, Q> const& v2)
	{
		return vec<1, T, Q>(static_cast<T>(v1.x << v2.x));
	}

	template<typename T, Qualifier Q>
	constexpr vec<1, T, Q> operator>>(vec<1, T, Q> const& v, T scalar)
	{
		return vec<1, T, Q>(static_cast<T>(v.x >> scalar));
	}

	template<typename T, Qualifier Q>
	constexpr vec<1, T, Q> operator>>(T scalar, vec<1, T, Q> const& v)
	{
		return vec<1, T, Q>(static_cast<T>(scalar >> v.x));
	}

	template<typename T, Qualifier Q>
	constexpr vec<1, T, Q> operator>>(vec<1, T, Q> const& v1,
		vec<1, T, Q> const& v2)
	{
		return vec<1, T, Q>(static_cast<T>(v1.x >> v2.x));
	}

	template<typename T, Qualifier Q>
	constexpr vec<1, T, Q> operator~(vec<1, T, Q> const& v)
	{
		return vec<1, T, Q>(~v.x);
	}

	template<typename T, Qualifier Q>
	constexpr bool operator==(vec<1, T, Q> const& v1, vec<1, T, Q> const& v2)
	{
		return v1.x == v2.x;
	}

	template<typename T, Qualifier Q>
	constexpr bool operator!=(vec<1, T, Q> const& v1, vec<1, T, Q> const& v2)
	{
		return !(v1 == v2);
	}

	template<Qualifier Q>
	constexpr vec<1, bool, Q> operator&&(vec<1, bool, Q> const& v1,
		vec<1, bool, Q> const& v2)
	{
		return vec<1, bool, Q>(v1.x && v2.x);
	}

	template<Qualifier Q>
	constexpr vec<1, bool, Q> operator||(vec<1, bool, Q> const& v1,
		vec<1, bool, Q> const& v2)
	{
		return vec<1, bool, Q>(v1.x || v2.x);
	}
}