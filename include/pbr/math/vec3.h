#pragma once

#include "pbr/math/qualifier.h"

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

		constexpr T& operator[](unsigned int i);
		constexpr T const& operator[](unsigned int i) const;

		constexpr vec() = default;
		constexpr vec(vec const& v) = default;
		template <Qualifier R>
		constexpr vec(vec<3, T, R> const& v);

		constexpr explicit vec(T scalar);
		constexpr vec(T x, T y);

		template <typename A, typename B, typename C>
			requires std::convertible_to<A, T>&& std::convertible_to<B, T>&& std::convertible_to<C, T>
		constexpr vec(A x, B y, C z);

		template <typename A, typename B, typename C>
			requires std::convertible_to<A, T>&& std::convertible_to<B, T>&& std::convertible_to<C, T>
		constexpr vec(vec<1, A, Q> const& x, B y, C z);

		template <typename A, typename B, typename C>
			requires std::convertible_to<A, T>&& std::convertible_to<B, T>&& std::convertible_to<C, T>
		constexpr vec(A x, vec<1, B, Q> const& y, C z);

		template <typename A, typename B, typename C>
			requires std::convertible_to<A, T>&& std::convertible_to<B, T>&& std::convertible_to<C, T>
		constexpr vec(vec<1, A, Q> const& x, vec<1, B, Q> const& y, C z);

		template <typename A, typename B, typename C>
			requires std::convertible_to<A, T>&& std::convertible_to<B, T>&& std::convertible_to<C, T>
		constexpr vec(A x, B y, vec<1, C, Q> const& z);

		template <typename A, typename B, typename C>
			requires std::convertible_to<A, T>&& std::convertible_to<B, T>&& std::convertible_to<C, T>
		constexpr vec(vec<1, A, Q> const& x, B y, vec<1, C, Q> const& z);

		template <typename A, typename B, typename C>
			requires std::convertible_to<A, T>&& std::convertible_to<B, T>&& std::convertible_to<C, T>
		constexpr vec(A x, vec<1, B, Q> const& y, vec<1, C, Q> const& z);

		template <typename A, typename B, typename C>
			requires std::convertible_to<A, T>&& std::convertible_to<B, T>&& std::convertible_to<C, T>
		constexpr vec(vec<1, A, Q> const& x, vec<1, B, Q> const& y, vec<1, C, Q> const& z);


		template <typename A, typename B>
			requires std::convertible_to<A, T>&& std::convertible_to<B, T>
		constexpr vec(vec<2, A, Q> const& xy, B z);

		template <typename A, typename B>
			requires std::convertible_to<A, T>&& std::convertible_to<B, T>
		constexpr vec(vec<2, A, Q> const& xy, vec<1, B, Q> const& z);

		template <typename A, typename B>
			requires std::convertible_to<A, T>&& std::convertible_to<B, T>
		constexpr vec(A x, vec<2, B, Q> const& yz);

		template <typename A, typename B>
			requires std::convertible_to<A, T>&& std::convertible_to<B, T>
		constexpr vec(vec<1, A, Q> const& x, vec<2, B, Q> const& yz);

		template <typename U, Qualifier R>
		constexpr explicit vec(vec<3, U, R> const& v);
		template <typename U, Qualifier R>
		constexpr explicit vec(vec<4, U, R> const& v);

		constexpr vec<3, T, Q>& operator=(vec const& v) = default;

		template<typename U>
			requires std::convertible_to<U, T>
		constexpr vec<3, T, Q>& operator=(vec<3, U, Q> const& v);
		template<typename U>
			requires std::convertible_to<U, T>
		constexpr vec<3, T, Q>& operator+=(U scalar);
		template<typename U>
			requires std::convertible_to<U, T>
		constexpr vec<3, T, Q>& operator+=(vec<1, U, Q> const& v);
		template<typename U>
			requires std::convertible_to<U, T>
		constexpr vec<3, T, Q>& operator+=(vec<3, U, Q> const& v);
		template<typename U>
			requires std::convertible_to<U, T>
		constexpr vec<3, T, Q>& operator-=(U scalar);
		template<typename U>
			requires std::convertible_to<U, T>
		constexpr vec<3, T, Q>& operator-=(vec<1, U, Q> const& v);
		template<typename U>
			requires std::convertible_to<U, T>
		constexpr vec<3, T, Q>& operator-=(vec<3, U, Q> const& v);
		template<typename U>
			requires std::convertible_to<U, T>
		constexpr vec<3, T, Q>& operator*=(U scalar);
		template<typename U>
			requires std::convertible_to<U, T>
		constexpr vec<3, T, Q>& operator*=(vec<1, U, Q> const& v);
		template<typename U>
			requires std::convertible_to<U, T>
		constexpr vec<3, T, Q>& operator*=(vec<3, U, Q> const& v);
		template<typename U>
			requires std::convertible_to<U, T>
		constexpr vec<3, T, Q>& operator/=(U scalar);
		template<typename U>
			requires std::convertible_to<U, T>
		constexpr vec<3, T, Q>& operator/=(vec<1, U, Q> const& v);
		template<typename U>
			requires std::convertible_to<U, T>
		constexpr vec<3, T, Q>& operator/=(vec<3, U, Q> const& v);
	};


	template<typename T, Qualifier Q>
	constexpr vec<3, T, Q> operator+(vec<3, T, Q> const& v);

	template<typename T, Qualifier Q>
	constexpr vec<3, T, Q> operator-(vec<3, T, Q> const& v);

	template<typename T, Qualifier Q>
	constexpr vec<3, T, Q> operator+(vec<3, T, Q> const& v, T scalar);

	template<typename T, Qualifier Q>
	constexpr vec<3, T, Q> operator+(vec<3, T, Q> const& v1,
		vec<1, T, Q> const& v2);

	template<typename T, Qualifier Q>
	constexpr vec<3, T, Q> operator+(T scalar, vec<3, T, Q> const& v);

	template<typename T, Qualifier Q>
	constexpr vec<3, T, Q> operator+(vec<1, T, Q> const& v1,
		vec<3, T, Q> const& v2);

	template<typename T, Qualifier Q>
	constexpr vec<3, T, Q> operator+(vec<3, T, Q> const& v1,
		vec<3, T, Q> const& v2);

	template<typename T, Qualifier Q>
	constexpr vec<3, T, Q> operator-(vec<3, T, Q> const& v, T scalar);

	template<typename T, Qualifier Q>
	constexpr vec<3, T, Q> operator-(vec<3, T, Q> const& v1,
		vec<1, T, Q> const& v2);

	template<typename T, Qualifier Q>
	constexpr vec<3, T, Q> operator-(T scalar, vec<3, T, Q> const& v);

	template<typename T, Qualifier Q>
	constexpr vec<3, T, Q> operator-(vec<1, T, Q> const& v1,
		vec<3, T, Q> const& v2);

	template<typename T, Qualifier Q>
	constexpr vec<3, T, Q> operator-(vec<3, T, Q> const& v1,
		vec<3, T, Q> const& v2);

	template<typename T, Qualifier Q>
	constexpr vec<3, T, Q> operator*(vec<3, T, Q> const& v, T scalar);

	template<typename T, Qualifier Q>
	constexpr vec<3, T, Q> operator*(vec<3, T, Q> const& v1,
		vec<1, T, Q> const& v2);

	template<typename T, Qualifier Q>
	constexpr vec<3, T, Q> operator*(T scalar, vec<3, T, Q> const& v);

	template<typename T, Qualifier Q>
	constexpr vec<3, T, Q> operator*(vec<1, T, Q> const& v1,
		vec<3, T, Q> const& v2);

	template<typename T, Qualifier Q>
	constexpr vec<3, T, Q> operator*(vec<3, T, Q> const& v1,
		vec<3, T, Q> const& v2);

	template<typename T, Qualifier Q>
	constexpr vec<3, T, Q> operator/(vec<3, T, Q> const& v, T scalar);

	template<typename T, Qualifier Q>
	constexpr vec<3, T, Q> operator/(vec<3, T, Q> const& v1,
		vec<1, T, Q> const& v2);

	template<typename T, Qualifier Q>
	constexpr vec<3, T, Q> operator/(T scalar, vec<3, T, Q> const& v);

	template<typename T, Qualifier Q>
	constexpr vec<3, T, Q> operator/(vec<1, T, Q> const& v1,
		vec<3, T, Q> const& v2);

	template<typename T, Qualifier Q>
	constexpr vec<3, T, Q> operator/(vec<3, T, Q> const& v1,
		vec<3, T, Q> const& v2);

	template<typename T, Qualifier Q>
	constexpr vec<3, T, Q> operator%(vec<3, T, Q> const& v, T scalar);

	template<typename T, Qualifier Q>
	constexpr vec<3, T, Q> operator%(vec<3, T, Q> const& v1,
		vec<1, T, Q> const& v2);

	template<typename T, Qualifier Q>
	constexpr vec<3, T, Q> operator%(T scalar, vec<3, T, Q> const& v);

	template<typename T, Qualifier Q>
	constexpr vec<3, T, Q> operator%(vec<1, T, Q> const& v1,
		vec<3, T, Q> const& v2);

	template<typename T, Qualifier Q>
	constexpr vec<3, T, Q> operator%(vec<3, T, Q> const& v1,
		vec<3, T, Q> const& v2);

	template<typename T, Qualifier Q>
	constexpr vec<3, T, Q> operator&(vec<3, T, Q> const& v, T scalar);

	template<typename T, Qualifier Q>
	constexpr vec<3, T, Q> operator&(vec<3, T, Q> const& v1,
		vec<1, T, Q> const& v2);

	template<typename T, Qualifier Q>
	constexpr vec<3, T, Q> operator&(T scalar, vec<3, T, Q> const& v);

	template<typename T, Qualifier Q>
	constexpr vec<3, T, Q> operator&(vec<1, T, Q> const& v1,
		vec<3, T, Q> const& v2);

	template<typename T, Qualifier Q>
	constexpr vec<3, T, Q> operator&(vec<3, T, Q> const& v1,
		vec<3, T, Q> const& v2);

	template<typename T, Qualifier Q>
	constexpr vec<3, T, Q> operator|(vec<3, T, Q> const& v, T scalar);

	template<typename T, Qualifier Q>
	constexpr vec<3, T, Q> operator|(vec<3, T, Q> const& v1,
		vec<1, T, Q> const& v2);

	template<typename T, Qualifier Q>
	constexpr vec<3, T, Q> operator|(T scalar, vec<3, T, Q> const& v);

	template<typename T, Qualifier Q>
	constexpr vec<3, T, Q> operator|(vec<1, T, Q> const& v1,
		vec<3, T, Q> const& v2);

	template<typename T, Qualifier Q>
	constexpr vec<3, T, Q> operator|(vec<3, T, Q> const& v1,
		vec<3, T, Q> const& v2);

	template<typename T, Qualifier Q>
	constexpr vec<3, T, Q> operator^(vec<3, T, Q> const& v, T scalar);

	template<typename T, Qualifier Q>
	constexpr vec<3, T, Q> operator^(vec<3, T, Q> const& v1,
		vec<1, T, Q> const& v2);

	template<typename T, Qualifier Q>
	constexpr vec<3, T, Q> operator^(T scalar, vec<3, T, Q> const& v);

	template<typename T, Qualifier Q>
	constexpr vec<3, T, Q> operator^(vec<1, T, Q> const& v1,
		vec<3, T, Q> const& v2);

	template<typename T, Qualifier Q>
	constexpr vec<3, T, Q> operator^(vec<3, T, Q> const& v1,
		vec<3, T, Q> const& v2);

	template<typename T, Qualifier Q>
	constexpr vec<3, T, Q> operator<<(vec<3, T, Q> const& v, T scalar);

	template<typename T, Qualifier Q>
	constexpr vec<3, T, Q> operator<<(vec<3, T, Q> const& v1,
		vec<1, T, Q> const& v2);

	template<typename T, Qualifier Q>
	constexpr vec<3, T, Q> operator<<(T scalar, vec<3, T, Q> const& v);

	template<typename T, Qualifier Q>
	constexpr vec<3, T, Q> operator<<(vec<1, T, Q> const& v1,
		vec<3, T, Q> const& v2);

	template<typename T, Qualifier Q>
	constexpr vec<3, T, Q> operator<<(vec<3, T, Q> const& v1,
		vec<3, T, Q> const& v2);

	template<typename T, Qualifier Q>
	constexpr vec<3, T, Q> operator>>(vec<3, T, Q> const& v, T scalar);

	template<typename T, Qualifier Q>
	constexpr vec<3, T, Q> operator>>(vec<3, T, Q> const& v1,
		vec<1, T, Q> const& v2);

	template<typename T, Qualifier Q>
	constexpr vec<3, T, Q> operator>>(T scalar, vec<3, T, Q> const& v);

	template<typename T, Qualifier Q>
	constexpr vec<3, T, Q> operator>>(vec<1, T, Q> const& v1,
		vec<3, T, Q> const& v2);

	template<typename T, Qualifier Q>
	constexpr vec<3, T, Q> operator>>(vec<3, T, Q> const& v1,
		vec<3, T, Q> const& v2);

	template<typename T, Qualifier Q>
	constexpr vec<3, T, Q> operator~(vec<3, T, Q> const& v);

	template<typename T, Qualifier Q>
	constexpr bool operator==(vec<3, T, Q> const& v1, vec<3, T, Q> const& v2);

	template<typename T, Qualifier Q>
	constexpr bool operator!=(vec<3, T, Q> const& v1, vec<3, T, Q> const& v2);

	template<Qualifier Q>
	constexpr vec<3, bool, Q> operator&&(vec<3, bool, Q> const& v1,
		vec<3, bool, Q> const& v2);

	template<Qualifier Q>
	constexpr vec<3, bool, Q> operator||(vec<3, bool, Q> const& v1,
		vec<3, bool, Q> const& v2);
}