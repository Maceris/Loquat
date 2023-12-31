#pragma once

namespace loquat
{
	template <typename T>
	struct Vec<2, T>
	{
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

		constexpr Vec()
			: x{ 0 }
			, y{ 0 }
		{}
		constexpr Vec(Vec<2, T> const& v)
			: x{ v.x }
			, y{ v.y }
		{}

		constexpr explicit Vec(T scalar)
			: x{ scalar }
			, y{ scalar }
		{}
		constexpr Vec(T x, T y)
			: x{ x }
			, y{ y }
		{}

		template <typename A, typename B>
			requires std::convertible_to<A, T>&& std::convertible_to<B, T>
		constexpr Vec(A x, B y)
			: x{ static_cast<T>(x) }
			, y{ static_cast<T>(y) }
		{}
		template <typename A, typename B>
			requires std::convertible_to<A, T>&& std::convertible_to<B, T>
		constexpr Vec(Vec<1, A> const& v, B y)
			: x{ static_cast<T>(v.x) }
			, y{ static_cast<T>(y) }
		{}
		template <typename A, typename B>
			requires std::convertible_to<A, T>&& std::convertible_to<B, T>
		constexpr Vec(A x, Vec<1, B> const& v)
			: x{ static_cast<T>(x) }
			, y{ static_cast<T>(v.x) }
		{}
		template <typename A, typename B>
			requires std::convertible_to<A, T>&& std::convertible_to<B, T>
		constexpr Vec(Vec<1, A> const& v1, Vec<1, B> const& v2)
			: x{ static_cast<T>(v1.x) }
			, y{ static_cast<T>(v2.x) }
		{}

		template <typename A>
			requires std::convertible_to<A, T>
		constexpr explicit Vec(Vec<2, A> const& v)
			: x{ static_cast<T>(v.x) }
			, y{ static_cast<T>(v.y) }
		{}
		template <typename A>
			requires std::convertible_to<A, T>
		constexpr explicit Vec(Vec<3, A> const& v)
			: x{ static_cast<T>(v.x) }
			, y{ static_cast<T>(v.y) }
		{}

		constexpr Vec<2, T>& operator=(Vec const& v)
		{
			this->x = v.x;
			this->y = v.y;
			return *this;
		}

		template<typename A>
			requires std::convertible_to<A, T>
		constexpr Vec<2, T>& operator=(Vec<2, A> const& v)
		{
			this->x = static_cast<T>(v.x);
			this->y = static_cast<T>(v.y);
			return *this;
		}
		template<typename A>
			requires std::convertible_to<A, T>
		constexpr Vec<2, T>& operator+=(A scalar)
		{
			this->x += static_cast<T>(scalar);
			this->y += static_cast<T>(scalar);
			return *this;
		}
		template<typename A>
			requires std::convertible_to<A, T>
		constexpr Vec<2, T>& operator+=(Vec<1, A> const& v)
		{
			this->x += static_cast<T>(v.x);
			this->y += static_cast<T>(v.x);
			return *this;
		}
		template<typename A>
			requires std::convertible_to<A, T>
		constexpr Vec<2, T>& operator+=(Vec<2, A> const& v)
		{
			this->x += static_cast<T>(v.x);
			this->y += static_cast<T>(v.y);
			return *this;
		}
		template<typename A>
			requires std::convertible_to<A, T>
		constexpr Vec<2, T>& operator-=(A scalar)
		{
			this->x -= static_cast<T>(scalar);
			this->y -= static_cast<T>(scalar);
			return *this;
		}
		template<typename A>
			requires std::convertible_to<A, T>
		constexpr Vec<2, T>& operator-=(Vec<1, A> const& v)
		{
			this->x -= static_cast<T>(v.x);
			this->y -= static_cast<T>(v.x);
			return *this;
		}
		template<typename A>
			requires std::convertible_to<A, T>
		constexpr Vec<2, T>& operator-=(Vec<2, A> const& v)
		{
			this->x -= static_cast<T>(v.x);
			this->y -= static_cast<T>(v.y);
			return *this;
		}
		template<typename A>
			requires std::convertible_to<A, T>
		constexpr Vec<2, T>& operator*=(A scalar)
		{
			this->x *= static_cast<T>(scalar);
			this->y *= static_cast<T>(scalar);
			return *this;
		}
		template<typename A>
			requires std::convertible_to<A, T>
		constexpr Vec<2, T>& operator*=(Vec<1, A> const& v)
		{
			this->x *= static_cast<T>(v.x);
			this->y *= static_cast<T>(v.x);
			return *this;
		}
		template<typename A>
			requires std::convertible_to<A, T>
		constexpr Vec<2, T>& operator*=(Vec<2, A> const& v)
		{
			this->x *= static_cast<T>(v.x);
			this->y *= static_cast<T>(v.y);
			return *this;
		}
		template<typename A>
			requires std::convertible_to<A, T>
		constexpr Vec<2, T>& operator/=(A scalar)
		{
			this->x /= static_cast<T>(scalar);
			this->y /= static_cast<T>(scalar);
			return *this;
		}
		template<typename A>
			requires std::convertible_to<A, T>
		constexpr Vec<2, T>& operator/=(Vec<1, A> const& v)
		{
			this->x /= static_cast<T>(v.x);
			this->y /= static_cast<T>(v.x);
			return *this;
		}
		template<typename A>
			requires std::convertible_to<A, T>
		constexpr Vec<2, T>& operator/=(Vec<2, A> const& v)
		{
			this->x /= static_cast<T>(v.x);
			this->y /= static_cast<T>(v.y);
			return *this;
		}

		constexpr Vec<2, T>& operator++()
		{
			++this->x;
			++this->y;
			return *this;
		}
		constexpr Vec<2, T>& operator--()
		{
			--this->x;
			--this->y;
			return *this;
		}
		constexpr Vec<2, T> operator++(int)
		{
			Vec<2, T> result(*this);
			++*this;
			return result;
		}
		constexpr Vec<2, T> operator--(int)
		{
			Vec<2, T> result(*this);
			--*this;
			return result;
		}

		template<typename A>
			requires std::convertible_to<A, T>
		constexpr Vec<2, T>& operator%=(A scalar)
		{
			this->x %= static_cast<T>(scalar);
			this->y %= static_cast<T>(scalar);
			return *this;
		}
		template<typename A>
			requires std::convertible_to<A, T>
		constexpr Vec<2, T>& operator%=(Vec<1, A> const& v)
		{
			this->x %= static_cast<T>(v.x);
			this->y %= static_cast<T>(v.x);
			return *this;
		}
		template<typename A>
			requires std::convertible_to<A, T>
		constexpr Vec<2, T>& operator%=(Vec<2, A> const& v)
		{
			this->x %= static_cast<T>(v.x);
			this->y %= static_cast<T>(v.y);
			return *this;
		}
		template<typename A>
			requires std::convertible_to<A, T>
		constexpr Vec<2, T>& operator&=(A scalar)
		{
			this->x &= static_cast<T>(scalar);
			this->y &= static_cast<T>(scalar);
			return *this;
		}
		template<typename A>
			requires std::convertible_to<A, T>
		constexpr Vec<2, T>& operator&=(Vec<1, A> const& v)
		{
			this->x &= static_cast<T>(v.x);
			this->y &= static_cast<T>(v.x);
			return *this;
		}
		template<typename A>
			requires std::convertible_to<A, T>
		constexpr Vec<2, T>& operator&=(Vec<2, A> const& v)
		{
			this->x &= static_cast<T>(v.x);
			this->y &= static_cast<T>(v.y);
			return *this;
		}
		template<typename A>
			requires std::convertible_to<A, T>
		constexpr Vec<2, T>& operator|=(A scalar)
		{
			this->x |= static_cast<T>(scalar);
			this->y |= static_cast<T>(scalar);
			return *this;
		}
		template<typename A>
			requires std::convertible_to<A, T>
		constexpr Vec<2, T>& operator|=(Vec<1, A> const& v)
		{
			this->x |= static_cast<T>(v.x);
			this->y |= static_cast<T>(v.x);
			return *this;
		}
		template<typename A>
			requires std::convertible_to<A, T>
		constexpr Vec<2, T>& operator|=(Vec<2, A> const& v)
		{
			this->x |= static_cast<T>(v.x);
			this->y |= static_cast<T>(v.y);
			return *this;
		}
		template<typename A>
			requires std::convertible_to<A, T>
		constexpr Vec<2, T>& operator^=(A scalar)
		{
			this->x ^= static_cast<T>(scalar);
			this->y ^= static_cast<T>(scalar);
			return *this;
		}
		template<typename A>
			requires std::convertible_to<A, T>
		constexpr Vec<2, T>& operator^=(Vec<1, A> const& v)
		{
			this->x ^= static_cast<T>(v.x);
			this->y ^= static_cast<T>(v.x);
			return *this;
		}
		template<typename A>
			requires std::convertible_to<A, T>
		constexpr Vec<2, T>& operator^=(Vec<2, A> const& v)
		{
			this->x ^= static_cast<T>(v.x);
			this->y ^= static_cast<T>(v.y);
			return *this;
		}
		template<typename A>
			requires std::convertible_to<A, T>
		constexpr Vec<2, T>& operator<<=(A scalar)
		{
			this->x <<= static_cast<T>(scalar);
			this->y <<= static_cast<T>(scalar);
			return *this;
		}
		template<typename A>
			requires std::convertible_to<A, T>
		constexpr Vec<2, T>& operator<<=(Vec<1, A> const& v)
		{
			this->x <<= static_cast<T>(v.x);
			this->y <<= static_cast<T>(v.x);
			return *this;
		}
		template<typename A>
			requires std::convertible_to<A, T>
		constexpr Vec<2, T>& operator<<=(Vec<2, A> const& v)
		{
			this->x <<= static_cast<T>(v.x);
			this->y <<= static_cast<T>(v.y);
			return *this;
		}
		template<typename A>
			requires std::convertible_to<A, T>
		constexpr Vec<2, T>& operator>>=(A scalar)
		{
			this->x >>= static_cast<T>(scalar);
			this->y >>= static_cast<T>(scalar);
			return *this;
		}
		template<typename A>
			requires std::convertible_to<A, T>
		constexpr Vec<2, T>& operator>>=(Vec<1, A> const& v)
		{
			this->x >>= static_cast<T>(v.x);
			this->y >>= static_cast<T>(v.x);
			return *this;
		}
		template<typename A>
			requires std::convertible_to<A, T>
		constexpr Vec<2, T>& operator>>=(Vec<2, A> const& v)
		{
			this->x >>= static_cast<T>(v.x);
			this->y >>= static_cast<T>(v.y);
			return *this;
		}
	};

	template<typename T>
	constexpr Vec<2, T> operator+(Vec<2, T> const& v)
	{
		return v;
	}

	template<typename T>
	constexpr Vec<2, T> operator-(Vec<2, T> const& v)
	{
		return Vec<2, T>(-v.x, -v.y);
	}

	template<typename T>
	constexpr Vec<2, T> operator+(Vec<2, T> const& v, T scalar)
	{
		return Vec<2, T>(v.x + scalar, v.y + scalar);
	}

	template<typename T>
	constexpr Vec<2, T> operator+(Vec<2, T> const& v1,
		Vec<1, T> const& v2)
	{
		return Vec<2, T>(v1.x + v2.x, v1.y + v2.x);
	}

	template<typename T>
	constexpr Vec<2, T> operator+(T scalar, Vec<2, T> const& v)
	{
		return Vec<2, T>(scalar + v.x, scalar + v.y);
	}

	template<typename T>
	constexpr Vec<2, T> operator+(Vec<1, T> const& v1,
		Vec<2, T> const& v2)
	{
		return Vec<2, T>(v1.x + v2.x, v1.x + v2.y);
	}

	template<typename T>
	constexpr Vec<2, T> operator+(Vec<2, T> const& v1,
		Vec<2, T> const& v2)
	{
		return Vec<2, T>(v1.x + v2.x, v1.y + v2.y);
	}

	template<typename T>
	constexpr Vec<2, T> operator-(Vec<2, T> const& v, T scalar)
	{
		return Vec<2, T>(v.x - scalar, v.y - scalar);
	}

	template<typename T>
	constexpr Vec<2, T> operator-(Vec<2, T> const& v1,
		Vec<1, T> const& v2)
	{
		return Vec<2, T>(v1.x - v2.x, v1.y - v2.x);
	}

	template<typename T>
	constexpr Vec<2, T> operator-(T scalar, Vec<2, T> const& v)
	{
		return Vec<2, T>(scalar - v.x, scalar - v.y);
	}

	template<typename T>
	constexpr Vec<2, T> operator-(Vec<1, T> const& v1,
		Vec<2, T> const& v2)
	{
		return Vec<2, T>(v1.x - v2.x, v1.x - v2.y);
	}

	template<typename T>
	constexpr Vec<2, T> operator-(Vec<2, T> const& v1,
		Vec<2, T> const& v2)
	{
		return Vec<2, T>(v1.x - v2.x, v1.y - v2.y);
	}

	template<typename T>
	constexpr Vec<2, T> operator*(Vec<2, T> const& v, T scalar)
	{
		return Vec<2, T>(v.x * scalar, v.y * scalar);
	}

	template<typename T>
	constexpr Vec<2, T> operator*(Vec<2, T> const& v1,
		Vec<1, T> const& v2)
	{
		return Vec<2, T>(v1.x * v2.x, v1.y * v2.x);
	}

	template<typename T>
	constexpr Vec<2, T> operator*(T scalar, Vec<2, T> const& v)
	{
		return Vec<2, T>(scalar * v.x, scalar * v.y);
	}

	template<typename T>
	constexpr Vec<2, T> operator*(Vec<1, T> const& v1,
		Vec<2, T> const& v2)
	{
		return Vec<2, T>(v1.x * v2.x, v1.x * v2.y);
	}

	template<typename T>
	constexpr Vec<2, T> operator*(Vec<2, T> const& v1,
		Vec<2, T> const& v2)
	{
		return Vec<2, T>(v1.x * v2.x, v1.y * v2.y);
	}

	template<typename T>
	constexpr Vec<2, T> operator/(Vec<2, T> const& v, T scalar)
	{
		return Vec<2, T>(v.x / scalar, v.y / scalar);
	}

	template<typename T>
	constexpr Vec<2, T> operator/(Vec<2, T> const& v1,
		Vec<1, T> const& v2)
	{
		return Vec<2, T>(v1.x / v2.x, v1.y / v2.x);
	}

	template<typename T>
	constexpr Vec<2, T> operator/(T scalar, Vec<2, T> const& v)
	{
		return Vec<2, T>(scalar / v.x, scalar / v.y);
	}

	template<typename T>
	constexpr Vec<2, T> operator/(Vec<1, T> const& v1,
		Vec<2, T> const& v2)
	{
		return Vec<2, T>(v1.x / v2.x, v1.x / v2.y);
	}

	template<typename T>
	constexpr Vec<2, T> operator/(Vec<2, T> const& v1,
		Vec<2, T> const& v2)
	{
		return Vec<2, T>(v1.x / v2.x, v1.y / v2.y);
	}

	template<typename T>
	constexpr Vec<2, T> operator%(Vec<2, T> const& v, T scalar)
	{
		return Vec<2, T>(v.x % scalar, v.y % scalar);
	}

	template<typename T>
	constexpr Vec<2, T> operator%(Vec<2, T> const& v1,
		Vec<1, T> const& v2)
	{
		return Vec<2, T>(v1.x % v2.x, v1.y % v2.x);
	}

	template<typename T>
	constexpr Vec<2, T> operator%(T scalar, Vec<2, T> const& v)
	{
		return Vec<2, T>(scalar % v.x, scalar % v.y);
	}

	template<typename T>
	constexpr Vec<2, T> operator%(Vec<1, T> const& v1,
		Vec<2, T> const& v2)
	{
		return Vec<2, T>(v1.x % v2.x, v1.x % v2.y);
	}

	template<typename T>
	constexpr Vec<2, T> operator%(Vec<2, T> const& v1,
		Vec<2, T> const& v2)
	{
		return Vec<2, T>(v1.x % v2.x, v1.y % v2.y);
	}

	template<typename T>
	constexpr Vec<2, T> operator&(Vec<2, T> const& v, T scalar)
	{
		return Vec<2, T>(v.x & scalar, v.y & scalar);
	}

	template<typename T>
	constexpr Vec<2, T> operator&(Vec<2, T> const& v1,
		Vec<1, T> const& v2)
	{
		return Vec<2, T>(v1.x & v2.x, v1.y & v2.x);
	}

	template<typename T>
	constexpr Vec<2, T> operator&(T scalar, Vec<2, T> const& v)
	{
		return Vec<2, T>(scalar & v.x, scalar & v.y);
	}

	template<typename T>
	constexpr Vec<2, T> operator&(Vec<1, T> const& v1,
		Vec<2, T> const& v2)
	{
		return Vec<2, T>(v1.x & v2.x, v1.x & v2.y);
	}

	template<typename T>
	constexpr Vec<2, T> operator&(Vec<2, T> const& v1,
		Vec<2, T> const& v2)
	{
		return Vec<2, T>(v1.x & v2.x, v1.y & v2.y);
	}

	template<typename T>
	constexpr Vec<2, T> operator|(Vec<2, T> const& v, T scalar)
	{
		return Vec<2, T>(v.x | scalar, v.y | scalar);
	}

	template<typename T>
	constexpr Vec<2, T> operator|(Vec<2, T> const& v1,
		Vec<1, T> const& v2)
	{
		return Vec<2, T>(v1.x | v2.x, v1.y | v2.x);
	}

	template<typename T>
	constexpr Vec<2, T> operator|(T scalar, Vec<2, T> const& v)
	{
		return Vec<2, T>(scalar | v.x, scalar | v.y);
	}

	template<typename T>
	constexpr Vec<2, T> operator|(Vec<1, T> const& v1,
		Vec<2, T> const& v2)
	{
		return Vec<2, T>(v1.x | v2.x, v1.x | v2.y);
	}

	template<typename T>
	constexpr Vec<2, T> operator|(Vec<2, T> const& v1,
		Vec<2, T> const& v2)
	{
		return Vec<2, T>(v1.x | v2.x, v1.y | v2.y);
	}

	template<typename T>
	constexpr Vec<2, T> operator^(Vec<2, T> const& v, T scalar)
	{
		return Vec<2, T>(v.x ^ scalar, v.y ^ scalar);
	}

	template<typename T>
	constexpr Vec<2, T> operator^(Vec<2, T> const& v1,
		Vec<1, T> const& v2)
	{
		return Vec<2, T>(v1.x ^ v2.x, v1.y ^ v2.x);
	}

	template<typename T>
	constexpr Vec<2, T> operator^(T scalar, Vec<2, T> const& v)
	{
		return Vec<2, T>(scalar ^ v.x, scalar ^ v.y);
	}

	template<typename T>
	constexpr Vec<2, T> operator^(Vec<1, T> const& v1,
		Vec<2, T> const& v2)
	{
		return Vec<2, T>(v1.x ^ v2.x, v1.x ^ v2.y);
	}

	template<typename T>
	constexpr Vec<2, T> operator^(Vec<2, T> const& v1,
		Vec<2, T> const& v2)
	{
		return Vec<2, T>(v1.x ^ v2.x, v1.y ^ v2.y);
	}

	template<typename T>
	constexpr Vec<2, T> operator<<(Vec<2, T> const& v, T scalar)
	{
		return Vec<2, T>(static_cast<T>(v.x << scalar), 
			static_cast<T>(v.y << scalar));
	}

	template<typename T>
	constexpr Vec<2, T> operator<<(Vec<2, T> const& v1,
		Vec<1, T> const& v2)
	{
		return Vec<2, T>(static_cast<T>(v1.x << v2.x),
			static_cast<T>(v1.y << v2.x));
	}

	template<typename T>
	constexpr Vec<2, T> operator<<(T scalar, Vec<2, T> const& v)
	{
		return Vec<2, T>(static_cast<T>(scalar << v.x),
			static_cast<T>(scalar << v.y));
	}

	template<typename T>
	constexpr Vec<2, T> operator<<(Vec<1, T> const& v1,
		Vec<2, T> const& v2)
	{
		return Vec<2, T>(static_cast<T>(v1.x << v2.x),
			static_cast<T>(v1.x << v2.y));
	}

	template<typename T>
	constexpr Vec<2, T> operator<<(Vec<2, T> const& v1,
		Vec<2, T> const& v2)
	{
		return Vec<2, T>(static_cast<T>(v1.x << v2.x),
			static_cast<T>(v1.y << v2.y));
	}

	template<typename T>
	constexpr Vec<2, T> operator>>(Vec<2, T> const& v, T scalar)
	{
		return Vec<2, T>(static_cast<T>(v.x >> scalar),
			static_cast<T>(v.y >> scalar));
	}

	template<typename T>
	constexpr Vec<2, T> operator>>(Vec<2, T> const& v1, Vec<1, T> const& v2)
	{
		return Vec<2, T>(static_cast<T>(v1.x >> v2.x),
			static_cast<T>(v1.y >> v2.x));
	}

	template<typename T>
	constexpr Vec<2, T> operator>>(T scalar, Vec<2, T> const& v)
	{
		return Vec<2, T>(static_cast<T>(scalar >> v.x),
			static_cast<T>(scalar >> v.y));
	}

	template<typename T>
	constexpr Vec<2, T> operator>>(Vec<1, T> const& v1, Vec<2, T> const& v2)
	{
		return Vec<2, T>(static_cast<T>(v1.x >> v2.x),
			static_cast<T>(v1.x >> v2.y));
	}

	template<typename T>
	constexpr Vec<2, T> operator>>(Vec<2, T> const& v1, Vec<2, T> const& v2)
	{
		return Vec<2, T>(static_cast<T>(v1.x >> v2.x),
			static_cast<T>(v1.y >> v2.y));
	}

	template<typename T>
	constexpr Vec<2, T> operator~(Vec<2, T> const& v)
	{
		return Vec<2, T>(~v.x, ~v.y);
	}

	template<typename T>
	constexpr bool operator==(Vec<2, T> const& v1, Vec<2, T> const& v2)
	{
		return v1.x == v2.x && v1.y == v2.y;
	}

	template<typename T>
	constexpr bool operator!=(Vec<2, T> const& v1, Vec<2, T> const& v2)
	{
		return !(v1 == v2);
	}

	constexpr Vec<2, bool> operator&&(Vec<2, bool> const& v1,
		Vec<2, bool> const& v2)
	{
		return Vec<2, bool>(v1.x && v2.x, v1.y && v2.y);
	}

	constexpr Vec<2, bool> operator||(Vec<2, bool> const& v1,
		Vec<2, bool> const& v2)
	{
		return Vec<2, bool>(v1.x || v2.x, v1.y || v2.y);
	}
}