#pragma once

#include "pbr/math/qualifier.h"

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

		constexpr T& operator[](unsigned int i);
		constexpr T const& operator[](unsigned int i) const;

		constexpr vec() = default;
		constexpr vec(vec const& v) = default;
		template <Qualifier R>
		constexpr vec(vec<2, T, R> const& v);

		constexpr explicit vec(T scalar);
		constexpr vec(T x, T y);

		template <typename A, typename B>
			requires std::convertible_to<A, T>&& std::convertible_to<B, T>
		constexpr vec(A x, B y);
		template <typename A, typename B>
			requires std::convertible_to<A, T>&& std::convertible_to<B, T>
		constexpr vec(vec<1, A, Q> const& x, B y);
		template <typename A, typename B>
			requires std::convertible_to<A, T>&& std::convertible_to<B, T>
		constexpr vec(A x, vec<1, B, Q> const& y);
		template <typename A, typename B>
			requires std::convertible_to<A, T>&& std::convertible_to<B, T>
		constexpr vec(vec<1, A, Q> const& x, vec<1, B, Q> const& y);

		template <typename U, Qualifier R>
		constexpr explicit vec(vec<2, U, R> const& v);
		template <typename U, Qualifier R>
		constexpr explicit vec(vec<3, U, R> const& v);
		template <typename U, Qualifier R>
		constexpr explicit vec(vec<4, U, R> const& v);

		constexpr vec<2, T, Q>& operator=(vec const& v) = default;

		template<typename U>
			requires std::convertible_to<U, T>
		constexpr vec<2, T, Q>& operator=(vec<2, U, Q> const& v);
		template<typename U>
			requires std::convertible_to<U, T>
		constexpr vec<2, T, Q>& operator+=(U scalar);
		template<typename U>
			requires std::convertible_to<U, T>
		constexpr vec<2, T, Q>& operator+=(vec<1, U, Q> const& v);
		template<typename U>
			requires std::convertible_to<U, T>
		constexpr vec<2, T, Q>& operator+=(vec<2, U, Q> const& v);
		template<typename U>
			requires std::convertible_to<U, T>
		constexpr vec<2, T, Q>& operator-=(U scalar);
		template<typename U>
			requires std::convertible_to<U, T>
		constexpr vec<2, T, Q>& operator-=(vec<1, U, Q> const& v);
		template<typename U>
			requires std::convertible_to<U, T>
		constexpr vec<2, T, Q>& operator-=(vec<2, U, Q> const& v);
		template<typename U>
			requires std::convertible_to<U, T>
		constexpr vec<2, T, Q>& operator*=(U scalar);
		template<typename U>
			requires std::convertible_to<U, T>
		constexpr vec<2, T, Q>& operator*=(vec<1, U, Q> const& v);
		template<typename U>
			requires std::convertible_to<U, T>
		constexpr vec<2, T, Q>& operator*=(vec<2, U, Q> const& v);
		template<typename U>
			requires std::convertible_to<U, T>
		constexpr vec<2, T, Q>& operator/=(U scalar);
		template<typename U>
			requires std::convertible_to<U, T>
		constexpr vec<2, T, Q>& operator/=(vec<1, U, Q> const& v);
		template<typename U>
			requires std::convertible_to<U, T>
		constexpr vec<2, T, Q>& operator/=(vec<2, U, Q> const& v);
	};


	template<typename T, Qualifier Q>
	constexpr vec<2, T, Q> operator+(vec<2, T, Q> const& v);

	template<typename T, Qualifier Q>
	constexpr vec<2, T, Q> operator-(vec<2, T, Q> const& v);

	template<typename T, Qualifier Q>
	constexpr vec<2, T, Q> operator+(vec<2, T, Q> const& v, T scalar);

	template<typename T, Qualifier Q>
	constexpr vec<2, T, Q> operator+(vec<2, T, Q> const& v1,
		vec<1, T, Q> const& v2);

	template<typename T, Qualifier Q>
	constexpr vec<2, T, Q> operator+(T scalar, vec<2, T, Q> const& v);

	template<typename T, Qualifier Q>
	constexpr vec<2, T, Q> operator+(vec<1, T, Q> const& v1,
		vec<2, T, Q> const& v2);

	template<typename T, Qualifier Q>
	constexpr vec<2, T, Q> operator+(vec<2, T, Q> const& v1,
		vec<2, T, Q> const& v2);

	template<typename T, Qualifier Q>
	constexpr vec<2, T, Q> operator-(vec<2, T, Q> const& v, T scalar);

	template<typename T, Qualifier Q>
	constexpr vec<2, T, Q> operator-(vec<2, T, Q> const& v1,
		vec<1, T, Q> const& v2);

	template<typename T, Qualifier Q>
	constexpr vec<2, T, Q> operator-(T scalar, vec<2, T, Q> const& v);

	template<typename T, Qualifier Q>
	constexpr vec<2, T, Q> operator-(vec<1, T, Q> const& v1,
		vec<2, T, Q> const& v2);

	template<typename T, Qualifier Q>
	constexpr vec<2, T, Q> operator-(vec<2, T, Q> const& v1,
		vec<2, T, Q> const& v2);

	template<typename T, Qualifier Q>
	constexpr vec<2, T, Q> operator*(vec<2, T, Q> const& v, T scalar);

	template<typename T, Qualifier Q>
	constexpr vec<2, T, Q> operator*(vec<2, T, Q> const& v1,
		vec<1, T, Q> const& v2);

	template<typename T, Qualifier Q>
	constexpr vec<2, T, Q> operator*(T scalar, vec<2, T, Q> const& v);

	template<typename T, Qualifier Q>
	constexpr vec<2, T, Q> operator*(vec<1, T, Q> const& v1,
		vec<2, T, Q> const& v2);

	template<typename T, Qualifier Q>
	constexpr vec<2, T, Q> operator*(vec<2, T, Q> const& v1,
		vec<2, T, Q> const& v2);

	template<typename T, Qualifier Q>
	constexpr vec<2, T, Q> operator/(vec<2, T, Q> const& v, T scalar);

	template<typename T, Qualifier Q>
	constexpr vec<2, T, Q> operator/(vec<2, T, Q> const& v1,
		vec<1, T, Q> const& v2);

	template<typename T, Qualifier Q>
	constexpr vec<2, T, Q> operator/(T scalar, vec<2, T, Q> const& v);

	template<typename T, Qualifier Q>
	constexpr vec<2, T, Q> operator/(vec<1, T, Q> const& v1,
		vec<2, T, Q> const& v2);

	template<typename T, Qualifier Q>
	constexpr vec<2, T, Q> operator/(vec<2, T, Q> const& v1,
		vec<2, T, Q> const& v2);

	template<typename T, Qualifier Q>
	constexpr vec<2, T, Q> operator%(vec<2, T, Q> const& v, T scalar);

	template<typename T, Qualifier Q>
	constexpr vec<2, T, Q> operator%(vec<2, T, Q> const& v1,
		vec<1, T, Q> const& v2);

	template<typename T, Qualifier Q>
	constexpr vec<2, T, Q> operator%(T scalar, vec<2, T, Q> const& v);

	template<typename T, Qualifier Q>
	constexpr vec<2, T, Q> operator%(vec<1, T, Q> const& v1,
		vec<2, T, Q> const& v2);

	template<typename T, Qualifier Q>
	constexpr vec<2, T, Q> operator%(vec<2, T, Q> const& v1,
		vec<2, T, Q> const& v2);

	template<typename T, Qualifier Q>
	constexpr vec<2, T, Q> operator&(vec<2, T, Q> const& v, T scalar);

	template<typename T, Qualifier Q>
	constexpr vec<2, T, Q> operator&(vec<2, T, Q> const& v1,
		vec<1, T, Q> const& v2);

	template<typename T, Qualifier Q>
	constexpr vec<2, T, Q> operator&(T scalar, vec<2, T, Q> const& v);

	template<typename T, Qualifier Q>
	constexpr vec<2, T, Q> operator&(vec<1, T, Q> const& v1,
		vec<2, T, Q> const& v2);

	template<typename T, Qualifier Q>
	constexpr vec<2, T, Q> operator&(vec<2, T, Q> const& v1,
		vec<2, T, Q> const& v2);

	template<typename T, Qualifier Q>
	constexpr vec<2, T, Q> operator|(vec<2, T, Q> const& v, T scalar);

	template<typename T, Qualifier Q>
	constexpr vec<2, T, Q> operator|(vec<2, T, Q> const& v1,
		vec<1, T, Q> const& v2);

	template<typename T, Qualifier Q>
	constexpr vec<2, T, Q> operator|(T scalar, vec<2, T, Q> const& v);

	template<typename T, Qualifier Q>
	constexpr vec<2, T, Q> operator|(vec<1, T, Q> const& v1,
		vec<2, T, Q> const& v2);

	template<typename T, Qualifier Q>
	constexpr vec<2, T, Q> operator|(vec<2, T, Q> const& v1,
		vec<2, T, Q> const& v2);

	template<typename T, Qualifier Q>
	constexpr vec<2, T, Q> operator^(vec<2, T, Q> const& v, T scalar);

	template<typename T, Qualifier Q>
	constexpr vec<2, T, Q> operator^(vec<2, T, Q> const& v1,
		vec<1, T, Q> const& v2);

	template<typename T, Qualifier Q>
	constexpr vec<2, T, Q> operator^(T scalar, vec<2, T, Q> const& v);

	template<typename T, Qualifier Q>
	constexpr vec<2, T, Q> operator^(vec<1, T, Q> const& v1,
		vec<2, T, Q> const& v2);

	template<typename T, Qualifier Q>
	constexpr vec<2, T, Q> operator^(vec<2, T, Q> const& v1,
		vec<2, T, Q> const& v2);

	template<typename T, Qualifier Q>
	constexpr vec<2, T, Q> operator<<(vec<2, T, Q> const& v, T scalar);

	template<typename T, Qualifier Q>
	constexpr vec<2, T, Q> operator<<(vec<2, T, Q> const& v1,
		vec<1, T, Q> const& v2);

	template<typename T, Qualifier Q>
	constexpr vec<2, T, Q> operator<<(T scalar, vec<2, T, Q> const& v);

	template<typename T, Qualifier Q>
	constexpr vec<2, T, Q> operator<<(vec<1, T, Q> const& v1,
		vec<2, T, Q> const& v2);

	template<typename T, Qualifier Q>
	constexpr vec<2, T, Q> operator<<(vec<2, T, Q> const& v1,
		vec<2, T, Q> const& v2);

	template<typename T, Qualifier Q>
	constexpr vec<2, T, Q> operator>>(vec<2, T, Q> const& v, T scalar);

	template<typename T, Qualifier Q>
	constexpr vec<2, T, Q> operator>>(vec<2, T, Q> const& v1,
		vec<1, T, Q> const& v2);

	template<typename T, Qualifier Q>
	constexpr vec<2, T, Q> operator>>(T scalar, vec<2, T, Q> const& v);

	template<typename T, Qualifier Q>
	constexpr vec<2, T, Q> operator>>(vec<1, T, Q> const& v1,
		vec<2, T, Q> const& v2);

	template<typename T, Qualifier Q>
	constexpr vec<2, T, Q> operator>>(vec<2, T, Q> const& v1,
		vec<2, T, Q> const& v2);

	template<typename T, Qualifier Q>
	constexpr vec<2, T, Q> operator~(vec<2, T, Q> const& v);

	template<typename T, Qualifier Q>
	constexpr bool operator==(vec<2, T, Q> const& v1, vec<2, T, Q> const& v2);

	template<typename T, Qualifier Q>
	constexpr bool operator!=(vec<2, T, Q> const& v1, vec<2, T, Q> const& v2);

	template<Qualifier Q>
	constexpr vec<2, bool, Q> operator&&(vec<2, bool, Q> const& v1,
		vec<2, bool, Q> const& v2);

	template<Qualifier Q>
	constexpr vec<2, bool, Q> operator||(vec<2, bool, Q> const& v1,
		vec<2, bool, Q> const& v2);
}