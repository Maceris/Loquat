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
            : hash_table(4 * running_threads() )
            , create{ []() { return T(); } }
        {}

        ThreadLocal(std::function<T(void)>&& create)
            : hash_table( 4 * running_threads() )
            , create{ create }
        {}

        T& get();

        template <typename F>
        void for_all(F&& func);

    private:
        struct Entry
        {
            std::thread::id tid;
            T value;
        };
        std::shared_mutex mutex;
        std::vector<std::optional<Entry>> hash_table;
        std::function<T(void)> create;
    };

    template <typename T>
    inline T& ThreadLocal<T>::get()
    {
        std::thread::id tid = std::this_thread::get_id();
        uint32_t hash = std::hash<std::thread::id>()(tid);
        hash %= hash_table.size();
        int step = 1;
        int tries = 0;

        mutex.lock_shared();
        while (true) {
            LOG_ASSERT(++tries < hash_table.size());  // full hash table

            if (hash_table[hash] && hash_table[hash]->tid == tid)
            {
                // Found it
                T& threadLocal = hash_table[hash]->value;
                mutex.unlock_shared();
                return threadLocal;
            }
            else if (!hash_table[hash])
            {
                mutex.unlock_shared();

                // get reader-writer lock before calling the callback so that the user
                // doesn't have to worry about writing a thread-safe callback.
                mutex.lock();
                T newItem = create();

                if (hash_table[hash])
                {
                    // someone else got there first--keep looking, but now
                    // with a writer lock.
                    while (true)
                    {
                        hash += step;
                        ++step;
                        if (hash >= hash_table.size())
                            hash %= hash_table.size();

                        if (!hash_table[hash])
                            break;
                    }
                }

                hash_table[hash] = Entry{ tid, std::move(newItem) };
                T& threadLocal = hash_table[hash]->value;
                mutex.unlock();
                return threadLocal;
            }

            hash += step;
            ++step;
            if (hash >= hash_table.size())
            {
                hash %= hash_table.size();
            }
        }
    }

    template <typename T>
    template <typename F>
    inline void ThreadLocal<T>::for_all(F&& func)
    {
        mutex.lock();
        for (auto& entry : hash_table)
        {
            if (entry)
            {
                func(entry->value);
            }
        }
        mutex.unlock();
    }

    class AtomicFloat
    {
    public:
        LOQUAT_CPU_GPU
        explicit AtomicFloat(float v = 0)
        {
#ifdef LOQUAT_IS_GPU_CODE
            value = v;
#else
            bits = float_to_bits(v);
#endif
        }

        LOQUAT_CPU_GPU
        operator float() const
        {
#ifdef LOQUAT_IS_GPU_CODE
            return value;
#else
            return bits_to_float(bits);
#endif
        }
        LOQUAT_CPU_GPU
        Float operator=(float v)
        {
#ifdef LOQUAT_IS_GPU_CODE
            value = v;
            return value;
#else
            bits = float_to_bits(v);
            return v;
#endif
        }

        LOQUAT_CPU_GPU
        void add(float v)
        {
#ifdef LOQUAT_IS_GPU_CODE
            atomicAdd(&value, v);
#else
            FloatBits oldBits = bits, newBits;
            do {
                newBits = float_to_bits(bits_to_float(oldBits) + v);
            } while (!bits.compare_exchange_weak(oldBits, newBits));
#endif
        }

        std::string to_string() const;

    private:
#ifdef LOQUAT_IS_GPU_CODE
        float value;
#else
        std::atomic<FloatBits> bits;
#endif
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

    class Barrier
    {
    public:
        explicit Barrier(int n)
            : num_to_block(n)
            , num_to_exit(n) 
        {}

        Barrier(const Barrier&) = delete;
        Barrier& operator=(const Barrier&) = delete;

        // All block. Returns true to only one thread (which should delete the
        // barrier).
        bool Block();

    private:
        std::mutex mutex;
        std::condition_variable cv;
        int num_to_block;
        int num_to_exit;
    };

    void parallel_for(int64_t start, int64_t end, std::function<void(int64_t, int64_t)> func);
    void parallel_for_2D(const AABB2i& extent, std::function<void(AABB2i)> func);

    inline void parallel_for(int64_t start, int64_t end, std::function<void(int64_t)> func)
    {
        parallel_for(start, end, [&func](int64_t start, int64_t end) {
            for (int64_t i = start; i < end; ++i)
                func(i);
            });
    }

    inline void parallel_for_2D(const AABB2i& extent, std::function<void(Point2i)> func)
    {
        parallel_for_2D(extent, [&func](AABB2i b) {
            for (Point2i p : b)
                func(p);
            });
    }

    class ThreadPool;

    class ParallelJob
    {
    public:
        virtual ~ParallelJob() { LOG_ASSERT(removed); }

        virtual bool have_work() const = 0;
        virtual void run_step(std::unique_lock<std::mutex>* lock) = 0;

        bool finished() const
        {
            return !have_work() && activeWorkers == 0;
        }

        virtual std::string to_string() const = 0;

        static ThreadPool* thread_pool;

    protected:
        std::string base_to_string() const
        {
            return std::format("activeWorkers: {} removed: {}", activeWorkers, 
                removed);
        }

    private:
        friend class ThreadPool;
        int activeWorkers = 0;
        ParallelJob* prev = nullptr;
        ParallelJob* next = nullptr;
        bool removed = false;
    };

    class ThreadPool
    {
    public:
        explicit ThreadPool(int nThreads);

        ~ThreadPool();

        size_t size() const { return threads.size(); }

        std::unique_lock<std::mutex> add_to_job_list(ParallelJob* job);
        void remove_from_job_list(ParallelJob* job);

        void work_or_wait(std::unique_lock<std::mutex>* lock, bool isEnqueuingThread);
        bool work_or_return();

        void disable();
        void reenable();

        void for_each_thread(std::function<void(void)> func);

        std::string to_string() const;

    private:
        void worker();

        std::vector<std::thread> threads;
        mutable std::mutex mutex;
        bool shutdown_threads = false;
        bool disabled = false;
        ParallelJob* jobList = nullptr;
        std::condition_variable job_list_condition;
    };


    bool do_parallel_work();

    template <typename T>
    class AsyncJob : public ParallelJob
    {
    public:
        AsyncJob(std::function<T(void)> w)
            : func(std::move(w))
        {}

        bool have_work() const { return !started; }

        void run_step(std::unique_lock<std::mutex>* lock)
        {
            thread_pool->remove_from_job_list(this);
            started = true;
            lock->unlock();
            // Execute asynchronous work and notify waiting threads of its completion
            T r = func();
            std::unique_lock<std::mutex> ul(mutex);
            result = r;
            cv.notify_all();
        }

        bool is_ready() const {
            std::lock_guard<std::mutex> lock(mutex);
            return result.has_value();
        }

        T get_result() {
            wait();
            std::lock_guard<std::mutex> lock(mutex);
            return *result;
        }

        pstd::optional<T> try_get_result(std::mutex* external_mutex)
        {
            {
                std::lock_guard<std::mutex> lock(mutex);
                if (result)
                    return result;
            }

            external_mutex->unlock();
            do_parallel_work();
            external_mutex->lock();
            return {};
        }

        void wait()
        {
            while (!is_ready() && do_parallel_work())
            {
                // Do nothing
            }
            std::unique_lock<std::mutex> lock(mutex);
            if (!result.has_value())
            {
                cv.wait(lock, [this]() { return result.has_value(); });
            }
        }

        void do_work()
        {
            T r = func();
            std::unique_lock<std::mutex> l(mutex);
            LOG_ASSERT(!result.has_value());
            result = r;
            cv.notify_all();
        }

        std::string to_string() const
        {
            return std::format("[ AsyncJob started: {} ]", started);
        }

    private:
        std::function<T(void)> func;
        bool started = false;
        pstd::optional<T> result;
        mutable std::mutex mutex;
        std::condition_variable cv;
    };

    void for_each_thread(std::function<void(void)> func);

    void disable_thread_pool();
    void reenable_thread_pool();

    template <typename F, typename... Args>
    inline auto run_async(F func, Args &&...args)
    {
        // Create _AsyncJob_ for _func_ and _args_
        auto fvoid = std::bind(func, std::forward<Args>(args)...);
        using R = typename std::invoke_result_t<F, Args...>;
        AsyncJob<R>* job = new AsyncJob<R>(std::move(fvoid));

        // Enqueue _job_ or run it immediately
        std::unique_lock<std::mutex> lock;
        if (running_threads() == 1)
        {
            job->do_work();
        }
        else
        {
            lock = ParallelJob::thread_pool->add_to_job_list(job);
        }

        return job;
    }

}
