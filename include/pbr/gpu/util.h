// pbrt is Copyright(c) 1998-2020 Matt Pharr, Wenzel Jakob, and Greg Humphreys.
// The pbrt source code is licensed under the Apache License, Version 2.0.
// SPDX: Apache-2.0

// This file has been modified from the original, original notice is above.

#pragma once

#include <format>
#include <map>
#include <typeindex>
#include <typeinfo>
#include <utility>
#include <vector>

#include <cuda.h>
#include <cuda_runtime_api.h>

#ifdef NVTX
#ifdef UNICODE
#undef UNICODE
#endif
#include <nvtx3/nvToolsExt.h>

#ifdef RGB
#undef RGB
#endif  // RGB
#endif

#include "main/loquat.h"

#include "debug/logger.h"
#include "pbr/util/parallel.h"
#include "pbr/util/progress_reporter.h"

#define CUDA_CHECK(EXPR)                                                     \
    if (EXPR != cudaSuccess) {                                               \
        cudaError_t error = cudaGetLastError();                              \
        LOG_FATAL(std::format("CUDA error: {}", cudaGetErrorString(error))); \
    } else /* eat semicolon */

#define CU_CHECK(EXPR)                                              \
    do {                                                            \
        CUresult result = EXPR;                                     \
        if (result != CUDA_SUCCESS) {                               \
            const char *str;                                        \
            CHECK_EQ(CUDA_SUCCESS, cuGetErrorString(result, &str)); \
            LOG_FATAL(std::format("CUDA error: %s", str));          \
        }                                                           \
    } while (false) /* eat semicolon */

namespace loquat
{

    std::pair<cudaEvent_t, cudaEvent_t> get_profiler_events(const char* description);

    template <typename F>
    inline int get_block_size(const char* description, F kernel)
    {
        // Note: this isn't reentrant, but that's fine for our purposes...
        static std::map<std::type_index, int> kernelBlockSizes;

        std::type_index index = std::type_index(typeid(F));

        auto iter = kernelBlockSizes.find(index);
        if (iter != kernelBlockSizes.end())
        {
            return iter->second;
        }

        int minGridSize, blockSize;
        CUDA_CHECK(
            cudaOccupancyMaxPotentialBlockSize(&minGridSize, &blockSize, kernel, 0, 0));
        kernelBlockSizes[index] = blockSize;
        LOG_INFO(std::format("[{}]: block size {}", description, blockSize));

        return blockSize;
    }

#ifdef __NVCC__
    template <typename F>
    __global__ void kernel(F func, int nItems) {
        int tid = blockIdx.x * blockDim.x + threadIdx.x;
        if (tid >= nItems)
        {
            return;
        }

        func(tid);
    }

    // GPU Launch Function Declarations
    template <typename F>
    void GPU_parallel_for(const char* description, int nItems, F func);

    template <typename F>
    void GPU_parallel_for(const char* description, int nItems, F func)
    {
#ifdef NVTX
        nvtxRangePush(description);
#endif
        auto kernel = &kernel<F>;

        int blockSize = get_block_size(description, kernel);
        std::pair<cudaEvent_t, cudaEvent_t> events = get_profiler_events(description);

#ifdef _DEBUG
        LOG_INFO(std::format("Launching {}", description));
#endif
        cudaEventRecord(events.first);
        int gridSize = (nItems + blockSize - 1) / blockSize;
        kernel << <gridSize, blockSize >> > (func, nItems);
        cudaEventRecord(events.second);

#ifdef _DEBUG
        CUDA_CHECK(cudaDeviceSynchronize());
        LOG_INFO(std::format("Post-sync {}", description));
#endif
#ifdef NVTX
        nvtxRangePop();
#endif
    }

#endif  // __NVCC__

    // GPU Synchronization Function Declarations
    void GPU_wait();

    void report_kernel_stats();

    void GPU_init();
    void GPU_thread_init();

    void GPU_memset(void* ptr, int byte, size_t bytes);

    void GPU_register_thread(const char* name);
    void GPU_name_stream(cudaStream_t stream, const char* name);
}
