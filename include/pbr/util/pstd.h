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
#include <iostream>
#include <iterator>
#include <new>
#include <string>
#include <thread>
#include <type_traits>
#include <typeinfo>
#include <utility>
#include <vector>

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

    namespace span_internal {

        // Wrappers for access to container data pointers.
        template <typename C>
        LOQUAT_CPU_GPU
        inline constexpr auto get_data_impl(C& c, char) noexcept
            -> decltype(c.data())
        {
            return c.data();
        }

        template <typename C>
        LOQUAT_CPU_GPU
        inline constexpr auto get_data(C& c) noexcept
            -> decltype(get_data_impl(c, 0))
        {
            return get_data_impl(c, 0);
        }

        template <typename C>
        using HasSize =
            std::is_integral<typename std::decay_t<
            decltype(std::declval<C&>().size())>>;

        template <typename T, typename C>
        using HasData =
            std::is_convertible<typename std::decay_t<
            decltype(get_data(std::declval<C&>()))>*,
            T* const*>;
    }

    inline constexpr std::size_t dynamic_extent = -1;

    // span implementation partially based on absl::Span from Google's Abseil library.
    template <typename T>
    class span
    {
    public:
        template <typename C>
        using EnableIfConvertibleFrom =
            typename std::enable_if_t<span_internal::HasData<T, C>::value&&
            span_internal::HasSize<C>::value>;

        template <typename U>
        using EnableIfConstView = typename std::enable_if_t<std::is_const_v<T>, U>;

        template <typename U>
        using EnableIfMutableView = typename std::enable_if_t<!std::is_const_v<T>, U>;

        using value_type = typename std::remove_cv_t<T>;
        using iterator = T*;
        using const_iterator = const T*;

        LOQUAT_CPU_GPU
        span()
            : ptr{ nullptr }
            , n{ 0 }
        {}

        LOQUAT_CPU_GPU
        span(T* ptr, size_t n)
            : ptr{ ptr }
            , n{ n }
        {}

        template <size_t N>
        LOQUAT_CPU_GPU span(T(&a)[N])
            : span(a, N)
        {}

        LOQUAT_CPU_GPU
        span(std::initializer_list<value_type> v)
            : span(v.begin(), v.size())
        {}

        template <typename V, typename X = EnableIfConvertibleFrom<V>,
            typename Y = EnableIfMutableView<V>>
        LOQUAT_CPU_GPU explicit span(V& v) noexcept
            : span(v.data(), v.size())
        {}

        template <typename V>
        span(std::vector<V>& v) noexcept
            : span(v.data(), v.size())
        {}

        template <typename V>
        span(const std::vector<V>& v) noexcept
            : span(v.data(), v.size())
        {}

        template <typename V, typename X = EnableIfConvertibleFrom<V>,
            typename Y = EnableIfConstView<V>>
        LOQUAT_CPU_GPU
        constexpr span(const V& v) noexcept
            : span(v.data(), v.size())
        {}

        LOQUAT_CPU_GPU
        iterator begin()
        {
            return ptr;
        }

        LOQUAT_CPU_GPU
        iterator end()
        {
            return ptr + n;
        }

        LOQUAT_CPU_GPU
        const_iterator begin() const
        {
            return ptr;
        }

        LOQUAT_CPU_GPU
        const_iterator end() const 
        {
            return ptr + n;
        }

        LOQUAT_CPU_GPU
        T& operator[](size_t i)
        {
            LOG_ASSERT(i < size());
            return ptr[i];
        }
        LOQUAT_CPU_GPU
            const T& operator[](size_t i) const
        {
            LOG_ASSERT(i < size());
            return ptr[i];
        }

        LOQUAT_CPU_GPU
        size_t size() const
        {
            return n;
        }

        LOQUAT_CPU_GPU
        bool empty() const
        {
            return size() == 0;
        }

        LOQUAT_CPU_GPU
        T* data()
        {
            return ptr;
        }

        LOQUAT_CPU_GPU
        const T* data() const
        {
            return ptr;
        }

        LOQUAT_CPU_GPU
        T front() const
        {
            return ptr[0];
        }

        LOQUAT_CPU_GPU
        T back() const
        {
            return ptr[n - 1];
        }

        LOQUAT_CPU_GPU
        void remove_prefix(size_t count)
        {
            ptr += count;
            n -= count;
        }

        LOQUAT_CPU_GPU
        void remove_suffix(size_t count)
        {
            n -= count;
        }

        LOQUAT_CPU_GPU
        span subspan(size_t pos, size_t count = dynamic_extent)
        {
            size_t np = count < (size() - pos) ? count : (size() - pos);
            return span(ptr + pos, np);
        }

    private:
        T* ptr;
        size_t n;
    };

    template <int &...ExplicitArgumentBarrier, typename T>
    LOQUAT_CPU_GPU
    inline constexpr span<T> make_span(T* ptr, size_t size) noexcept
    {
        return span<T>(ptr, size);
    }

    template <int &...ExplicitArgumentBarrier, typename T>
    LOQUAT_CPU_GPU
    inline span<T> make_span(T* begin, T* end) noexcept
    {
        return span<T>(begin, end - begin);
    }

    template <int &...ExplicitArgumentBarrier, typename T>
    inline span<T> make_span(std::vector<T>& v) noexcept
    {
        return span<T>(v.data(), v.size());
    }

    template <int &...ExplicitArgumentBarrier, typename C>
    LOQUAT_CPU_GPU
    inline constexpr auto make_span(C& c) noexcept
        -> decltype(make_span(span_internal::get_data(c), c.size()))
    {
        return make_span(span_internal::get_data(c), c.size());
    }

    template <int &...ExplicitArgumentBarrier, typename T, size_t N>
    LOQUAT_CPU_GPU
    inline constexpr span<T> make_span(T(&array)[N]) noexcept
    {
        return span<T>(array, N);
    }

    template <int &...ExplicitArgumentBarrier, typename T>
    LOQUAT_CPU_GPU
    inline constexpr span<const T> make_const_span(T* ptr, size_t size) noexcept
    {
        return span<const T>(ptr, size);
    }

    template <int &...ExplicitArgumentBarrier, typename T>
    LOQUAT_CPU_GPU
    inline span<const T> make_const_span(T* begin, T* end) noexcept
    {
        return span<const T>(begin, end - begin);
    }

    template <int &...ExplicitArgumentBarrier, typename T>
    inline span<const T> make_const_span(const std::vector<T>& v) noexcept
    {
        return span<const T>(v.data(), v.size());
    }

    template <int &...ExplicitArgumentBarrier, typename C>
    LOQUAT_CPU_GPU
    inline constexpr auto make_const_span(const C& c) noexcept
        -> decltype(make_span(c))
    {
        return make_span(c);
    }

    template <int &...ExplicitArgumentBarrier, typename T, size_t N>
    LOQUAT_CPU_GPU
    inline constexpr span<const T> make_const_span(const T(&array)[N]) noexcept
    {
        return span<const T>(array, N);
    }


    namespace pmr
    {

        class memory_resource
        {
            static constexpr size_t max_align = alignof(std::max_align_t);

        public:
            virtual ~memory_resource();
            void* allocate(size_t bytes, size_t alignment = max_align)
            {
                if (bytes == 0)
                {
                    return nullptr;
                }
                return do_allocate(bytes, alignment);
            }
            void deallocate(void* p, size_t bytes, size_t alignment = max_align)
            {
                if (!p)
                {
                    return;
                }
                return do_deallocate(p, bytes, alignment);
            }
            bool is_equal(const memory_resource& other) const noexcept
            {
                return do_is_equal(other);
            }

        private:
            virtual void* do_allocate(size_t bytes, size_t alignment) = 0;
            virtual void do_deallocate(void* p, size_t bytes, size_t alignment) = 0;
            virtual bool do_is_equal(const memory_resource& other) const noexcept = 0;
        };

        inline bool operator==(const memory_resource& a, const memory_resource& b) noexcept
        {
            return a.is_equal(b);
        }

        inline bool operator!=(const memory_resource& a, const memory_resource& b) noexcept
        {
            return !(a == b);
        }

        // TODO(ches)
        struct pool_options
        {
            size_t max_blocks_per_chunk = 0;
            size_t largest_required_pool_block = 0;
        };
        class synchronized_pool_resource;
        class unsynchronized_pool_resource;

        // global memory resources
        memory_resource* new_delete_resource() noexcept;
#if 0
        // TODO(ches)
        memory_resource* null_memory_resource() noexcept;
#endif
        memory_resource* set_default_resource(memory_resource* r) noexcept;
        memory_resource* get_default_resource() noexcept;

        class alignas(64) monotonic_buffer_resource : public memory_resource
        {
        public:
            explicit monotonic_buffer_resource(memory_resource* upstream)
                : upstream(upstream)
            {
#ifdef _DEBUG
                constructTID = std::this_thread::get_id();
#endif
            }
            monotonic_buffer_resource(size_t block_size, memory_resource* upstream)
                : block_size(block_size), upstream(upstream)
            {
#ifdef _DEBUG
                constructTID = std::this_thread::get_id();
#endif
            }
#if 0
            // TODO(ches)
            monotonic_buffer_resource(void* buffer, size_t buffer_size,
                memory_resource* upstream);
#endif
            monotonic_buffer_resource()
                : monotonic_buffer_resource(get_default_resource())
            {}
            explicit monotonic_buffer_resource(size_t initial_size)
                : monotonic_buffer_resource(initial_size, get_default_resource())
            {}
#if 0
            // TODO(ches)
            monotonic_buffer_resource(void* buffer, size_t buffer_size)
                : monotonic_buffer_resource(buffer, buffer_size, get_default_resource())
            {}
#endif
            monotonic_buffer_resource(const monotonic_buffer_resource&) = delete;

            ~monotonic_buffer_resource()
            {
                release();
            }

            monotonic_buffer_resource operator=(const monotonic_buffer_resource&) = delete;

            void release()
            {
                block* b = block_list;
                while (b)
                {
                    block* next = b->next;
                    free_block(b);
                    b = next;
                }
                block_list = nullptr;
                current = nullptr;
            }

            memory_resource* upstream_resource() const
            {
                return upstream;
            }

        protected:
            void* do_allocate(size_t bytes, size_t align) override;

            void do_deallocate(void* p, size_t bytes, size_t alignment) override
            {
                if (bytes > block_size)
                {
                    upstream->deallocate(p, bytes);
                }
            }

            bool do_is_equal(const memory_resource& other) const noexcept override
            {
                return this == &other;
            }

        private:
            struct block
            {
                void* ptr;
                size_t size;
                block* next;
            };

            block* allocate_block(size_t size)
            {
                block* b = static_cast<block*>(
                    upstream->allocate(sizeof(block) + size, alignof(block)));

                b->ptr = reinterpret_cast<char*>(b) + sizeof(block);
                b->size = size;
                b->next = block_list;
                block_list = b;

                return b;
            }
            void free_block(block* b)
            {
                upstream->deallocate(b, sizeof(block) + b->size);
            }

#ifdef _DEBUG
            std::thread::id constructTID;
#endif
            memory_resource* upstream;
            size_t block_size = 256 * 1024;
            block* current = nullptr;
            size_t current_pos = 0;
            block* block_list = nullptr;
        };

        template <class Tp = std::byte>
        class polymorphic_allocator
        {
        public:
            using value_type = Tp;

            polymorphic_allocator() noexcept
            {
                memoryResource = new_delete_resource();
            }
            polymorphic_allocator(memory_resource* r)
                : memoryResource(r)
            {}
            polymorphic_allocator(const polymorphic_allocator& other) = default;
            template <class U>
            polymorphic_allocator(const polymorphic_allocator<U>& other) noexcept
                : memoryResource(other.resource())
            {}

            polymorphic_allocator& operator=(const polymorphic_allocator& rhs) = delete;

            [[nodiscard]]
            Tp* allocate(size_t n)
            {
                return static_cast<Tp*>(resource()->allocate(n * sizeof(Tp), alignof(Tp)));
            }

            void deallocate(Tp* p, size_t n)
            {
                resource()->deallocate(p, n * sizeof(Tp));
            }

            void* allocate_bytes(size_t nbytes, size_t alignment = alignof(max_align_t))
            {
                return resource()->allocate(nbytes, alignment);
            }

            void deallocate_bytes(void* p, size_t nbytes,
                size_t alignment = alignof(std::max_align_t))
            {
                return resource()->deallocate(p, nbytes, alignment);
            }

            template <class T>
            T* allocate_object(size_t n = 1)
            {
                return static_cast<T*>(allocate_bytes(n * sizeof(T), alignof(T)));
            }

            template <class T>
            void deallocate_object(T* p, size_t n = 1)
            {
                deallocate_bytes(p, n * sizeof(T), alignof(T));
            }
            template <class T, class... Args>
            T* new_object(Args &&...args)
            {
                // NOTE: this doesn't handle constructors that throw exceptions
                T* p = allocate_object<T>();
                construct(p, std::forward<Args>(args)...);
                return p;
            }

            template <class T>
            void delete_object(T* p)
            {
                destroy(p);
                deallocate_object(p);
            }

            template <class T, class... Args>
            void construct(T* p, Args &&...args)
            {
                ::new ((void*)p) T(std::forward<Args>(args)...);
            }

            template <class T>
            void destroy(T* p)
            {
                p->~T();
            }
#if 0
            polymorphic_allocator select_on_container_copy_construction() const;
#endif
            memory_resource* resource() const
            {
                return memoryResource;
            }

        private:
            memory_resource* memoryResource;
        };

        template <class T1, class T2>
        bool operator==(const polymorphic_allocator<T1>& a,
            const polymorphic_allocator<T2>& b) noexcept
        {
            return a.resource() == b.resource();
        }

        template <class T1, class T2>
        bool operator!=(const polymorphic_allocator<T1>& a,
            const polymorphic_allocator<T2>& b) noexcept
        {
            return !(a == b);
        }

    }

    template <typename T, class Allocator = pmr::polymorphic_allocator<T>>
    class vector
    {
    public:
        using value_type = T;
        using allocator_type = Allocator;
        using size_type = std::size_t;
        using difference_type = std::ptrdiff_t;
        using reference = value_type&;
        using const_reference = const value_type&;
        using pointer = T*;
        using const_pointer = const T*;
        using iterator = T*;
        using const_iterator = const T*;
        using reverse_iterator = std::reverse_iterator<iterator>;
        using const_reverse_iterator = std::reverse_iterator<const iterator>;

        vector(const Allocator& alloc = {})
            : alloc{ alloc }
        {}

        vector(size_t count, const T& value, const Allocator& alloc = {})
            : alloc{ alloc }
        {
            reserve(count);
            for (size_t i = 0; i < count; ++i)
            {
                this->alloc.template construct<T>(ptr + i, value);
            }
            stored_count = count;
        }

        vector(size_t count, const Allocator& alloc = {})
            : vector{ count, T{}, alloc }
        {}

        vector(const vector& other, const Allocator& alloc = {})
            : alloc{ alloc }
        {
            reserve(other.size());
            for (size_t i = 0; i < other.size(); ++i)
            {
                this->alloc.template construct<T>(ptr + i, other[i]);
            }
            stored_count = other.size();
        }

        template <class InputIt>
        vector(InputIt first, InputIt last, const Allocator& alloc = {})
            : alloc{ alloc }
        {
            reserve(last - first);
            size_t i = 0;
            for (InputIt iter = first; iter != last; ++iter, ++i)
                this->alloc.template construct<T>(ptr + i, *iter);
            stored_count = allocated_count;
        }

        vector(vector&& other)
            : alloc{ other.alloc }
        {
            stored_count = other.stored_count;
            allocated_count = other.allocated_count;
            ptr = other.ptr;

            other.stored_count = other.allocated_count = 0;
            other.ptr = nullptr;
        }

        vector(vector&& other, const Allocator& alloc)
        {
            if (alloc == other.alloc)
            {
                ptr = other.ptr;
                allocated_count = other.allocated_count;
                stored_count = other.stored_count;

                other.ptr = nullptr;
                other.allocated_count = other.stored_count = 0;
            }
            else
            {
                reserve(other.size());
                for (size_t i = 0; i < other.size(); ++i)
                {
                    alloc.template construct<T>(ptr + i, std::move(other[i]));
                }
                stored_count = other.size();
            }
        }
        vector(std::initializer_list<T> init, const Allocator& alloc = {})
            : vector{ init.begin(), init.end(), alloc }
        {}

        vector& operator=(const vector& other)
        {
            if (this == &other)
            {
                return *this;
            }

            clear();
            reserve(other.size());
            for (size_t i = 0; i < other.size(); ++i)
            {
                alloc.template construct<T>(ptr + i, other[i]);
            }
            stored_count = other.size();

            return *this;
        }

        vector& operator=(vector&& other)
        {
            if (this == &other)
            {
                return *this;
            }

            if (alloc == other.alloc)
            {
                pstd::swap(ptr, other.ptr);
                pstd::swap(allocated_count, other.allocated_count);
                pstd::swap(stored_count, other.stored_count);
            }
            else {
                clear();
                reserve(other.size());
                for (size_t i = 0; i < other.size(); ++i)
                {
                    alloc.template construct<T>(ptr + i, std::move(other[i]));
                }
                stored_count = other.size();
            }

            return *this;
        }

        vector& operator=(std::initializer_list<T>& init)
        {
            reserve(init.size());
            clear();
            iterator iter = begin();
            for (const auto& value : init)
            {
                *iter = value;
                ++iter;
            }
            return *this;
        }

        void assign(size_type count, const T& value)
        {
            clear();
            reserve(count);
            for (size_t i = 0; i < count; ++i)
            {
                push_back(value);
            }
        }

        template <class InputIt>
        void assign(InputIt first, InputIt last)
        {
            // TODO(ches)
            LOG_FATAL("TODO");
        }

        void assign(std::initializer_list<T>& init)
        {
            assign(init.begin(), init.end());
        }

        ~vector()
        {
            clear();
            alloc.deallocate_object(ptr, allocated_count);
        }

        LOQUAT_CPU_GPU
        iterator begin()
        {
            return ptr;
        }

        LOQUAT_CPU_GPU
        iterator end()
        {
            return ptr + stored_count;
        }

        LOQUAT_CPU_GPU
        const_iterator begin() const
        {
            return ptr;
        }

        LOQUAT_CPU_GPU
        const_iterator end() const
        {
            return ptr + stored_count;
        }

        LOQUAT_CPU_GPU
        const_iterator cbegin() const
        {
            return ptr;
        }

        LOQUAT_CPU_GPU
        const_iterator cend() const
        {
            return ptr + stored_count;
        }

        LOQUAT_CPU_GPU
        reverse_iterator rbegin()
        {
            return reverse_iterator(end());
        }

        LOQUAT_CPU_GPU
        reverse_iterator rend()
        {
            return reverse_iterator(begin());
        }

        LOQUAT_CPU_GPU
        const_reverse_iterator rbegin() const
        {
            return const_reverse_iterator(end());
        }

        LOQUAT_CPU_GPU
        const_reverse_iterator rend() const
        {
            return const_reverse_iterator(begin());
        }

        allocator_type get_allocator() const
        {
            return alloc;
        }

        LOQUAT_CPU_GPU
        size_t size() const
        {
            return stored_count;
        }

        LOQUAT_CPU_GPU
        bool empty() const
        {
            return size() == 0;
        }

        LOQUAT_CPU_GPU
        size_t max_size() const
        {
            return static_cast<size_t>(-1);
        }

        LOQUAT_CPU_GPU
        size_t capacity() const
        {
            return allocated_count;
        }

        void reserve(size_t n)
        {
            if (allocated_count >= n)
            {
                return;
            }

            T* ra = alloc.template allocate_object<T>(n);
            for (int i = 0; i < stored_count; ++i)
            {
                alloc.template construct<T>(ra + i, std::move(begin()[i]));
                alloc.destroy(begin() + i);
            }

            alloc.deallocate_object(ptr, allocated_count);
            allocated_count = n;
            ptr = ra;
        }
        // TODO(ches): shrink_to_fit

        LOQUAT_CPU_GPU
        reference operator[](size_type index)
        {
            LOG_ASSERT(index < size());
            return ptr[index];
        }

        LOQUAT_CPU_GPU
        const_reference operator[](size_type index) const
        {
            LOG_ASSERT(index < size());
            return ptr[index];
        }

        LOQUAT_CPU_GPU
        reference front()
        {
            return ptr[0];
        }

        LOQUAT_CPU_GPU
        const_reference front() const
        {
            return ptr[0];
        }

        LOQUAT_CPU_GPU
        reference back()
        {
            return ptr[stored_count - 1];
        }

        LOQUAT_CPU_GPU
        const_reference back() const
        {
            return ptr[stored_count - 1];
        }

        LOQUAT_CPU_GPU
        pointer data()
        {
            return ptr;
        }

        LOQUAT_CPU_GPU
        const_pointer data() const
        {
            return ptr;
        }

        void clear()
        {
            for (int i = 0; i < stored_count; ++i)
            {
                alloc.destroy(&ptr[i]);
            }
            stored_count = 0;
        }

        iterator insert(const_iterator, const T& value)
        {
            // TODO(ches)
            LOG_FATAL("TODO");
        }

        iterator insert(const_iterator, T&& value)
        {
            // TODO(ches)
            LOG_FATAL("TODO");
        }

        iterator insert(const_iterator pos, size_type count, const T& value)
        {
            // TODO(ches)
            LOG_FATAL("TODO");
        }

        template <class InputIt>
        iterator insert(const_iterator pos, InputIt first, InputIt last)
        {
            if (pos == end())
            {
                size_t firstOffset = size();
                for (auto iter = first; iter != last; ++iter)
                {
                    push_back(*iter);
                }
                return begin() + firstOffset;
            }
            else
            {
                LOG_FATAL("TODO");
            }
        }

        iterator insert(const_iterator pos, std::initializer_list<T> init)
        {
            // TODO(ches)
            LOG_FATAL("TODO");
        }

        template <class... Args>
        iterator emplace(const_iterator pos, Args &&...args)
        {
            // TODO(ches)
            LOG_FATAL("TODO");
        }

        template <class... Args>
        void emplace_back(Args &&...args)
        {
            if (allocated_count == stored_count)
            {
                reserve(allocated_count == 0 ? 4 : 2 * allocated_count);
            }

            alloc.construct(ptr + stored_count, std::forward<Args>(args)...);
            ++stored_count;
        }

        iterator erase(const_iterator pos)
        {
            // TODO(ches)
            LOG_FATAL("TODO");
        }

        iterator erase(const_iterator first, const_iterator last)
        {
            // TODO(ches)
            LOG_FATAL("TODO");
        }

        void push_back(const T& value)
        {
            if (allocated_count == stored_count)
            {
                reserve(allocated_count == 0 ? 4 : 2 * allocated_count);
            }

            alloc.construct(ptr + stored_count, value);
            ++stored_count;
        }

        void push_back(T&& value)
        {
            if (allocated_count == stored_count)
            {
                reserve(allocated_count == 0 ? 4 : 2 * allocated_count);
            }

            alloc.construct(ptr + stored_count, std::move(value));
            ++stored_count;
        }

        void pop_back()
        {
            DCHECK(!empty());
            alloc.destroy(ptr + stored_count - 1);
            --stored_count;
        }

        void resize(size_type n)
        {
            if (n < size())
            {
                for (size_t i = n; i < size(); ++i)
                {
                    alloc.destroy(ptr + i);
                }
                if (n == 0)
                {
                    alloc.deallocate_object(ptr, allocated_count);
                    ptr = nullptr;
                    allocated_count = 0;
                }
            }
            else if (n > size())
            {
                reserve(n);
                for (size_t i = stored_count; i < n; ++i)
                {
                    alloc.construct(ptr + i);
                }
            }
            stored_count = n;
        }

        void resize(size_type count, const value_type& value)
        {
            // TODO(ches)
            LOG_FATAL("TODO");
        }

        void swap(vector& other)
        {
            LOG_ASSERT(alloc == other.alloc);// TODO(ches): handle this
            std::swap(ptr, other.ptr);
            std::swap(allocated_count, other.allocated_count);
            std::swap(stored_count, other.stored_count);
        }

    private:
        Allocator alloc;
        T* ptr = nullptr;
        size_t allocated_count = 0;
        size_t stored_count = 0;
    };

    template <typename... Ts>
    struct tuple;
    
    template <>
    struct tuple<>
    {
        template <size_t>
        using type = void;
    };

    template <typename T, typename... Ts>
    struct tuple<T, Ts...> : tuple<Ts...>
    {
        using Base = tuple<Ts...>;

        tuple() = default;
        tuple(const tuple&) = default;
        tuple(tuple&&) = default;
        tuple& operator=(tuple&&) = default;
        tuple& operator=(const tuple&) = default;

        tuple(const T& value, const Ts &...values)
            : Base(values...)
            , value(value)
        {}

        tuple(T&& value, Ts &&...values)
            : Base(std::move(values)...)
            , value(std::move(value))
        {}

        T value;
    };

    template <typename... Ts>
    tuple(Ts &&...) -> tuple<std::decay_t<Ts>...>;

    template <size_t I, typename T, typename... Ts>
    LOQUAT_CPU_GPU
    auto& get(tuple<T, Ts...>& t)
    {
        if constexpr (I == 0)
        {
            return t.value;
        }
        else
        {
            return get<I - 1>(static_cast<tuple<Ts...> &>(t));
        }
    }

    template <size_t I, typename T, typename... Ts>
    LOQUAT_CPU_GPU
    const auto& get(const tuple<T, Ts...>& t)
    {
        if constexpr (I == 0)
        {
            return t.value;
        }
        else
        {
            return get<I - 1>(static_cast<const tuple<Ts...>&>(t));
        }
    }

    template <typename Req, typename T, typename... Ts>
    LOQUAT_CPU_GPU
    auto& get(tuple<T, Ts...>& t)
    {
        if constexpr (std::is_same_v<Req, T>)
        {
            return t.value;
        }
        else
        {
            return get<Req>(static_cast<tuple<Ts...>&>(t));
        }
    }

    template <typename Req, typename T, typename... Ts>
    LOQUAT_CPU_GPU
    const auto& get(const tuple<T, Ts...>& t)
    {
        if constexpr (std::is_same_v<Req, T>)
        {
            return t.value;
        }
        else
        {
            return get<Req>(static_cast<const tuple<Ts...>&>(t));
        }
    }

    template <typename T>
    struct complex
    {
        LOQUAT_CPU_GPU
        complex(T re)
            : re(re)
            , im(0)
        {}
        LOQUAT_CPU_GPU
        complex(T re, T im)
            : re(re)
            , im(im)
        {}

        LOQUAT_CPU_GPU
        complex operator-() const
        {
            return { -re, -im };
        }

        LOQUAT_CPU_GPU
        complex operator+(complex z) const
        {
            return { re + z.re, im + z.im };
        }

        LOQUAT_CPU_GPU
        complex operator-(complex z) const
        {
            return { re - z.re, im - z.im };
        }

        LOQUAT_CPU_GPU
        complex operator*(complex z) const
        {
            return { re * z.re - im * z.im, re * z.im + im * z.re };
        }

        LOQUAT_CPU_GPU
        complex operator/(complex z) const
        {
            T scale = 1 / (z.re * z.re + z.im * z.im);
            return {
                scale * (re * z.re + im * z.im),
                scale * (im * z.re - re * z.im)
            };
        }

        LOQUAT_CPU_GPU
        friend complex operator+(T value, complex z)
        {
            return complex(value) + z;
        }

        LOQUAT_CPU_GPU
        friend complex operator-(T value, complex z)
        {
            return complex(value) - z;
        }

        LOQUAT_CPU_GPU
        friend complex operator*(T value, complex z)
        {
            return complex(value) * z;
        }

        LOQUAT_CPU_GPU
        friend complex operator/(T value, complex z)
        {
            return complex(value) / z;
        }

        T re;
        T im;
    };

    LOQUAT_CPU_GPU
    inline float sqrt(float f)
    {
        return ::sqrtf(f);
    }

    LOQUAT_CPU_GPU
    inline double sqrt(double f)
    {
        return ::sqrt(f);
    }

    LOQUAT_CPU_GPU
    inline float abs(float f)
    {
        return ::fabsf(f);
    }

    LOQUAT_CPU_GPU
    inline double abs(double f)
    {
        return ::fabs(f);
    }

    LOQUAT_CPU_GPU
    inline float copysign(float mag, float sign)
    {
#ifdef LOQUAT_IS_GPU_CODE
        return ::copysignf(mag, sign);
#else
        return std::copysign(mag, sign);
#endif
    }

    LOQUAT_CPU_GPU
    inline double copysign(double mag, double sign)
    {
#ifdef LOQUAT_IS_GPU_CODE
        return ::copysign(mag, sign);
#else
        return std::copysign(mag, sign);
#endif
    }

    LOQUAT_CPU_GPU
    inline float floor(float arg)
    {
#ifdef LOQUAT_IS_GPU_CODE
        return ::floorf(arg);
#else
        return std::floor(arg);
#endif
    }

    LOQUAT_CPU_GPU
    inline double floor(double arg)
    {
#ifdef LOQUAT_IS_GPU_CODE
        return ::floor(arg);
#else
        return std::floor(arg);
#endif
    }

    LOQUAT_CPU_GPU
    inline float ceil(float arg)
    {
#ifdef LOQUAT_IS_GPU_CODE
        return ::ceilf(arg);
#else
        return std::ceil(arg);
#endif
    }

    LOQUAT_CPU_GPU
    inline double ceil(double arg)
    {
#ifdef LOQUAT_IS_GPU_CODE
        return ::ceil(arg);
#else
        return std::ceil(arg);
#endif
    }

    LOQUAT_CPU_GPU
    inline float round(float arg)
    {
#ifdef LOQUAT_IS_GPU_CODE
        return ::roundf(arg);
#else
        return std::round(arg);
#endif
    }

    LOQUAT_CPU_GPU
    inline double round(double arg)
    {
#ifdef LOQUAT_IS_GPU_CODE
        return ::round(arg);
#else
        return std::round(arg);
#endif
    }

    LOQUAT_CPU_GPU
    inline float fmod(float x, float y)
    {
#ifdef LOQUAT_IS_GPU_CODE
        return ::fmodf(x, y);
#else
        return std::fmod(x, y);
#endif
    }

    LOQUAT_CPU_GPU
    inline double fmod(double x, double y)
    {
#ifdef LOQUAT_IS_GPU_CODE
        return ::fmod(x, y);
#else
        return std::fmod(x, y);
#endif
    }

    template <typename T>
    LOQUAT_CPU_GPU
    T real(const complex<T>& z)
    {
        return z.re;
    }

    template <typename T>
    LOQUAT_CPU_GPU
    T imag(const complex<T>& z)
    {
        return z.im;
    }

    template <typename T>
    LOQUAT_CPU_GPU
    T norm(const complex<T>& z)
    {
        return z.re * z.re + z.im * z.im;
    }

    template <typename T>
    LOQUAT_CPU_GPU
    T abs(const complex<T>& z)
    {
        return pstd::sqrt(pstd::norm(z));
    }

    template <typename T>
    LOQUAT_CPU_GPU
    complex<T> sqrt(const complex<T>& z)
    {
        T n = pstd::abs(z);
        T t1 = pstd::sqrt(T(.5) * (n + pstd::abs(z.re)));
        T t2 = T(.5) * z.im / t1;

        if (n == 0)
        {
            return 0;
        }

        if (z.re >= 0)
        {
            return { t1, t2 };
        }
        else
        {
            return { pstd::abs(t2), pstd::copysign(t1, z.im) };
        }
    }
}
