// pbrt is Copyright(c) 1998-2020 Matt Pharr, Wenzel Jakob, and Greg Humphreys.
// The pbrt source code is licensed under the Apache License, Version 2.0.
// SPDX: Apache-2.0

// This file has been modified from the original, original notice is above.

#pragma once

#include <atomic>
#include <cstddef>
#include <list>
#include <memory>
#include <mutex>
#include <type_traits>
#include <utility>

#include "main/loquat.h"

#include "debug/logger.h"
#include "pbr/math/math.h"
#include "pbr/util/pstd.h"

namespace loquat
{
    size_t get_current_RSS();

    class TrackedMemoryResource : public pstd::pmr::memory_resource
    {
    public:
        TrackedMemoryResource(
            pstd::pmr::memory_resource* source = pstd::pmr::get_default_resource())
            : source(source)
        {}

        void* do_allocate(size_t size, size_t alignment)
        {
            void* ptr = source->allocate(size, alignment);
            uint64_t currentBytes = allocatted_bytes.fetch_add(size) + size;
            uint64_t prevMax = maximum_allocated_bytes.load(std::memory_order_relaxed);
            while (prevMax < currentBytes &&
                !maximum_allocated_bytes.compare_exchange_weak(prevMax, currentBytes))
                ;
            return ptr;
        }

        void do_deallocate(void* p, size_t bytes, size_t alignment)
        {
            source->deallocate(p, bytes, alignment);
            allocatted_bytes -= bytes;
        }

        bool do_is_equal(const memory_resource& other) const noexcept
        {
            return this == &other;
        }

        size_t current_allocated_bytes() const { return allocatted_bytes.load(); }
        size_t max_allocated_bytes() const { return maximum_allocated_bytes.load(); }

    private:
        pstd::pmr::memory_resource* source;
        std::atomic<uint64_t> allocatted_bytes{ 0 };
        std::atomic<uint64_t> maximum_allocated_bytes{ 0 };
    };

    template <typename T>
    struct AllocationTraits
    {
        using SingleObject = T*;
    };

    template <typename T>
    struct AllocationTraits<T[]>
    {
        using Array = T*;
    };

    template <typename T, size_t n>
    struct AllocationTraits<T[n]>
    {
        struct Invalid {};
    };

    class alignas(LOQUAT_L1_CACHE_LINE_SIZE) ScratchBuffer
    {
    public:
        ScratchBuffer(int size = 256)
            : allocSize(size)
        {
            ptr = (char*)Allocator().allocate_bytes(size, align);
        }

        ScratchBuffer(const ScratchBuffer&) = delete;

        ScratchBuffer(ScratchBuffer&& b)
        {
            ptr = b.ptr;
            allocSize = b.allocSize;
            offset = b.offset;
            smallBuffers = std::move(b.smallBuffers);

            b.ptr = nullptr;
            b.allocSize = b.offset = 0;
        }

        ~ScratchBuffer()
        {
            reset();
            Allocator().deallocate_bytes(ptr, allocSize, align);
        }

        ScratchBuffer& operator=(const ScratchBuffer&) = delete;

        ScratchBuffer& operator=(ScratchBuffer&& b)
        {
            std::swap(b.ptr, ptr);
            std::swap(b.allocSize, allocSize);
            std::swap(b.offset, offset);
            std::swap(b.smallBuffers, smallBuffers);
            return *this;
        }

        void* alloc(size_t size, size_t align)
        {
            if ((offset % align) != 0)
            {
                offset += align - (offset % align);
            }
            if (offset + size > allocSize)
            {
                realloc(size);
            }
            void* p = ptr + offset;
            offset += size;
            return p;
        }

        template <typename T, typename... Args>
        typename AllocationTraits<T>::SingleObject alloc(Args &&...args)
        {
            T* p = (T*)alloc(sizeof(T), alignof(T));
            return new (p) T(std::forward<Args>(args)...);
        }

        template <typename T>
        typename AllocationTraits<T>::Array alloc(size_t n = 1) {
            using ElementType = typename std::remove_extent_t<T>;
            ElementType* ret =
                (ElementType*)alloc(n * sizeof(ElementType), alignof(ElementType));
            for (size_t i = 0; i < n; ++i)
            {
                new (&ret[i]) ElementType();
            }
            return ret;
        }

        void reset()
        {
            for (const auto& buf : smallBuffers)
            {
                Allocator().deallocate_bytes(buf.first, buf.second, align);
            }
            smallBuffers.clear();
            offset = 0;
        }

    private:
        void realloc(size_t minSize)
        {
            smallBuffers.push_back(std::make_pair(ptr, allocSize));
            allocSize = std::max(2 * minSize, allocSize + minSize);
            ptr = (char*)Allocator().allocate_bytes(allocSize, align);
            offset = 0;
        }

        static constexpr int align = LOQUAT_L1_CACHE_LINE_SIZE;
        char* ptr = nullptr;
        int allocSize = 0;
        int offset = 0;
        std::list<std::pair<char*, size_t>> smallBuffers;
    };

}