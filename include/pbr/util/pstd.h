// pbrt is Copyright(c) 1998-2020 Matt Pharr, Wenzel Jakob, and Greg Humphreys.
// The pbrt source code is licensed under the Apache License, Version 2.0.
// SPDX: Apache-2.0

// This file has been modified from the original, original notice is above.

#pragma once

#include <float.h>
#include <limits.h>
#include <cassert>
#include <cmath>
#include <cstddef>
#include <cstring>
#include <initializer_list>
#include <iterator>
#include <new>
#include <string>
#include <thread>
#include <type_traits>
#include <typeinfo>
#include <utility>

#include "main/loquat.h"

namespace pstd
{

	template <typename T>
	LOQUAT_CPU_GPU
	inline void swap(T& a, T& b)
	{
		T tmp = std::move(a);
		a = std::move(b);
		b = std::move(tmp);
	}

	template <class To, class From>
	LOQUAT_CPU_GPU
	typename std::enable_if_t<sizeof(To) == sizeof(From) &&
		std::is_trivially_copyable_v<From>&&
		std::is_trivially_copyable_v<To>,
		To>
	bit_cast(const From& src) noexcept
	{
		static_assert(std::is_trivially_constructible_v<To>,
			"This implementation requies the destination type to be "
			"trivially constructible");

		To dst;
		std::memcpy(&dst, &src, sizeof(To));
		return dst;
	}

	template <typename T, int N>
	class array;

    template <typename T>
    class array<T, 0>
    {
    public:
        using value_type = T;
        using iterator = value_type*;
        using const_iterator = const value_type*;
        using size_t = std::size_t;

        array() = default;

        LOQUAT_CPU_GPU
        void fill(const T& v)
        {
            assert(false && "This should never be called");
        }

        LOQUAT_CPU_GPU
        bool operator==(const array<T, 0>& a) const 
        {
            return true;
        }

        LOQUAT_CPU_GPU
        bool operator!=(const array<T, 0>& a) const
        {
            return false;
        }

        LOQUAT_CPU_GPU
        iterator begin()
        {
            return nullptr;
        }

        LOQUAT_CPU_GPU
        iterator end()
        {
            return nullptr;
        }

        LOQUAT_CPU_GPU
        const_iterator begin() const
        {
            return nullptr;
        }
        LOQUAT_CPU_GPU
        const_iterator end() const
        {
            return nullptr;
        }

        LOQUAT_CPU_GPU
        size_t size() const
        {
            return 0;
        }

        LOQUAT_CPU_GPU
        T& operator[](size_t i)
        {
            assert(false && "This should never be called");
            static T t;
            return t;
        }
        LOQUAT_CPU_GPU
        const T& operator[](size_t i) const
        {
            assert(false && "This should never be called");
            static T t;
            return t;
        }

        LOQUAT_CPU_GPU
        T* data()
        {
            return nullptr;
        }

        LOQUAT_CPU_GPU
        const T* data() const
        {
            return nullptr;
        }
    };

    template <typename T, int N>
    class array
    {
    public:
        using value_type = T;
        using iterator = value_type*;
        using const_iterator = const value_type*;
        using size_t = std::size_t;

        array() = default;

        LOQUAT_CPU_GPU
        array(std::initializer_list<T> v)
        {
            size_t i = 0;
            for (const T& val : v)
            {
                values[i++] = val;
            }
        }

        LOQUAT_CPU_GPU
        void fill(const T& v)
        {
            for (int i = 0; i < N; ++i)
            {
                values[i] = v;
            }
        }

        LOQUAT_CPU_GPU
        bool operator==(const array<T, N>& a) const
        {
            for (int i = 0; i < N; ++i)
            {
                if (values[i] != a.values[i])
                {
                    return false;
                }
            }
            return true;
        }

        LOQUAT_CPU_GPU
        bool operator!=(const array<T, N>& a) const
        {
            return !(*this == a);
        }

        LOQUAT_CPU_GPU
        iterator begin()
        {
            return values;
        }

        LOQUAT_CPU_GPU
        iterator end()
        {
            return values + N;
        }

        LOQUAT_CPU_GPU
        const_iterator begin() const
        {
            return values;
        }

        LOQUAT_CPU_GPU
        const_iterator end() const
        {
            return values + N;
        }

        LOQUAT_CPU_GPU
        size_t size() const
        {
            return N;
        }

        LOQUAT_CPU_GPU
        T& operator[](size_t i)
        {
            return values[i];
        }

        LOQUAT_CPU_GPU
        const T& operator[](size_t i) const
        {
            return values[i];
        }

        LOQUAT_CPU_GPU
        T* data()
        { 
           return values;
        }

        LOQUAT_CPU_GPU
        const T* data() const
        {
            return values;
        }

    private:
        T values[N] = {};
    };
}
//TODO(ches) finish this