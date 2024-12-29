// pbrt is Copyright(c) 1998-2020 Matt Pharr, Wenzel Jakob, and Greg Humphreys.
// The pbrt source code is licensed under the Apache License, Version 2.0.
// SPDX: Apache-2.0

// This file has been modified from the original, original notice is above.

#pragma once

#include "main/loquat.h"

#include "pbr/util/pstd.h"

namespace loquat
{
    TriangleMesh* loop_subdivide(const Transform* render_from_object,
        bool reverse_orientation, int level_count,
        pstd::span<const int> vertex_indices,
        pstd::span<const Point3f> p, Allocator allocator);
}