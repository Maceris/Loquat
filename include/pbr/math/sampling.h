// pbrt is Copyright(c) 1998-2020 Matt Pharr, Wenzel Jakob, and Greg Humphreys.
// The pbrt source code is licensed under the Apache License, Version 2.0.
// SPDX: Apache-2.0

// This file has been modified from the original, original notice is above.

#pragma once

#include "main/loquat.h"

#include "pbr/math/vector_math.h"

namespace loquat
{
	inline Vec3f sample_uniform_sphere(Point2f sample_2D) {
        Float z = 1 - 2 * sample_2D[0];
        Float r = safe_square_root(1 - square(z));
        Float phi = 2 * PI * sample_2D[1];
        return { r * std::cos(phi), r * std::sin(phi), z };
    }
}