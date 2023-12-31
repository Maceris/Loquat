#pragma once

namespace loquat
{
	template <typename T>
	struct Vec<3, T>
	{
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

		constexpr Vec()
			: x{ 0 }
			, y{ 0 }
			, z{ 0 }
		{}
		constexpr Vec(Vec<3, T> const& v)
			: x{ v.x }
			, y{ v.y }
			, z{ v.z }
		{}

		constexpr explicit Vec(T scalar)
			: x{ scalar }
			, y{ scalar }
			, z{ scalar }
		{}
		constexpr Vec(T x, T y, T z)
			: x{ x }
			, y{ y }
			, z{ z }
		{}

		template<typename A>
			requires std::convertible_to<A, T>
		constexpr explicit Vec(Vec<1, A> const& v)
			: x{ v.x }
			, y{ v.x }
			, z{ v.x }
		{}

		template <typename A, typename B, typename C>
			requires std::convertible_to<A, T>&& std::convertible_to<B, T>&& std::convertible_to<C, T>
		constexpr Vec(A x, B y, C z)
			: x{ static_cast<T>(x) }
			, y{ static_cast<T>(y) }
			, z{ static_cast<T>(z) }
		{}

		template <typename A, typename B, typename C>
			requires std::convertible_to<A, T>&& std::convertible_to<B, T>&& std::convertible_to<C, T>
		constexpr Vec(Vec<1, A> const& v, B y, C z)
			: x{ static_cast<T>(v.x) }
			, y{ static_cast<T>(y) }
			, z{ static_cast<T>(z) }
		{}

		template <typename A, typename B, typename C>
			requires std::convertible_to<A, T>&& std::convertible_to<B, T>&& std::convertible_to<C, T>
		constexpr Vec(A x, Vec<1, B> const& v, C z)
			: x{ static_cast<T>(x) }
			, y{ static_cast<T>(v.x) }
			, z{ static_cast<T>(z) }
		{}

		template <typename A, typename B, typename C>
			requires std::convertible_to<A, T>&& std::convertible_to<B, T>&& std::convertible_to<C, T>
		constexpr Vec(Vec<1, A> const& v1, Vec<1, B> const& v2, C z)
			: x{ static_cast<T>(v1.x) }
			, y{ static_cast<T>(v2.x) }
			, z{ static_cast<T>(z) }
		{}

		template <typename A, typename B, typename C>
			requires std::convertible_to<A, T>&& std::convertible_to<B, T>&& std::convertible_to<C, T>
		constexpr Vec(A x, B y, Vec<1, C> const& v)
			: x{ static_cast<T>(x) }
			, y{ static_cast<T>(y) }
			, z{ static_cast<T>(v.x) }
		{}

		template <typename A, typename B, typename C>
			requires std::convertible_to<A, T>&& std::convertible_to<B, T>&& std::convertible_to<C, T>
		constexpr Vec(Vec<1, A> const& v1, B y, Vec<1, C> const& v2)
			: x{ static_cast<T>(v1.x) }
			, y{ static_cast<T>(y) }
			, z{ static_cast<T>(v2.x) }
		{}

		template <typename A, typename B, typename C>
			requires std::convertible_to<A, T>&& std::convertible_to<B, T>&& std::convertible_to<C, T>
		constexpr Vec(A x, Vec<1, B> const& v1, Vec<1, C> const& v2)
			: x{ static_cast<T>(x) }
			, y{ static_cast<T>(v1.x) }
			, z{ static_cast<T>(v2.x) }
		{}

		template <typename A, typename B, typename C>
			requires std::convertible_to<A, T>&& std::convertible_to<B, T>&& std::convertible_to<C, T>
		constexpr Vec(Vec<1, A> const& v1, Vec<1, B> const& v2, 
			Vec<1, C> const& v3)
			: x{ static_cast<T>(v1.x) }
			, y{ static_cast<T>(v2.x) }
			, z{ static_cast<T>(v3.x) }
		{}


		template <typename A, typename B>
			requires std::convertible_to<A, T>&& std::convertible_to<B, T>
		constexpr Vec(Vec<2, A> const& xy, B z)
			: x{ static_cast<T>(xy.x) }
			, y{ static_cast<T>(xy.y) }
			, z{ static_cast<T>(z) }
		{}

		template <typename A, typename B>
			requires std::convertible_to<A, T>&& std::convertible_to<B, T>
		constexpr Vec(Vec<2, A> const& xy, Vec<1, B> const& z)
			: x{ static_cast<T>(xy.x) }
			, y{ static_cast<T>(xy.y) }
			, z{ static_cast<T>(z.x) }
		{}

		template <typename A, typename B>
			requires std::convertible_to<A, T>&& std::convertible_to<B, T>
		constexpr Vec(A x, Vec<2, B> const& yz)
			: x{ static_cast<T>(x) }
			, y{ static_cast<T>(yz.x) }
			, z{ static_cast<T>(yz.y) }
		{}

		template <typename A, typename B>
			requires std::convertible_to<A, T>&& std::convertible_to<B, T>
		constexpr Vec(Vec<1, A> const& x, Vec<2, B> const& yz)
			: x{ static_cast<T>(x.x) }
			, y{ static_cast<T>(yz.x) }
			, z{ static_cast<T>(yz.y) }
		{}

		template <typename A>
			requires std::convertible_to<A, T>
		constexpr explicit Vec(Vec<3, A> const& v)
			: x{ static_cast<T>(v.x) }
			, y{ static_cast<T>(v.y) }
			, z{ static_cast<T>(v.z) }
		{}

		constexpr Vec<3, T>& operator=(Vec<3, T> const& v)
		{
			this->x = v.x;
			this->y = v.y;
			this->z = v.z;
			return *this;
		}

		template<typename A>
			requires std::convertible_to<A, T>
		constexpr Vec<3, T>& operator=(Vec<3, A> const& v)
		{
			this->x = static_cast<T>(v.x);
			this->y = static_cast<T>(v.y);
			this->z = static_cast<T>(v.z);
			return *this;
		}
		template<typename A>
			requires std::convertible_to<A, T>
		constexpr Vec<3, T>& operator+=(A scalar)
		{
			this->x += static_cast<T>(scalar);
			this->y += static_cast<T>(scalar);
			this->z += static_cast<T>(scalar);
			return *this;
		}
		template<typename A>
			requires std::convertible_to<A, T>
		constexpr Vec<3, T>& operator+=(Vec<1, A> const& v)
		{
			this->x += static_cast<T>(v.x);
			this->y += static_cast<T>(v.x);
			this->z += static_cast<T>(v.x);
			return *this;
		}
		template<typename A>
			requires std::convertible_to<A, T>
		constexpr Vec<3, T>& operator+=(Vec<3, A> const& v)
		{
			this->x += static_cast<T>(v.x);
			this->y += static_cast<T>(v.y);
			this->z += static_cast<T>(v.z);
			return *this;
		}
		template<typename A>
			requires std::convertible_to<A, T>
		constexpr Vec<3, T>& operator-=(A scalar)
		{
			this->x -= static_cast<T>(scalar);
			this->y -= static_cast<T>(scalar);
			this->z -= static_cast<T>(scalar);
			return *this;
		}
		template<typename A>
			requires std::convertible_to<A, T>
		constexpr Vec<3, T>& operator-=(Vec<1, A> const& v)
		{
			this->x -= static_cast<T>(v.x);
			this->y -= static_cast<T>(v.x);
			this->z -= static_cast<T>(v.x);
			return *this;
		}
		template<typename A>
			requires std::convertible_to<A, T>
		constexpr Vec<3, T>& operator-=(Vec<3, A> const& v)
		{
			this->x -= static_cast<T>(v.x);
			this->y -= static_cast<T>(v.y);
			this->z -= static_cast<T>(v.z);
			return *this;
		}
		template<typename A>
			requires std::convertible_to<A, T>
		constexpr Vec<3, T>& operator*=(A scalar)
		{
			this->x *= static_cast<T>(scalar);
			this->y *= static_cast<T>(scalar);
			this->z *= static_cast<T>(scalar);
			return *this;
		}
		template<typename A>
			requires std::convertible_to<A, T>
		constexpr Vec<3, T>& operator*=(Vec<1, A> const& v)
		{
			this->x *= static_cast<T>(v.x);
			this->y *= static_cast<T>(v.x);
			this->z *= static_cast<T>(v.x);
			return *this;
		}
		template<typename A>
			requires std::convertible_to<A, T>
		constexpr Vec<3, T>& operator*=(Vec<3, A> const& v)
		{
			this->x *= static_cast<T>(v.x);
			this->y *= static_cast<T>(v.y);
			this->z *= static_cast<T>(v.z);
			return *this;
		}
		template<typename A>
			requires std::convertible_to<A, T>
		constexpr Vec<3, T>& operator/=(A scalar)
		{
			this->x /= static_cast<T>(scalar);
			this->y /= static_cast<T>(scalar);
			this->z /= static_cast<T>(scalar);
			return *this;
		}
		template<typename A>
			requires std::convertible_to<A, T>
		constexpr Vec<3, T>& operator/=(Vec<1, A> const& v)
		{
			this->x /= static_cast<T>(v.x);
			this->y /= static_cast<T>(v.x);
			this->z /= static_cast<T>(v.x);
			return *this;
		}
		template<typename A>
			requires std::convertible_to<A, T>
		constexpr Vec<3, T>& operator/=(Vec<3, A> const& v)
		{
			this->x /= static_cast<T>(v.x);
			this->y /= static_cast<T>(v.y);
			this->z /= static_cast<T>(v.z);
			return *this;
		}

		constexpr Vec<3, T>& operator++()
		{
			++this->x;
			++this->y;
			++this->z;
			return *this;
		}
		constexpr Vec<3, T>& operator--()
		{
			--this->x;
			--this->y;
			--this->z;
			return *this;
		}
		constexpr Vec<3, T> operator++(int)
		{
			Vec<3, T> result(*this);
			++*this;
			return result;
		}
		constexpr Vec<3, T> operator--(int)
		{
			Vec<3, T> result(*this);
			--*this;
			return result;
		}

		template<typename A>
			requires std::convertible_to<A, T>
		constexpr Vec<3, T>& operator%=(A scalar)
		{
			this->x %= static_cast<T>(scalar);
			this->y %= static_cast<T>(scalar);
			this->z %= static_cast<T>(scalar);
			return *this;
		}
		template<typename A>
			requires std::convertible_to<A, T>
		constexpr Vec<3, T>& operator%=(Vec<1, A> const& v)
		{
			this->x %= static_cast<T>(v.x);
			this->y %= static_cast<T>(v.x);
			this->z %= static_cast<T>(v.x);
			return *this;
		}
		template<typename A>
			requires std::convertible_to<A, T>
		constexpr Vec<3, T>& operator%=(Vec<3, A> const& v)
		{
			this->x %= static_cast<T>(v.x);
			this->y %= static_cast<T>(v.y);
			this->z %= static_cast<T>(v.z);
			return *this;
		}
		template<typename A>
			requires std::convertible_to<A, T>
		constexpr Vec<3, T>& operator&=(A scalar)
		{
			this->x &= static_cast<T>(scalar);
			this->y &= static_cast<T>(scalar);
			this->z &= static_cast<T>(scalar);
			return *this;
		}
		template<typename A>
			requires std::convertible_to<A, T>
		constexpr Vec<3, T>& operator&=(Vec<1, A> const& v)
		{
			this->x &= static_cast<T>(v.x);
			this->y &= static_cast<T>(v.x);
			this->z &= static_cast<T>(v.x);
			return *this;
		}
		template<typename A>
			requires std::convertible_to<A, T>
		constexpr Vec<3, T>& operator&=(Vec<3, A> const& v)
		{
			this->x &= static_cast<T>(v.x);
			this->y &= static_cast<T>(v.y);
			this->z &= static_cast<T>(v.z);
			return *this;
		}
		template<typename A>
			requires std::convertible_to<A, T>
		constexpr Vec<3, T>& operator|=(A scalar)
		{
			this->x |= static_cast<T>(scalar);
			this->y |= static_cast<T>(scalar);
			this->z |= static_cast<T>(scalar);
			return *this;
		}
		template<typename A>
			requires std::convertible_to<A, T>
		constexpr Vec<3, T>& operator|=(Vec<1, A> const& v)
		{
			this->x |= static_cast<T>(v.x);
			this->y |= static_cast<T>(v.x);
			this->z |= static_cast<T>(v.x);
			return *this;
		}
		template<typename A>
			requires std::convertible_to<A, T>
		constexpr Vec<3, T>& operator|=(Vec<3, A> const& v)
		{
			this->x |= static_cast<T>(v.x);
			this->y |= static_cast<T>(v.y);
			this->z |= static_cast<T>(v.z);
			return *this;
		}
		template<typename A>
			requires std::convertible_to<A, T>
		constexpr Vec<3, T>& operator^=(A scalar)
		{
			this->x ^= static_cast<T>(scalar);
			this->y ^= static_cast<T>(scalar);
			this->z ^= static_cast<T>(scalar);
			return *this;
		}
		template<typename A>
			requires std::convertible_to<A, T>
		constexpr Vec<3, T>& operator^=(Vec<1, A> const& v)
		{
			this->x ^= static_cast<T>(v.x);
			this->y ^= static_cast<T>(v.x);
			this->z ^= static_cast<T>(v.x);
			return *this;
		}
		template<typename A>
			requires std::convertible_to<A, T>
		constexpr Vec<3, T>& operator^=(Vec<3, A> const& v)
		{
			this->x ^= static_cast<T>(v.x);
			this->y ^= static_cast<T>(v.y);
			this->z ^= static_cast<T>(v.z);
			return *this;
		}
		template<typename A>
			requires std::convertible_to<A, T>
		constexpr Vec<3, T>& operator<<=(A scalar)
		{
			this->x <<= static_cast<T>(scalar);
			this->y <<= static_cast<T>(scalar);
			this->z <<= static_cast<T>(scalar);
			return *this;
		}
		template<typename A>
			requires std::convertible_to<A, T>
		constexpr Vec<3, T>& operator<<=(Vec<1, A> const& v)
		{
			this->x <<= static_cast<T>(v.x);
			this->y <<= static_cast<T>(v.x);
			this->z <<= static_cast<T>(v.x);
			return *this;
		}
		template<typename A>
			requires std::convertible_to<A, T>
		constexpr Vec<3, T>& operator<<=(Vec<3, A> const& v)
		{
			this->x <<= static_cast<T>(v.x);
			this->y <<= static_cast<T>(v.y);
			this->z <<= static_cast<T>(v.z);
			return *this;
		}
		template<typename A>
			requires std::convertible_to<A, T>
		constexpr Vec<3, T>& operator>>=(A scalar)
		{
			this->x >>= static_cast<T>(scalar);
			this->y >>= static_cast<T>(scalar);
			this->z >>= static_cast<T>(scalar);
			return *this;
		}
		template<typename A>
			requires std::convertible_to<A, T>
		constexpr Vec<3, T>& operator>>=(Vec<1, A> const& v)
		{
			this->x >>= static_cast<T>(v.x);
			this->y >>= static_cast<T>(v.x);
			this->z >>= static_cast<T>(v.x);
			return *this;
		}
		template<typename A>
			requires std::convertible_to<A, T>
		constexpr Vec<3, T>& operator>>=(Vec<3, A> const& v)
		{
			this->x >>= static_cast<T>(v.x);
			this->y >>= static_cast<T>(v.y);
			this->z >>= static_cast<T>(v.z);
			return *this;
		}
	};

	template<typename T>
	constexpr Vec<3, T> operator+(Vec<3, T> const& v)
	{
		return v;
	}

	template<typename T>
	constexpr Vec<3, T> operator-(Vec<3, T> const& v)
	{
		return Vec<3, T>(-v.x, -v.y, -v.z);
	}

	template<typename T>
	constexpr Vec<3, T> operator+(Vec<3, T> const& v, T scalar)
	{
		return Vec<3, T>(
			v.x + scalar,
			v.y + scalar,
			v.z + scalar
		);
	}

	template<typename T>
	constexpr Vec<3, T> operator+(Vec<3, T> const& v1,
		Vec<1, T> const& v2)
	{
		return Vec<3, T>(
			v1.x + v2.x,
			v1.y + v2.x,
			v1.z + v2.x
		);
	}

	template<typename T>
	constexpr Vec<3, T> operator+(T scalar, Vec<3, T> const& v)
	{
		return Vec<3, T>(
			scalar + v.x,
			scalar + v.y,
			scalar + v.z
		);
	}

	template<typename T>
	constexpr Vec<3, T> operator+(Vec<1, T> const& v1,
		Vec<3, T> const& v2)
	{
		return Vec<3, T>(
			v1.x + v2.x,
			v1.x + v2.y,
			v1.x + v2.z
		);
	}

	template<typename T>
	constexpr Vec<3, T> operator+(Vec<3, T> const& v1,
		Vec<3, T> const& v2)
	{
		return Vec<3, T>(
			v1.x + v2.x,
			v1.y + v2.y,
			v1.z + v2.z
		);
	}

	template<typename T>
	constexpr Vec<3, T> operator-(Vec<3, T> const& v, T scalar)
	{
		return Vec<3, T>(
			v.x - scalar,
			v.y - scalar,
			v.z - scalar
		);
	}

	template<typename T>
	constexpr Vec<3, T> operator-(Vec<3, T> const& v1,
		Vec<1, T> const& v2)
	{
		return Vec<3, T>(
			v1.x - v2.x,
			v1.y - v2.x,
			v1.z - v2.x
		);
	}

	template<typename T>
	constexpr Vec<3, T> operator-(T scalar, Vec<3, T> const& v)
	{
		return Vec<3, T>(
			scalar - v.x,
			scalar - v.y,
			scalar - v.z
		);
	}

	template<typename T>
	constexpr Vec<3, T> operator-(Vec<1, T> const& v1,
		Vec<3, T> const& v2)
	{
		return Vec<3, T>(
			v1.x - v2.x,
			v1.x - v2.y,
			v1.x - v2.z
		);
	}

	template<typename T>
	constexpr Vec<3, T> operator-(Vec<3, T> const& v1,
		Vec<3, T> const& v2)
	{
		return Vec<3, T>(
			v1.x - v2.x,
			v1.y - v2.y,
			v1.z - v2.z
		);
	}

	template<typename T>
	constexpr Vec<3, T> operator*(Vec<3, T> const& v, T scalar)
	{
		return Vec<3, T>(
			v.x * scalar,
			v.y * scalar,
			v.z * scalar
		);
	}

	template<typename T>
	constexpr Vec<3, T> operator*(Vec<3, T> const& v1,
		Vec<1, T> const& v2)
	{
		return Vec<3, T>(
			v1.x * v2.x,
			v1.y * v2.x,
			v1.z * v2.x
		);
	}

	template<typename T>
	constexpr Vec<3, T> operator*(T scalar, Vec<3, T> const& v)
	{
		return Vec<3, T>(
			scalar * v.x,
			scalar * v.y,
			scalar * v.z
		);
	}

	template<typename T>
	constexpr Vec<3, T> operator*(Vec<1, T> const& v1,
		Vec<3, T> const& v2)
	{
		return Vec<3, T>(
			v1.x * v2.x,
			v1.x * v2.y,
			v1.x * v2.z
		);
	}

	template<typename T>
	constexpr Vec<3, T> operator*(Vec<3, T> const& v1,
		Vec<3, T> const& v2)
	{
		return Vec<3, T>(
			v1.x * v2.x,
			v1.y * v2.y,
			v1.z * v2.z
		);
	}

	template<typename T>
	constexpr Vec<3, T> operator/(Vec<3, T> const& v, T scalar)
	{
		return Vec<3, T>(
			v.x / scalar,
			v.y / scalar,
			v.z / scalar
		);
	}

	template<typename T>
	constexpr Vec<3, T> operator/(Vec<3, T> const& v1,
		Vec<1, T> const& v2)
	{
		return Vec<3, T>(
			v1.x / v2.x,
			v1.y / v2.x,
			v1.z / v2.x
		);
	}

	template<typename T>
	constexpr Vec<3, T> operator/(T scalar, Vec<3, T> const& v)
	{
		return Vec<3, T>(
			scalar / v.x,
			scalar / v.y,
			scalar / v.z
		);
	}

	template<typename T>
	constexpr Vec<3, T> operator/(Vec<1, T> const& v1,
		Vec<3, T> const& v2)
	{
		return Vec<3, T>(
			v1.x / v2.x,
			v1.x / v2.y,
			v1.x / v2.z
		);
	}

	template<typename T>
	constexpr Vec<3, T> operator/(Vec<3, T> const& v1,
		Vec<3, T> const& v2)
	{
		return Vec<3, T>(
			v1.x / v2.x,
			v1.y / v2.y,
			v1.z / v2.z
		);
	}

	template<typename T>
	constexpr Vec<3, T> operator%(Vec<3, T> const& v, T scalar)
	{
		return Vec<3, T>(
			v.x % scalar,
			v.y % scalar,
			v.z % scalar
		);
	}

	template<typename T>
	constexpr Vec<3, T> operator%(Vec<3, T> const& v1,
		Vec<1, T> const& v2)
	{
		return Vec<3, T>(
			v1.x % v2.x,
			v1.y % v2.x,
			v1.z % v2.x
		);
	}

	template<typename T>
	constexpr Vec<3, T> operator%(T scalar, Vec<3, T> const& v)
	{
		return Vec<3, T>(
			scalar % v.x,
			scalar % v.y,
			scalar % v.z
		);
	}

	template<typename T>
	constexpr Vec<3, T> operator%(Vec<1, T> const& v1,
		Vec<3, T> const& v2)
	{
		return Vec<3, T>(
			v1.x % v2.x,
			v1.x % v2.y,
			v1.x % v2.z
		);
	}

	template<typename T>
	constexpr Vec<3, T> operator%(Vec<3, T> const& v1,
		Vec<3, T> const& v2)
	{
		return Vec<3, T>(
			v1.x % v2.x,
			v1.y % v2.y,
			v1.z % v2.z
		);
	}

	template<typename T>
	constexpr Vec<3, T> operator&(Vec<3, T> const& v, T scalar)
	{
		return Vec<3, T>(
			v.x & scalar,
			v.y & scalar,
			v.z & scalar
		);
	}

	template<typename T>
	constexpr Vec<3, T> operator&(Vec<3, T> const& v1,
		Vec<1, T> const& v2)
	{
		return Vec<3, T>(
			v1.x & v2.x,
			v1.y & v2.x,
			v1.z & v2.x
		);
	}

	template<typename T>
	constexpr Vec<3, T> operator&(T scalar, Vec<3, T> const& v)
	{
		return Vec<3, T>(
			scalar & v.x,
			scalar & v.y,
			scalar & v.z
		);
	}

	template<typename T>
	constexpr Vec<3, T> operator&(Vec<1, T> const& v1,
		Vec<3, T> const& v2)
	{
		return Vec<3, T>(
			v1.x & v2.x,
			v1.x & v2.y,
			v1.x & v2.z
		);
	}

	template<typename T>
	constexpr Vec<3, T> operator&(Vec<3, T> const& v1,
		Vec<3, T> const& v2)
	{
		return Vec<3, T>(
			v1.x & v2.x,
			v1.y & v2.y,
			v1.z & v2.z
		);
	}

	template<typename T>
	constexpr Vec<3, T> operator|(Vec<3, T> const& v, T scalar)
	{
		return Vec<3, T>(
			v.x | scalar,
			v.y | scalar,
			v.z | scalar
		);
	}

	template<typename T>
	constexpr Vec<3, T> operator|(Vec<3, T> const& v1,
		Vec<1, T> const& v2)
	{
		return Vec<3, T>(
			v1.x | v2.x,
			v1.y | v2.x,
			v1.z | v2.x
		);
	}

	template<typename T>
	constexpr Vec<3, T> operator|(T scalar, Vec<3, T> const& v)
	{
		return Vec<3, T>(
			scalar | v.x,
			scalar | v.y,
			scalar | v.z
		);
	}

	template<typename T>
	constexpr Vec<3, T> operator|(Vec<1, T> const& v1,
		Vec<3, T> const& v2)
	{
		return Vec<3, T>(
			v1.x | v2.x,
			v1.x | v2.y,
			v1.x | v2.z
		);
	}

	template<typename T>
	constexpr Vec<3, T> operator|(Vec<3, T> const& v1,
		Vec<3, T> const& v2)
	{
		return Vec<3, T>(
			v1.x | v2.x,
			v1.y | v2.y,
			v1.z | v2.z
		);
	}

	template<typename T>
	constexpr Vec<3, T> operator^(Vec<3, T> const& v, T scalar)
	{
		return Vec<3, T>(
			v.x ^ scalar,
			v.y ^ scalar,
			v.z ^ scalar
		);
	}

	template<typename T>
	constexpr Vec<3, T> operator^(Vec<3, T> const& v1,
		Vec<1, T> const& v2)
	{
		return Vec<3, T>(
			v1.x ^ v2.x,
			v1.y ^ v2.x,
			v1.z ^ v2.x
		);
	}

	template<typename T>
	constexpr Vec<3, T> operator^(T scalar, Vec<3, T> const& v)
	{
		return Vec<3, T>(
			scalar ^ v.x,
			scalar ^ v.y,
			scalar ^ v.z
		);
	}

	template<typename T>
	constexpr Vec<3, T> operator^(Vec<1, T> const& v1,
		Vec<3, T> const& v2)
	{
		return Vec<3, T>(
			v1.x ^ v2.x,
			v1.x ^ v2.y,
			v1.x ^ v2.z
		);
	}

	template<typename T>
	constexpr Vec<3, T> operator^(Vec<3, T> const& v1,
		Vec<3, T> const& v2)
	{
		return Vec<3, T>(
			v1.x ^ v2.x,
			v1.y ^ v2.y,
			v1.z ^ v2.z
		);
	}

	template<typename T>
	constexpr Vec<3, T> operator<<(Vec<3, T> const& v, T scalar)
	{
		return Vec<3, T>(
			static_cast<T>(v.x << scalar),
			static_cast<T>(v.y << scalar),
			static_cast<T>(v.z << scalar)
		);
	}

	template<typename T>
	constexpr Vec<3, T> operator<<(Vec<3, T> const& v1,
		Vec<1, T> const& v2)
	{
		return Vec<3, T>(
			static_cast<T>(v1.x << v2.x),
			static_cast<T>(v1.y << v2.x),
			static_cast<T>(v1.z << v2.x)
		);
	}

	template<typename T>
	constexpr Vec<3, T> operator<<(T scalar, Vec<3, T> const& v)
	{
		return Vec<3, T>(
			static_cast<T>(scalar << v.x),
			static_cast<T>(scalar << v.y),
			static_cast<T>(scalar << v.z)
		);
	}

	template<typename T>
	constexpr Vec<3, T> operator<<(Vec<1, T> const& v1,
		Vec<3, T> const& v2)
	{
		return Vec<3, T>(
			static_cast<T>(v1.x << v2.x),
			static_cast<T>(v1.x << v2.y),
			static_cast<T>(v1.x << v2.z)
		);
	}

	template<typename T>
	constexpr Vec<3, T> operator<<(Vec<3, T> const& v1,
		Vec<3, T> const& v2)
	{
		return Vec<3, T>(
			static_cast<T>(v1.x << v2.x),
			static_cast<T>(v1.y << v2.y),
			static_cast<T>(v1.z << v2.z)
		);
	}

	template<typename T>
	constexpr Vec<3, T> operator>>(Vec<3, T> const& v, T scalar)
	{
		return Vec<3, T>(
			static_cast<T>(v.x >> scalar),
			static_cast<T>(v.y >> scalar),
			static_cast<T>(v.z >> scalar)
		);
	}

	template<typename T>
	constexpr Vec<3, T> operator>>(Vec<3, T> const& v1,
		Vec<1, T> const& v2)
	{
		return Vec<3, T>(
			static_cast<T>(v1.x >> v2.x),
			static_cast<T>(v1.y >> v2.x),
			static_cast<T>(v1.z >> v2.x)
		);
	}

	template<typename T>
	constexpr Vec<3, T> operator>>(T scalar, Vec<3, T> const& v)
	{
		return Vec<3, T>(
			static_cast<T>(scalar >> v.x),
			static_cast<T>(scalar >> v.y),
			static_cast<T>(scalar >> v.z)
		);
	}

	template<typename T>
	constexpr Vec<3, T> operator>>(Vec<1, T> const& v1,
		Vec<3, T> const& v2)
	{
		return Vec<3, T>(
			static_cast<T>(v1.x >> v2.x),
			static_cast<T>(v1.x >> v2.y),
			static_cast<T>(v1.x >> v2.z)
		);
	}

	template<typename T>
	constexpr Vec<3, T> operator>>(Vec<3, T> const& v1,
		Vec<3, T> const& v2)
	{
		return Vec<3, T>(
			static_cast<T>(v1.x >> v2.x),
			static_cast<T>(v1.y >> v2.y),
			static_cast<T>(v1.z >> v2.z)
		);
	}

	template<typename T>
	constexpr Vec<3, T> operator~(Vec<3, T> const& v)
	{
		return Vec<3, T>(~v.x, ~v.y, ~v.z);
	}

	template<typename T>
	constexpr bool operator==(Vec<3, T> const& v1, Vec<3, T> const& v2)
	{
		return v1.x == v2.x && v1.y == v2.y && v1.z == v2.z;
	}

	template<typename T>
	constexpr bool operator!=(Vec<3, T> const& v1, Vec<3, T> const& v2)
	{
		return !(v1 == v2);
	}

	constexpr Vec<3, bool> operator&&(Vec<3, bool> const& v1,
		Vec<3, bool> const& v2)
	{
		return Vec<3, bool>(v1.x && v2.x, v1.y && v2.y, v1.z && v2.z);
	}

	constexpr Vec<3, bool> operator||(Vec<3, bool> const& v1,
		Vec<3, bool> const& v2)
	{
		return Vec<3, bool>(v1.x || v2.x, v1.y || v2.y, v1.z || v2.z);
	}
}