// pbrt is Copyright(c) 1998-2020 Matt Pharr, Wenzel Jakob, and Greg Humphreys.
// The pbrt source code is licensed under the Apache License, Version 2.0.
// SPDX: Apache-2.0

// This file has been modified from the original, original notice is above.

#pragma once

#include <atomic>
#include <cstddef>
#include <memory>
#include <mutex>
#include <type_traits>
#include <unordered_map>

#include "main/loquat.h"

#include "pbr/math/math.h"
#include "pbr/util/pstd.h"

namespace loquat
{

#ifdef LOQUAT_BUILD_GPU_RENDERER

    class CUDAMemoryResource : public pstd::pmr::memory_resource
    {
        void* do_allocate(size_t size, size_t alignment);
        void do_deallocate(void* p, size_t bytes, size_t alignment);

        bool do_is_equal(const memory_resource& other) const noexcept
        {
            return this == &other;
        }
    };

    class CUDATrackedMemoryResource : public CUDAMemoryResource
    {
    public:
        void* do_allocate(size_t size, size_t alignment);
        void do_deallocate(void* p, size_t bytes, size_t alignment);

        bool do_is_equal(const memory_resource& other) const noexcept
        {
            return this == &other;
        }

        void prefetch_to_GPU() const;
        size_t get_bytes_allocated() const
        {
            return bytes_allocated;
        }

        static CUDATrackedMemoryResource singleton;

    private:
        mutable std::mutex mutex;
        std::atomic<size_t> bytes_allocated{};
        std::unordered_map<void*, size_t> allocations;
    };

#endif

}