#pragma once

#include "pbr/math/Qualifier.h"

namespace loquat
{
	template <typename T, Qualifier Q>
	struct vec<1, T, Q>
	{
		using value_type = T;
		using type = vec<1, T, Q>;
		union
		{
			struct { T x; };
			struct { T r; };
			struct { T s; };
		};

		static constexpr unsigned int length()
		{
			return 1;
		}

		constexpr T& operator[](unsigned int i);
		constexpr T const& operator[](unsigned int i) const;

		constexpr vec() = default;
		constexpr vec(vec const& v) = default;
		template <Qualifier R>
		constexpr vec(vec<1, T, R> const& v);

		constexpr explicit vec(T scalar);
		constexpr vec(T x, T y);

		template <typename A>
			requires std::convertible_to<A, T>
		constexpr vec(A x);

		constexpr vec<1, T, Q>& operator=(vec const& v) = default;

		template<typename U>
		constexpr vec<1, T, Q>& operator=(vec<1, U, Q> const& v);
		template<typename U>
		constexpr vec<1, T, Q>& operator+=(U scalar);
		template<typename U>
		constexpr vec<1, T, Q>& operator+=(vec<1, U, Q> const& v);
		template<typename U>
		constexpr vec<1, T, Q>& operator-=(U scalar);
		template<typename U>
		constexpr vec<1, T, Q>& operator-=(vec<1, U, Q> const& v);
		template<typename U>
		constexpr vec<1, T, Q>& operator*=(U scalar);
		template<typename U>
		constexpr vec<1, T, Q>& operator*=(vec<1, U, Q> const& v);
		template<typename U>
		constexpr vec<1, T, Q>& operator/=(U scalar);
		template<typename U>
		constexpr vec<1, T, Q>& operator/=(vec<1, U, Q> const& v);

		constexpr vec<1, T, Q>& operator++();
		constexpr vec<1, T, Q>& operator--();
		constexpr vec<1, T, Q> operator++(int);
		constexpr vec<1, T, Q> operator--(int);
	};

	template<typename T, Qualifier Q>
	constexpr vec<1, T, Q> operator+(vec<1, T, Q> const& v);

	template<typename T, Qualifier Q>
	constexpr vec<1, T, Q> operator-(vec<1, T, Q> const& v);

	template<typename T, Qualifier Q>
	constexpr vec<1, T, Q> operator+(vec<1, T, Q> const& v, T scalar);

	template<typename T, Qualifier Q>
	constexpr vec<1, T, Q> operator+(T scalar, vec<1, T, Q> const& v);

	template<typename T, Qualifier Q>
	constexpr vec<1, T, Q> operator+(vec<1, T, Q> const& v1, vec<1, T, Q> const& v2);

	template<typename T, Qualifier Q>
	constexpr vec<1, T, Q> operator-(vec<1, T, Q> const& v, T scalar);

	template<typename T, Qualifier Q>
	constexpr vec<1, T, Q> operator-(T scalar, vec<1, T, Q> const& v);

	template<typename T, Qualifier Q>
	constexpr vec<1, T, Q> operator-(vec<1, T, Q> const& v1, vec<1, T, Q> const& v2);

	template<typename T, Qualifier Q>
	constexpr vec<1, T, Q> operator*(vec<1, T, Q> const& v, T scalar);

	template<typename T, Qualifier Q>
	constexpr vec<1, T, Q> operator*(T scalar, vec<1, T, Q> const& v);

	template<typename T, Qualifier Q>
	constexpr vec<1, T, Q> operator*(vec<1, T, Q> const& v1, vec<1, T, Q> const& v2);

	template<typename T, Qualifier Q>
	constexpr vec<1, T, Q> operator/(vec<1, T, Q> const& v, T scalar);

	template<typename T, Qualifier Q>
	constexpr vec<1, T, Q> operator/(T scalar, vec<1, T, Q> const& v);

	template<typename T, Qualifier Q>
	constexpr vec<1, T, Q> operator/(vec<1, T, Q> const& v1, vec<1, T, Q> const& v2);

	template<typename T, Qualifier Q>
	constexpr vec<1, T, Q> operator%(vec<1, T, Q> const& v, T scalar);

	template<typename T, Qualifier Q>
	constexpr vec<1, T, Q> operator%(T scalar, vec<1, T, Q> const& v);

	template<typename T, Qualifier Q>
	constexpr vec<1, T, Q> operator%(vec<1, T, Q> const& v1, vec<1, T, Q> const& v2);

	template<typename T, Qualifier Q>
	constexpr vec<1, T, Q> operator&(vec<1, T, Q> const& v, T scalar);

	template<typename T, Qualifier Q>
	constexpr vec<1, T, Q> operator&(T scalar, vec<1, T, Q> const& v);

	template<typename T, Qualifier Q>
	constexpr vec<1, T, Q> operator&(vec<1, T, Q> const& v1, vec<1, T, Q> const& v2);

	template<typename T, Qualifier Q>
	constexpr vec<1, T, Q> operator|(vec<1, T, Q> const& v, T scalar);

	template<typename T, Qualifier Q>
	constexpr vec<1, T, Q> operator|(T scalar, vec<1, T, Q> const& v);

	template<typename T, Qualifier Q>
	constexpr vec<1, T, Q> operator|(vec<1, T, Q> const& v1, vec<1, T, Q> const& v2);

	template<typename T, Qualifier Q>
	constexpr vec<1, T, Q> operator^(vec<1, T, Q> const& v, T scalar);

	template<typename T, Qualifier Q>
	constexpr vec<1, T, Q> operator^(T scalar, vec<1, T, Q> const& v);

	template<typename T, Qualifier Q>
	constexpr vec<1, T, Q> operator^(vec<1, T, Q> const& v1, vec<1, T, Q> const& v2);

	template<typename T, Qualifier Q>
	constexpr vec<1, T, Q> operator<<(vec<1, T, Q> const& v, T scalar);

	template<typename T, Qualifier Q>
	constexpr vec<1, T, Q> operator<<(T scalar, vec<1, T, Q> const& v);

	template<typename T, Qualifier Q>
	constexpr vec<1, T, Q> operator<<(vec<1, T, Q> const& v1, vec<1, T, Q> const& v2);

	template<typename T, Qualifier Q>
	constexpr vec<1, T, Q> operator>>(vec<1, T, Q> const& v, T scalar);

	template<typename T, Qualifier Q>
	constexpr vec<1, T, Q> operator>>(T scalar, vec<1, T, Q> const& v);

	template<typename T, Qualifier Q>
	constexpr vec<1, T, Q> operator>>(vec<1, T, Q> const& v1, vec<1, T, Q> const& v2);

	template<typename T, Qualifier Q>
	constexpr vec<1, T, Q> operator~(vec<1, T, Q> const& v);

	template<typename T, Qualifier Q>
	constexpr bool operator==(vec<1, T, Q> const& v1, vec<1, T, Q> const& v2);

	template<typename T, Qualifier Q>
	constexpr bool operator!=(vec<1, T, Q> const& v1, vec<1, T, Q> const& v2);

	template<Qualifier Q>
	constexpr vec<1, bool, Q> operator&&(vec<1, bool, Q> const& v1, vec<1, bool, Q> const& v2);

	template<Qualifier Q>
	constexpr vec<1, bool, Q> operator||(vec<1, bool, Q> const& v1, vec<1, bool, Q> const& v2);
}