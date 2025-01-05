// pbrt is Copyright(c) 1998-2020 Matt Pharr, Wenzel Jakob, and Greg Humphreys.
// The pbrt source code is licensed under the Apache License, Version 2.0.
// SPDX: Apache-2.0

// This file has been modified from the original, original notice is above.

#pragma once

#include "main/loquat.h"

#include "pbr/math/math.h"
#include "pbr/util/pstd.h"
#include "pbr/math/vector_math.h"

namespace loquat
{
    template <typename P>
    LOQUAT_CPU_GPU
    inline P blossom_cubic_bezier(pstd::span<const P> p, Float u0, Float u1,
        Float u2)
    {
        P a[3] = { lerp(u0, p[0], p[1]), lerp(u0, p[1], p[2]), lerp(u0, p[2], p[3]) };
        P b[2] = { lerp(u1, a[0], a[1]), lerp(u1, a[1], a[2]) };
        return lerp(u2, b[0], b[1]);
    }

    template <typename P>
    LOQUAT_CPU_GPU
    inline P evaluate_cubic_bezier(pstd::span<const P> cp, Float u)
    {
        return blossom_cubic_bezier(cp, u, u, u);
    }

    LOQUAT_CPU_GPU
    inline Point3f evaluate_cubic_bezier(pstd::span<const Point3f> cp, Float u,
        Vec3f* deriv)
    {
        Point3f cp1[3] = {
            lerp(u, cp[0], cp[1]),
            lerp(u, cp[1], cp[2]),
            lerp(u, cp[2], cp[3])
        };
        Point3f cp2[2] = {
            lerp(u, cp1[0], cp1[1]),
            lerp(u, cp1[1], cp1[2]) 
        };
        if (deriv)
        {
            // Compute B\'ezier curve derivative at $u$
            if (length_squared(cp2[1] - cp2[0]) > 0)
                *deriv = 3.0f * (cp2[1] - cp2[0]);
            else
                *deriv = cp[3] - cp[0];
        }
        return lerp(u, cp2[0], cp2[1]);
    }

    LOQUAT_CPU_GPU
    inline pstd::array<Point3f, 7> subdivide_cubic_bezier(
        pstd::span<const Point3f> cp)
    {
        return { cp[0],
                (cp[0] + cp[1]) / 2.0f,
                (cp[0] + 2.0f * cp[1] + cp[2]) / 4.0f,
                (cp[0] + 3.0f * cp[1] + 3.0f * cp[2] + cp[3]) / 8.0f,
                (cp[1] + 2.0f * cp[2] + cp[3]) / 4.0f,
                (cp[2] + cp[3]) / 2.0f,
                cp[3] };
    }

    LOQUAT_CPU_GPU
    inline pstd::array<Point3f, 4> cubic_bezier_control_points(
        pstd::span<const Point3f> cp, Float uMin, Float uMax)
    {
        return { blossom_cubic_bezier(cp, uMin, uMin, uMin),
                blossom_cubic_bezier(cp, uMin, uMin, uMax),
                blossom_cubic_bezier(cp, uMin, uMax, uMax),
                blossom_cubic_bezier(cp, uMax, uMax, uMax) };
    }

    LOQUAT_CPU_GPU
    inline AABB3f bound_cubic_bezier(pstd::span<const Point3f> cp)
    {
        return union_bounds(AABB3f(cp[0], cp[1]), AABB3f(cp[2], cp[3]));
    }

    LOQUAT_CPU_GPU
    inline AABB3f bound_cubic_bezier(pstd::span<const Point3f> cp, Float uMin,
        Float uMax)
    {
        if (uMin == 0 && uMax == 1)
            return bound_cubic_bezier(cp);
        auto cpSeg = cubic_bezier_control_points(cp, uMin, uMax);
        return bound_cubic_bezier(pstd::span<const Point3f>(cpSeg));
    }

    LOQUAT_CPU_GPU
    inline pstd::array<Point3f, 4> elevate_quadratic_bezier_to_cubic(
        pstd::span<const Point3f> cp)
    {
        return { cp[0], lerp(2.f / 3.f, cp[0], cp[1]), lerp(1.f / 3.f, cp[1], cp[2]), cp[2] };
    }

    LOQUAT_CPU_GPU
    inline pstd::array<Point3f, 3> quadratic_bspline_to_bezier(
        pstd::span<const Point3f> cp)
    {
        // We can compute equivalent Bezier control points via some blossoming.
        // We have three control points and a uniform knot vector; we will label
        // the points p01, p12, and p23.  We want the Bezier control points of
        // the equivalent curve, which are p11, p12, and p22.  We already have
        // p12.
        Point3f p11 = lerp(0.5, cp[0], cp[1]);
        Point3f p22 = lerp(0.5, cp[1], cp[2]);
        return { p11, cp[1], p22 };
    }

    LOQUAT_CPU_GPU
    inline pstd::array<Point3f, 4> cubic_bspline_to_bezier(
        pstd::span<const Point3f> cp)
    {
        // Blossom from p012, p123, p234, and p345 to the Bezier control points
        // p222, p223, p233, and p333.
        // https://people.eecs.berkeley.edu/~sequin/CS284/IMGS/cubicbsplinepoints.gif
        Point3f p012 = cp[0], p123 = cp[1], p234 = cp[2], p345 = cp[3];

        Point3f p122 = lerp(2.f / 3.f, p012, p123);
        Point3f p223 = lerp(1.f / 3.f, p123, p234);
        Point3f p233 = lerp(2.f / 3.f, p123, p234);
        Point3f p334 = lerp(1.f / 3.f, p234, p345);

        Point3f p222 = lerp(0.5f, p122, p223);
        Point3f p333 = lerp(0.5f, p233, p334);

        return { p222, p223, p233, p333 };
    }

}