#pragma once

namespace loquat
{
	template <typename T>
	struct Vec<1, T>
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

		constexpr Vec()
			: x{ 0 }
		{}
		constexpr Vec(Vec<1, T> const& v)
			: x{ v.x }
		{}

		constexpr explicit Vec(T scalar)
			: x{ scalar }
		{}

		template <typename A>
			requires std::convertible_to<A, T>
		constexpr Vec(A x)
			: x{ static_cast<T>(x) }
		{}

		template <typename A>
			requires std::convertible_to<A, T>
		constexpr Vec(Vec<2, A> const& v)
			: x{ static_cast<T>(v.x) }
		{}
		template <typename A>
			requires std::convertible_to<A, T>
		constexpr Vec(Vec<3, A> const& v)
			: x{ static_cast<T>(v.x) }
		{}

		constexpr Vec<1, T>& operator=(Vec const& v)
		{
			this->x = v.x;
			return *this;
		}

		template<typename A>
			requires std::convertible_to<A, T>
		constexpr Vec<1, T>& operator=(Vec<1, A> const& v)
		{
			this->x = static_cast<T>(v.x);
			return *this;
		}
		template<typename A>
			requires std::convertible_to<A, T>
		constexpr Vec<1, T>& operator+=(A scalar)
		{
			this->x += static_cast<T>(scalar);
			return *this;
		}
		template<typename A>
			requires std::convertible_to<A, T>
		constexpr Vec<1, T>& operator+=(Vec<1, A> const& v)
		{
			this->x += static_cast<T>(v.x);
			return *this;
		}
		template<typename A>
			requires std::convertible_to<A, T>
		constexpr Vec<1, T>& operator-=(A scalar)
		{
			this->x -= static_cast<T>(scalar);
			return *this;
		}
		template<typename A>
			requires std::convertible_to<A, T>
		constexpr Vec<1, T>& operator-=(Vec<1, A> const& v)
		{
			this->x -= static_cast<T>(v.x);
			return *this;
		}
		template<typename A>
			requires std::convertible_to<A, T>
		constexpr Vec<1, T>& operator*=(A scalar)
		{
			this->x *= static_cast<T>(scalar);
			return *this;
		}
		template<typename A>
			requires std::convertible_to<A, T>
		constexpr Vec<1, T>& operator*=(Vec<1, A> const& v)
		{
			this->x *= static_cast<T>(v.x);
			return *this;
		}
		template<typename A>
			requires std::convertible_to<A, T>
		constexpr Vec<1, T>& operator/=(A scalar)
		{
			this->x /= static_cast<T>(scalar);
			return *this;
		}
		template<typename A>
			requires std::convertible_to<A, T>
		constexpr Vec<1, T>& operator/=(Vec<1, A> const& v)
		{
			this->x /= static_cast<T>(v.x);
			return *this;
		}

		constexpr Vec<1, T>& operator++()
		{
			++this->x;
			return *this;
		}
		constexpr Vec<1, T>& operator--()
		{
			--this->x;
			return *this;
		}
		constexpr Vec<1, T> operator++(int)
		{
			Vec<1, T> result(*this);
			++*this;
			return result;
		}
		constexpr Vec<1, T> operator--(int)
		{
			Vec<1, T> result(*this);
			--*this;
			return result;
		}

		template<typename A>
			requires std::convertible_to<A, T>
		constexpr Vec<1, T>& operator%=(A scalar)
		{
			this->x %= static_cast<T>(scalar);
			return *this;
		}
		template<typename A>
			requires std::convertible_to<A, T>
		constexpr Vec<1, T>& operator%=(Vec<1, A> const& v)
		{
			this->x %= static_cast<T>(v.x);
			return *this;
		}
		template<typename A>
			requires std::convertible_to<A, T>
		constexpr Vec<1, T>& operator&=(A scalar)
		{
			this->x &= static_cast<T>(scalar);
			return *this;
		}
		template<typename A>
			requires std::convertible_to<A, T>
		constexpr Vec<1, T>& operator&=(Vec<1, A> const& v)
		{
			this->x &= static_cast<T>(v.x);
			return *this;
		}
		template<typename A>
			requires std::convertible_to<A, T>
		constexpr Vec<1, T>& operator|=(A scalar)
		{
			this->x |= static_cast<T>(scalar);
			return *this;
		}
		template<typename A>
			requires std::convertible_to<A, T>
		constexpr Vec<1, T>& operator|=(Vec<1, A> const& v)
		{
			this->x |= static_cast<T>(v.x);
			return *this;
		}
		template<typename A>
			requires std::convertible_to<A, T>
		constexpr Vec<1, T>& operator^=(A scalar)
		{
			this->x ^= static_cast<T>(scalar);
			return *this;
		}
		template<typename A>
			requires std::convertible_to<A, T>
		constexpr Vec<1, T>& operator^=(Vec<1, A> const& v)
		{
			this->x ^= static_cast<T>(v.x);
			return *this;
		}
		template<typename A>
			requires std::convertible_to<A, T>
		constexpr Vec<1, T>& operator<<=(A scalar)
		{
			this->x <<= static_cast<T>(scalar);
			return *this;
		}
		template<typename A>
			requires std::convertible_to<A, T>
		constexpr Vec<1, T>& operator<<=(Vec<1, A> const& v)
		{
			this->x <<= static_cast<T>(v.x);
			return *this;
		}
		template<typename A>
			requires std::convertible_to<A, T>
		constexpr Vec<1, T>& operator>>=(A scalar)
		{
			this->x >>= static_cast<T>(scalar);
			return *this;
		}
		template<typename A>
			requires std::convertible_to<A, T>
		constexpr Vec<1, T>& operator>>=(Vec<1, A> const& v)
		{
			this->x >>= static_cast<T>(v.x);
			return *this;
		}
	};

	template<typename T>
	constexpr Vec<1, T> operator+(Vec<1, T> const& v)
	{
		return v;
	}

	template<typename T>
	constexpr Vec<1, T> operator-(Vec<1, T> const& v)
	{
		return Vec<1, T>(-v.x);
	}

	template<typename T>
	constexpr Vec<1, T> operator+(Vec<1, T> const& v, T scalar)
	{
		return Vec<1, T>(v.x + scalar);
	}

	template<typename T>
	constexpr Vec<1, T> operator+(T scalar, Vec<1, T> const& v)
	{
		return Vec<1, T>(scalar + v.x);
	}

	template<typename T>
	constexpr Vec<1, T> operator+(Vec<1, T> const& v1,
		Vec<1, T> const& v2)
	{
		return Vec<1, T>(v1.x + v2.x);
	}

	template<typename T>
	constexpr Vec<1, T> operator-(Vec<1, T> const& v, T scalar)
	{
		return Vec<1, T>(v.x - scalar);
	}

	template<typename T>
	constexpr Vec<1, T> operator-(T scalar, Vec<1, T> const& v)
	{
		return Vec<1, T>(scalar - v.x);
	}

	template<typename T>
	constexpr Vec<1, T> operator-(Vec<1, T> const& v1,
		Vec<1, T> const& v2)
	{
		return Vec<1, T>(v1.x - v2.x);
	}

	template<typename T>
	constexpr Vec<1, T> operator*(Vec<1, T> const& v, T scalar)
	{
		return Vec<1, T>(v.x * scalar);
	}

	template<typename T>
	constexpr Vec<1, T> operator*(T scalar, Vec<1, T> const& v)
	{
		return Vec<1, T>(scalar * v.x);
	}

	template<typename T>
	constexpr Vec<1, T> operator*(Vec<1, T> const& v1,
		Vec<1, T> const& v2)
	{
		return Vec<1, T>(v1.x * v2.x);
	}

	template<typename T>
	constexpr Vec<1, T> operator/(Vec<1, T> const& v, T scalar)
	{
		return Vec<1, T>(v.x / scalar);
	}

	template<typename T>
	constexpr Vec<1, T> operator/(T scalar, Vec<1, T> const& v)
	{
		return Vec<1, T>(scalar / v.x);
	}

	template<typename T>
	constexpr Vec<1, T> operator/(Vec<1, T> const& v1,
		Vec<1, T> const& v2)
	{
		return Vec<1, T>(v1.x / v2.x);
	}

	template<typename T>
	constexpr Vec<1, T> operator%(Vec<1, T> const& v, T scalar)
	{
		return Vec<1, T>(v.x % scalar);
	}

	template<typename T>
	constexpr Vec<1, T> operator%(T scalar, Vec<1, T> const& v)
	{
		return Vec<1, T>(scalar % v.x);
	}

	template<typename T>
	constexpr Vec<1, T> operator%(Vec<1, T> const& v1,
		Vec<1, T> const& v2)
	{
		return Vec<1, T>(v1.x % v2.x);
	}

	template<typename T>
	constexpr Vec<1, T> operator&(Vec<1, T> const& v, T scalar)
	{
		return Vec<1, T>(v.x & scalar);
	}

	template<typename T>
	constexpr Vec<1, T> operator&(T scalar, Vec<1, T> const& v)
	{
		return Vec<1, T>(scalar & v.x);
	}

	template<typename T>
	constexpr Vec<1, T> operator&(Vec<1, T> const& v1,
		Vec<1, T> const& v2)
	{
		return Vec<1, T>(v1.x & v2.x);
	}

	template<typename T>
	constexpr Vec<1, T> operator|(Vec<1, T> const& v, T scalar)
	{
		return Vec<1, T>(v.x | scalar);
	}

	template<typename T>
	constexpr Vec<1, T> operator|(T scalar, Vec<1, T> const& v)
	{
		return Vec<1, T>(scalar | v.x);
	}

	template<typename T>
	constexpr Vec<1, T> operator|(Vec<1, T> const& v1,
		Vec<1, T> const& v2)
	{
		return Vec<1, T>(v1.x | v2.x);
	}

	template<typename T>
	constexpr Vec<1, T> operator^(Vec<1, T> const& v, T scalar)
	{
		return Vec<1, T>(v.x ^ scalar);
	}

	template<typename T>
	constexpr Vec<1, T> operator^(T scalar, Vec<1, T> const& v)
	{
		return Vec<1, T>(scalar ^ v.x);
	}

	template<typename T>
	constexpr Vec<1, T> operator^(Vec<1, T> const& v1,
		Vec<1, T> const& v2)
	{
		return Vec<1, T>(v1.x ^ v2.x);
	}

	template<typename T>
	constexpr Vec<1, T> operator<<(Vec<1, T> const& v, T scalar)
	{
		return Vec<1, T>(static_cast<T>(v.x << scalar));
	}

	template<typename T>
	constexpr Vec<1, T> operator<<(T scalar, Vec<1, T> const& v)
	{
		return Vec<1, T>(static_cast<T>(scalar << v.x));
	}

	template<typename T>
	constexpr Vec<1, T> operator<<(Vec<1, T> const& v1,
		Vec<1, T> const& v2)
	{
		return Vec<1, T>(static_cast<T>(v1.x << v2.x));
	}

	template<typename T>
	constexpr Vec<1, T> operator>>(Vec<1, T> const& v, T scalar)
	{
		return Vec<1, T>(static_cast<T>(v.x >> scalar));
	}

	template<typename T>
	constexpr Vec<1, T> operator>>(T scalar, Vec<1, T> const& v)
	{
		return Vec<1, T>(static_cast<T>(scalar >> v.x));
	}

	template<typename T>
	constexpr Vec<1, T> operator>>(Vec<1, T> const& v1,
		Vec<1, T> const& v2)
	{
		return Vec<1, T>(static_cast<T>(v1.x >> v2.x));
	}

	template<typename T>
	constexpr Vec<1, T> operator~(Vec<1, T> const& v)
	{
		return Vec<1, T>(~v.x);
	}

	template<typename T>
	constexpr bool operator==(Vec<1, T> const& v1, Vec<1, T> const& v2)
	{
		return v1.x == v2.x;
	}

	template<typename T>
	constexpr bool operator!=(Vec<1, T> const& v1, Vec<1, T> const& v2)
	{
		return !(v1 == v2);
	}

	constexpr Vec<1, bool> operator&&(Vec<1, bool> const& v1,
		Vec<1, bool> const& v2)
	{
		return Vec<1, bool>(v1.x && v2.x);
	}

	constexpr Vec<1, bool> operator||(Vec<1, bool> const& v1,
		Vec<1, bool> const& v2)
	{
		return Vec<1, bool>(v1.x || v2.x);
	}
}