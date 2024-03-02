// pbrt is Copyright(c) 1998-2020 Matt Pharr, Wenzel Jakob, and Greg Humphreys.
// The pbrt source code is licensed under the Apache License, Version 2.0.
// SPDX: Apache-2.0

// This file has been modified from the original, original notice is above.

#pragma once

#include "main/loquat.h"

namespace loquat
{
    static constexpr int SOBOL_DIMENSIONS = 1024;
    static constexpr int SOBOL_MATRIX_SIZE = 52;

    extern const uint32_t sobol_matrices_32[SOBOL_DIMENSIONS
        * SOBOL_MATRIX_SIZE];

    extern const uint64_t transformed_sobol_matrices[][SOBOL_MATRIX_SIZE];
    extern const uint64_t
        inverse_transformed_sobol_matrices[][SOBOL_MATRIX_SIZE];

}
