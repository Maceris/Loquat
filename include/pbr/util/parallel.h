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
}