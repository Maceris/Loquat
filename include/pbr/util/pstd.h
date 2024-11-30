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


    template <typename T>
    class optional
    {
    public:
        using value_type = T;

        optional() = default;

        LOQUAT_CPU_GPU
        optional(const T& v)
            : set(true)
        {
            new (ptr()) T(v);
        }

        LOQUAT_CPU_GPU
        optional(T&& v)
            : set(true)
        {
            new (ptr()) T(std::move(v));
        }

        LOQUAT_CPU_GPU
        optional(const optional& v)
            : set(v.has_value())
        {
            if (v.has_value())
            {
                new (ptr()) T(v.value());
            }
        }

        LOQUAT_CPU_GPU
        optional(optional&& v)
            : set(v.has_value())
        {
            if (v.has_value())
            {
                new (ptr()) T(std::move(v.value()));
                v.reset();
            }
        }

        LOQUAT_CPU_GPU
        optional& operator=(const T& v)
        {
            reset();
            new (ptr()) T(v);
            set = true;
            return *this;
        }

        LOQUAT_CPU_GPU
        optional& operator=(T&& v)
        {
            reset();
            new (ptr()) T(std::move(v));
            set = true;
            return *this;
        }

        LOQUAT_CPU_GPU
        optional& operator=(const optional& v)
        {
            reset();
            if (v.has_value()) {
                new (ptr()) T(v.value());
                set = true;
            }
            return *this;
        }

        LOQUAT_CPU_GPU
        optional& operator=(optional&& v)
        {
            reset();
            if (v.has_value()) {
                new (ptr()) T(std::move(v.value()));
                set = true;
                v.reset();
            }
            return *this;
        }

        LOQUAT_CPU_GPU
        ~optional()
        {
            reset();
        }

        LOQUAT_CPU_GPU
        explicit operator bool() const
        {
            return set;
        }

        LOQUAT_CPU_GPU
        T value_or(const T& alt) const
        {
            return set ? value() : alt;
        }

        LOQUAT_CPU_GPU
        T* operator->()
        {
            return &value();
        }

        LOQUAT_CPU_GPU
        const T* operator->() const
        {
            return &value();
        }

        LOQUAT_CPU_GPU
        T& operator*()
        {
            return value();
        }

        LOQUAT_CPU_GPU
        const T& operator*() const
        {
            return value();
        }

        LOQUAT_CPU_GPU
        T& value()
        {
            LOG_ASSERT(set);
            return *ptr();
        }

        LOQUAT_CPU_GPU
        const T& value() const
        {
            LOG_ASSERT(set);
            return *ptr();
        }

        LOQUAT_CPU_GPU
        void reset()
        {
            if (set)
            {
                value().~T();
                set = false;
            }
        }

        LOQUAT_CPU_GPU
        bool has_value() const
        {
            return set;
        }

    private:
#ifdef __NVCC__
        // Work around NVCC bug
        LOQUAT_CPU_GPU
        T* ptr()
        {
            return reinterpret_cast<T*>(&optionalValue);
        }

        LOQUAT_CPU_GPU
        const T* ptr() const
        {
            return reinterpret_cast<const T*>(&optionalValue);
        }
#else
        LOQUAT_CPU_GPU
        T* ptr()
        {
            return std::launder(reinterpret_cast<T*>(&optionalValue));
        }

        LOQUAT_CPU_GPU
        const T* ptr() const
        {
            return std::launder(reinterpret_cast<const T*>(&optionalValue));
        }
#endif

        std::aligned_storage_t<sizeof(T), alignof(T)> optionalValue;
        bool set = false;
    };

    template <typename T>
    inline std::ostream& operator<<(std::ostream& os, const optional<T>& opt)
    {
        if (opt.has_value())
        {
            return os << "[ pstd::optional<" << typeid(T).name() <<
                "> set: true " << "value: " << opt.value() << " ]";
        }
        else
        {
            return os << "[ pstd::optional<" << typeid(T).name()
                << "> set: false value: n/a ]";
        }
    }
}
//TODO(ches) finish this