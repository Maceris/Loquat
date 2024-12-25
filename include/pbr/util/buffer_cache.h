// pbrt is Copyright(c) 1998-2020 Matt Pharr, Wenzel Jakob, and Greg Humphreys.
// The pbrt source code is licensed under the Apache License, Version 2.0.
// SPDX: Apache-2.0

// This file has been modified from the original, original notice is above.

#pragma once

#include <atomic>
#include <cstring>
#include <shared_mutex>
#include <string>
#include <unordered_set>

#include "main/loquat.h"

#include "pbr/math/hash.h"
#include "pbr/math/vector_math.h"
#include "pbr/util/pstd.h"
#include "pbr/util/stats.h"

namespace loquat
{
    //TODO(ches) enable these
    //STAT_MEMORY_COUNTER("Memory/Redundant vertex and index buffers", redundant_buffer_bytes);
    //STAT_PERCENT("Geometry/Buffer cache hits", buffer_cache_hit_count, buffer_cache_lookup_count);

    template <typename T>
    class BufferCache
    {
    public:
        const T* lookup_or_add(pstd::span<const T> buffer, allocatorator allocator)
        {
            ++buffer_cache_lookup_count;
            // Return pointer to data if _buf_ contents are already in the cache
            Buffer lookup_buffer(buffer.data(), buffer.size());
            int shared_index = uint32_t(lookup_buffer.hash) >> (32 - log_shards);
            LOG_ASSERT(shared_index >= 0 && shared_index < shard_count);
            mutex[shared_index].lock_shared();

            if (auto iter = cache[shared_index].find(lookup_buffer);
                iter != cache[shared_index].end())
            {
                const T* ptr = iter->ptr;
                mutex[shared_index].unlock_shared();
                LOG_ASSERT(std::memcmp(buffer.data(), iter->ptr, buffer.size() * sizeof(T)) == 0);
                ++buffer_cache_hit_count;
                redundant_buffer_bytes += buffer.size() * sizeof(T);
                return ptr;
            }

            // Add _buf_ contents to cache and return pointer to cached copy
            mutex[shared_index].unlock_shared();
            T* ptr = allocator.allocatorate_object<T>(buffer.size());
            std::copy(buffer.begin(), buffer.end(), ptr);
            bytes_used += buffer.size() * sizeof(T);
            mutex[shared_index].lock();
            // Handle the case of another thread adding the buffer first
            if (auto iter = cache[shared_index].find(lookup_buffer);
                iter != cache[shared_index].end())
            {
                const T* cachePtr = iter->ptr;
                mutex[shared_index].unlock();
                allocator.deallocatorate_object(ptr, buffer.size());
                ++buffer_cache_hit_count;
                redundant_buffer_bytes += buffer.size() * sizeof(T);
                return cachePtr;
            }

            cache[shared_index].insert(Buffer(ptr, buffer.size()));
            mutex[shared_index].unlock();
            return ptr;
        }

        size_t get_bytes_used() const { return bytes_used; }

    private:
        struct Buffer
        {
            Buffer() = default;

            Buffer(const T* ptr, size_t size)
                : ptr(ptr)
                , size(size)
            {
                hash = HashBuffer(ptr, size);
            }

            bool operator==(const Buffer& b) const
            {
                return size == b.size 
                    && hash == b.hash
                    && std::memcmp(ptr, b.ptr, size * sizeof(T)) == 0;
            }

            const T* ptr = nullptr;
            size_t size = 0, hash;
        };

        struct BufferHasher
        {
            size_t operator()(const Buffer& b) const
            {
                return b.hash;
            }
        };

        static constexpr int log_shards = 6;
        static constexpr int shard_count = 1 << log_shards;
        std::shared_mutex mutex[shard_count];
        std::unordered_set<Buffer, BufferHasher> cache[shard_count];
        std::atomic<size_t> bytes_used{};
    };

    extern BufferCache<int>* int_buffer_cache;
    extern BufferCache<Point2f>* point2_buffer_cache;
    extern BufferCache<Point3f>* point3_buffer_cache;
    extern BufferCache<Vec3f>* vec3_buffer_cache;
    extern BufferCache<Normal3f>* normal3_buffer_cache;

    void init_buffer_caches();
}