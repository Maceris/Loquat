// pbrt is Copyright(c) 1998-2020 Matt Pharr, Wenzel Jakob, and Greg Humphreys.
// The pbrt source code is licensed under the Apache License, Version 2.0.
// SPDX: Apache-2.0

// This file has been modified from the original, original notice is above.

#pragma once

#include "main/loquat.h"

#include <atomic>
#include <chrono>
#include <concepts>
#include <condition_variable>
#include <cstdint>
#include <functional>
#include <future>
#include <initializer_list>
#include <mutex>
#include <shared_mutex>
#include <string>
#include <thread>
#include <type_traits>
#include <vector>

namespace loquat
{

    void parallel_init(int thread_count = -1);
    void parallel_cleanup();

    int available_cores();
    int running_threads();

    template <typename T>
    class ThreadLocal
    {
    public:
        ThreadLocal()
            : hashTable{ 4 * running_threads() }
            , create{ []() { return T(); } }
        {}

        ThreadLocal(std::function<T(void)>&& create)
            : hashTable{ 4 * running_threads() }
            , create{ create }
        {}

        T& get();

        template <typename F>
        void for_all(F&& func);

    private:
        struct Entry
        {
            std::thread::id t_id;
            T value;
        };
        std::shared_mutex mutex;
        std::vector<std::optional<Entry>> hash_table;
        std::function<T(void)> create;
    };

    class AtomicDouble
    {
    public:
        LOQUAT_CPU_GPU
        explicit AtomicDouble(double v = 0)
        {
#if (defined(__CUDA_ARCH__) && __CUDA_ARCH__ >= 600)
            value = v;
#else
            bits = float_to_bits(v);
#endif
        }

        LOQUAT_CPU_GPU
        operator double() const
        {
#if (defined(__CUDA_ARCH__) && __CUDA_ARCH__ >= 600)
            return value;
#else
            return bits_to_float(bits);
#endif
        }

        LOQUAT_CPU_GPU
        double operator=(double v)
        {
#if (defined(__CUDA_ARCH__) && __CUDA_ARCH__ >= 600)
            value = v;
            return value;
#else
            bits = float_to_bits(v);
            return v;
#endif
        }

        LOQUAT_CPU_GPU
        void add(double v)
        {
#if (defined(__CUDA_ARCH__) && __CUDA_ARCH__ >= 600)
            atomicAdd(&value, v);
#elif defined(__CUDA_ARCH__)
            uint64_t old = bits;
            uint64_t assumed;

            do
            {
                assumed = old;
                old = atomicCAS((unsigned long long int*) & bits, assumed,
                    __double_as_longlong(v + __longlong_as_double(assumed)));
            } while (assumed != old);
#else
            uint64_t oldBits = bits, newBits;
            do
            {
                newBits = float_to_bits(bits_to_float(oldBits) + v);
            } while (!bits.compare_exchange_weak(oldBits, newBits));
#endif
        }

        [[nodiscard]]
        std::string to_string() const;

    private:
#if (defined(__CUDA_ARCH__) && __CUDA_ARCH__ >= 600)
        double value;
#elif defined(__CUDA_ARCH__)
        uint64_t bits;
#else
        std::atomic<uint64_t> bits;
#endif
    };
}
//TODO(ches) finish this