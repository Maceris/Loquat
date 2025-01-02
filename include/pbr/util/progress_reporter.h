// pbrt is Copyright(c) 1998-2020 Matt Pharr, Wenzel Jakob, and Greg Humphreys.
// The pbrt source code is licensed under the Apache License, Version 2.0.
// SPDX: Apache-2.0

// This file has been modified from the original, original notice is above.

#pragma once

//TODO(ches) fill this out

#include <atomic>
#include <chrono>
#include <cstdint>
#include <string>
#include <thread>

#ifdef LOQUAT_BUILD_GPU_RENDERER
#include <cuda_runtime.h>
#include <vector>
#endif

#include "main/loquat.h"

#include "pbr/util/pstd.h"

namespace loquat
{
    class Timer
    {
    public:
        Timer()
        {
            start = clock::now();
        }

        double elapsed_seconds() const
        {
            clock::time_point now = clock::now();
            int64_t elapsed_uS =
                std::chrono::duration_cast<std::chrono::microseconds>(now - start).count();
            return elapsed_uS / 1'000'000.0;
        }

        std::string to_string() const;

    private:
        using clock = std::chrono::steady_clock;
        clock::time_point start;
    };

    class ProgressReporter
    {
    public:
        ProgressReporter()
            : quiet(true)
        {}
        ProgressReporter(int64_t total_work, std::string title, bool quiet,
            bool gpu = false);

        ~ProgressReporter();

        void update(int64_t num = 1);
        void done();
        double elapsed_seconds() const;

        std::string to_string() const;

    private:
        void print_bar();

        int64_t total_work;
        std::string title;
        bool quiet;
        Timer timer;
        std::atomic<int64_t> work_done;
        std::atomic<bool> exit_thread;
        std::thread update_thread;
        pstd::optional<float> finish_time;

#ifdef LOQUAT_BUILD_GPU_RENDERER
        std::vector<cudaEvent_t> gpu_events;
        std::atomic<size_t> gpu_events_launched_offset;
        int gpu_events_finished_offset;
#endif
    };

    inline double ProgressReporter::elapsed_seconds() const
    {
        return finish_time ? *finish_time : timer.elapsed_seconds();
    }

    inline void ProgressReporter::update(int64_t num)
    {
#ifdef LOQUAT_BUILD_GPU_RENDERER
        if (gpu_events.size() > 0)
        {
            if (gpu_events_launched_offset + num <= gpu_events.size())
            {
                while (num-- > 0)
                {
                    LOG_ASSERT(cudaEventRecord(gpu_events[gpu_events_launched_offset]) == cudaSuccess);
                    ++gpu_events_launched_offset;
                }
            }
            return;
        }
#endif
        if (num == 0 || quiet)
        {
            return;
        }
        work_done += num;
    }

}