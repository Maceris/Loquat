// pbrt is Copyright(c) 1998-2020 Matt Pharr, Wenzel Jakob, and Greg Humphreys.
// The pbrt source code is licensed under the Apache License, Version 2.0.
// SPDX: Apache-2.0

// This file has been modified from the original, original notice is above.

#pragma once

#include <optix.h>

#include "main/loquat.h"

#include "pbr/math/vector_math.h"
#include "pbr/util/color.h"

namespace loquat
{

    class Denoiser
    {
    public:
        Denoiser(Vec2i resolution, bool have_albedo_and_normal);

        // All pointers should be to GPU memory.
        // |n| and |albedo| should be nullptr iff \have_albedo_and_normal| is false.
        void denoise(RGB* rgb, Normal3f* n, RGB* albedo, RGB* result);

    private:
        Vec2i resolution;
        bool have_albedo_and_normal;
        OptixDenoiser denoiser_handle;
        OptixDenoiserSizes memory_sizes;
        void* denoiserState;
        void* scratch_buffer;
        void* intensity;
    };

}