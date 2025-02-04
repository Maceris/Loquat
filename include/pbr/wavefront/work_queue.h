// pbrt is Copyright(c) 1998-2020 Matt Pharr, Wenzel Jakob, and Greg Humphreys.
// The pbrt source code is licensed under the Apache License, Version 2.0.
// SPDX: Apache-2.0

// This file has been modified from the original, original notice is above.

#pragma once

#include <atomic>
#include <utility>

#include "main/loquat.h"

#include "pbr/options.h"
#ifdef LOQUAT_BUILD_GPU_RENDERER
#include "pbr/gpu/util.h"
#endif  // LOQUAT_BUILD_GPU_RENDERER
#include "pbr/util/parallel.h"
#include "pbr/util/pstd.h"

#ifdef __CUDACC__
	#ifdef LOQUAT_IS_WINDOWS
		#if (__CUDA_ARCH__ < 700)
			#define LOQUAT_USE_LEGACY_CUDA_ATOMICS
		#endif
	#else
		#if (__CUDA_ARCH__ < 600)
			#define LOQUAT_USE_LEGACY_CUDA_ATOMICS
		#endif
	#endif  // LOQUAT_IS_WINDOWS

	#ifndef LOQUAT_USE_LEGACY_CUDA_ATOMICS
		#include <cuda/atomic>
	#endif
#endif  // __CUDACC__

namespace loquat
{
    template <typename WorkItem>
    class WorkQueue : public SOA<WorkItem>
    {
    public:
        WorkQueue() = default;
        WorkQueue(int n, Allocator alloc)
            : SOA<WorkItem>(n, alloc)
        {}

        WorkQueue& operator=(const WorkQueue& w)
        {
            SOA<WorkItem>::operator=(w);
#if defined(LOQUAT_IS_GPU_CODE) && defined(LOQUAT_USE_LEGACY_CUDA_ATOMICS)
            size = w.size;
#else
            size.store(w.size.load());
#endif
            return *this;
        }

        LOQUAT_CPU_GPU
        int get_size() const
        {
#ifdef LOQUAT_IS_GPU_CODE
#ifdef LOQUAT_USE_LEGACY_CUDA_ATOMICS
            return size;
#else
            return size.load(cuda::std::memory_order_relaxed);
#endif
#else
            return size.load(std::memory_order_relaxed);
#endif
        }
        LOQUAT_CPU_GPU
        void reset()
        {
#ifdef LOQUAT_IS_GPU_CODE
#ifdef LOQUAT_USE_LEGACY_CUDA_ATOMICS
            size = 0;
#else
            size.store(0, cuda::std::memory_order_relaxed);
#endif
#else
            size.store(0, std::memory_order_relaxed);
#endif
        }

        LOQUAT_CPU_GPU
        int push(WorkItem w)
        {
            int index = allocate_entry();
            (*this)[index] = w;
            return index;
        }

    protected:
        LOQUAT_CPU_GPU
        int allocate_entry()
        {
#ifdef LOQUAT_IS_GPU_CODE
#ifdef LOQUAT_USE_LEGACY_CUDA_ATOMICS
            return atomicAdd(&size, 1);
#else
            return size.fetch_add(1, cuda::std::memory_order_relaxed);
#endif
#else
            return size.fetch_add(1, std::memory_order_relaxed);
#endif
        }

    private:
#ifdef LOQUAT_IS_GPU_CODE
#ifdef LOQUAT_USE_LEGACY_CUDA_ATOMICS
        int size = 0;
#else
        cuda::atomic<int, cuda::thread_scope_device> size{ 0 };
#endif
#else
        std::atomic<int> size{ 0 };
#endif  // LOQUAT_IS_GPU_CODE
    };

    template <typename F, typename WorkItem>
    void for_all_queued(const char* desc, const WorkQueue<WorkItem>* q,
        int max_queued, F&& func)
    {
        if (options->use_GPU) {
            // Launch GPU threads to process _q_ using _func_
#ifdef LOQUAT_BUILD_GPU_RENDERER
            GPUParallelFor(desc, max_queued, [=] LOQUAT_GPU(int index) mutable {
                if (index >= q->get_size())
                    return;
                func((*q)[index]);
            });
#else
            LOG_FATAL("options->use_GPU was set without LOQUAT_BUILD_GPU_RENDERER enabled");
#endif

        }
        else {
            // Process _q_ using _func_ with CPU threads
            parallel_for(0, q->get_size(), [&](int index) { func((*q)[index]); });
        }
    }

    template <typename T>
    class MultiWorkQueue;

    template <typename... Ts>
    class MultiWorkQueue<TypePack<Ts...>>
    {
    public:
        template <typename T>
        LOQUAT_CPU_GPU
        WorkQueue<T>* get()
        {
            return &pstd::get<WorkQueue<T>>(queues);
        }

        MultiWorkQueue(int n, Allocator alloc, pstd::span<const bool> haveType)
        {
            int index = 0;
            ((*get<Ts>() = WorkQueue<Ts>(haveType[index++] ? n : 1, alloc)), ...);
        }

        template <typename T>
        LOQUAT_CPU_GPU
        int get_size() const
        {
            return get<T>()->get_size();
        }
        template <typename T>
        LOQUAT_CPU_GPU
        int push(const T& value)
        {
            return get<T>()->push(value);
        }

        LOQUAT_CPU_GPU
        void reset()
        {
            (get<Ts>()->reset(), ...);
        }

    private:
        pstd::tuple<WorkQueue<Ts>...> queues;
    };
}