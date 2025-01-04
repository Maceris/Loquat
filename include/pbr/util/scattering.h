// pbrt is Copyright(c) 1998-2020 Matt Pharr, Wenzel Jakob, and Greg Humphreys.
// The pbrt source code is licensed under the Apache License, Version 2.0.
// SPDX: Apache-2.0

// This file has been modified from the original, original notice is above.

#pragma once

#include <algorithm>
#include <cmath>
#include <format>
#include <functional>
#include <ostream>
#include <string>
#include <type_traits>
#include <vector>

#include "main/loquat.h"

#include "pbr/math/low_discrepancy.h"
#include "pbr/math/math.h"
#include "pbr/math/rng.h"
#include "pbr/math/vector_math.h"
#include "pbr/struct/containers.h"
#include "pbr/util/memory.h"
#include "pbr/util/pstd.h"

namespace loquat
{
    LOQUAT_CPU_GPU
    inline int sample_discrete(pstd::span<const Float> weights, Float u,
        Float* pmf = nullptr, Float* uRemapped = nullptr);

    LOQUAT_CPU_GPU
    inline Float sample_linear(Float u, Float a, Float b);
    LOQUAT_CPU_GPU
    inline Float invert_linear_sample(Float x, Float a, Float b);

    LOQUAT_CPU_GPU
    pstd::array<Float, 3> sample_spherical_triangle(
        const pstd::array<Point3f, 3>& v, Point3f p, Point2f u,
        Float* pdf = nullptr);

    LOQUAT_CPU_GPU
    Point2f invert_spherical_triangle_sample(const pstd::array<Point3f, 3>& v,
        Point3f p, Vec3f w);

    LOQUAT_CPU_GPU
    Point3f sample_spherical_rectangle(Point3f p, Point3f v00, Vec3f eu,
        Vec3f ev, Point2f u, Float* pdf = nullptr);

    LOQUAT_CPU_GPU
    Point2f invert_spherical_rectangle_sample(Point3f pRef, Point3f v00,
        Vec3f eu, Vec3f ev, Point3f pRect);

    LOQUAT_CPU_GPU
    Vec3f sample_henyey_greenstein(Vec3f wo, Float g, Point2f u,
        Float* pdf = nullptr);

    LOQUAT_CPU_GPU
    Float sample_catmull_rom(pstd::span<const Float> nodes, pstd::span<const Float> f,
        pstd::span<const Float> cdf, Float sample, Float* fval = nullptr,
        Float* pdf = nullptr);

    LOQUAT_CPU_GPU
    Float sample_catmull_rom_2D(pstd::span<const Float> nodes1,
        pstd::span<const Float> nodes2,
        pstd::span<const Float> values, pstd::span<const Float> cdf,
        Float alpha, Float sample, Float* fval = nullptr,
        Float* pdf = nullptr);

    LOQUAT_CPU_GPU
    inline Float balance_heuristic(int nf, Float fPdf, int ng,
        Float gPdf)
    {
        return (nf * fPdf) / (nf * fPdf + ng * gPdf);
    }
    
    LOQUAT_CPU_GPU
    inline Float power_heuristic(int nf, Float fPdf, int ng, Float gPdf)
    {
        Float f = nf * fPdf, g = ng * gPdf;
        if (is_inf(square(f)))
        {
            return 1;
        }
        return square(f) / (square(f) + square(g));
    }

    LOQUAT_CPU_GPU
    inline int sample_discrete(pstd::span<const Float> weights,
        Float u, Float* pmf, Float* uRemapped)
    {
        // Handle empty _weights_ for discrete sampling
        if (weights.empty())
        {
            if (pmf)
            {
                *pmf = 0;
            }
            return -1;
        }

        // Compute sum of _weights_
        Float sumWeights = 0;
        for (Float w : weights)
        {
            sumWeights += w;
        }

        // Compute rescaled $u'$ sample
        Float up = u * sumWeights;
        if (up == sumWeights)
        {
            up = next_float_down(up);
        }

        // Find offset in _weights_ corresponding to $u'$
        int offset = 0;
        Float sum = 0;
        while (sum + weights[offset] <= up)
        {
            sum += weights[offset++];

            LOG_ASSERT(offset < weights.size());
        }

        // Compute PMF and remapped _u_ value, if necessary
        if (pmf)
        {
            *pmf = weights[offset] / sumWeights;
        }
        if (uRemapped)
        {
            *uRemapped = std::min((up - sum) / weights[offset], 
                ONE_MINUS_EPSILON);
        }

        return offset;
    }

    LOQUAT_CPU_GPU
    inline Float linear_PDF(Float x, Float a, Float b)
    {
        LOG_ASSERT(a >= 0 && b >= 0);
        if (x < 0 || x > 1)
            return 0;
        return 2 * lerp(x, a, b) / (a + b);
    }

    LOQUAT_CPU_GPU
    inline Float sample_linear(Float u, Float a, Float b)
    {
        LOG_ASSERT(a >= 0 && b >= 0);
        if (u == 0 && a == 0)
            return 0;
        Float x = u * (a + b) / (a + std::sqrt(lerp(u, square(a), square(b))));
        return std::min(x, ONE_MINUS_EPSILON);
    }

    LOQUAT_CPU_GPU
    inline Float invert_linear_sample(Float x, Float a, Float b)
    {
        return x * (a * (2 - x) + b * x) / (a + b);
    }

    LOQUAT_CPU_GPU
    inline Float bilinear_PDF(Point2f p,
        pstd::span<const Float> w)
    {
        LOG_ASSERT(4 == w.size());
        if (p.x < 0 || p.x > 1 || p.y < 0 || p.y > 1)
        {
            return 0;
        }
        if (w[0] + w[1] + w[2] + w[3] == 0)
        {
            return 1;
        }
        return 4 *
            ((1 - p[0]) * (1 - p[1]) * w[0] 
                + p[0] * (1 - p[1]) * w[1] 
                + (1 - p[0]) * p[1] * w[2] 
                + p[0] * p[1] * w[3]
                ) 
            / (w[0] + w[1] + w[2] + w[3]);
    }

    LOQUAT_CPU_GPU
    inline Point2f sample_bilinear(Point2f u,
        pstd::span<const Float> w)
    {
        LOG_ASSERT(4 == w.size());
        Point2f p;
        // sample $y$ for bilinear marginal distribution
        p.y = sample_linear(u[1], w[0] + w[1], w[2] + w[3]);

        // sample $x$ for bilinear conditional distribution
        p.x = sample_linear(u[0], lerp(p.y, w[0], w[2]), lerp(p.y, w[1], w[3]));

        return p;
    }

    LOQUAT_CPU_GPU
    inline Point2f invert_bilinear_sample(Point2f p,
        pstd::span<const Float> w)
    {
        return {invert_linear_sample(p.x, lerp(p.y, w[0], w[2]), 
                lerp(p.y, w[1], w[3])),
            invert_linear_sample(p.y, w[0] + w[1], w[2] + w[3]) 
        };
    }

    LOQUAT_CPU_GPU
    inline Float visible_wavelengths_PDF(Float lambda)
    {
        if (lambda < 360 || lambda > 830)
        {
            return 0;
        }
        return 0.0039398042f / square(std::cosh(0.0072f * (lambda - 538)));
    }

    LOQUAT_CPU_GPU
    inline Float sample_visible_wavelengths(Float u)
    {
        return 538 - 138.888889f * std::atanh(0.85691062f - 1.82750197f * u);
    }

    LOQUAT_CPU_GPU
    inline pstd::array<Float, 3> sample_uniform_triangle(Point2f u)
    {
        Float b0, b1;
        if (u[0] < u[1])
        {
            b0 = u[0] / 2;
            b1 = u[1] - b0;
        }
        else
        {
            b1 = u[1] / 2;
            b0 = u[0] - b1;
        }
        return { b0, b1, 1 - b0 - b1 };
    }

    LOQUAT_CPU_GPU
    inline Point2f invert_uniform_triangle_sample(const pstd::array<Float, 3>& b)
    {
        if (b[0] > b[1])
        {
            // b0 = u[0] - u[1] / 2, b1 = u[1] / 2
            return { b[0] + b[1], 2 * b[1] };
        }
        else
        {
            // b1 = u[1] - u[0] / 2, b0 = u[0] / 2
            return { 2 * b[0], b[1] + b[0] };
        }
    }

    LOQUAT_CPU_GPU
    inline Float sample_tent(Float u, Float r)
    {
        if (sample_discrete({ 0.5f, 0.5f }, u, nullptr, &u) == 0)
            return -r + r * sample_linear(u, 0, 1);
        else
            return r * sample_linear(u, 1, 0);
    }

    LOQUAT_CPU_GPU
    inline Float tent_PDF(Float x, Float r)
    {
        if (std::abs(x) >= r)
            return 0;
        return 1 / r - std::abs(x) / square(r);
    }

    LOQUAT_CPU_GPU
    inline Float invert_tent_sample(Float x, Float r)
    {
        if (x <= 0)
        {
            return (1 - invert_linear_sample(-x / r, 1, 0)) / 2;
        }
        else
        {
            return 0.5f + invert_linear_sample(x / r, 1, 0) / 2;
        }
    }

    LOQUAT_CPU_GPU
    inline Float exponential_PDF(Float x, Float a)
    {
        LOG_ASSERT(a > 0);
        return a * std::exp(-a * x);
    }

    LOQUAT_CPU_GPU
    inline Float sample_exponential(Float u, Float a)
    {
        LOG_ASSERT(a > 0);
        return -std::log(1 - u) / a;
    }

    LOQUAT_CPU_GPU
    inline Float invert_exponential_sample(Float x, Float a)
    {
        LOG_ASSERT(a > 0);
        return 1 - std::exp(-a * x);
    }

    LOQUAT_CPU_GPU
    inline Float normal_PDF(Float x, Float mu = 0, Float sigma = 1)
    {
        return gaussian(x, mu, sigma);
    }

    LOQUAT_CPU_GPU
    inline Float sample_normal(Float u, Float mu = 0, Float sigma = 1)
    {
        return mu + SQRT2 * sigma * error_function_inverse(2 * u - 1);
    }

    LOQUAT_CPU_GPU
        inline Float invert_normal_sample(Float x, Float mu = 0, Float sigma = 1)
    {
        return 0.5f * (1 + std::erf((x - mu) / (sigma * SQRT2)));
    }

    LOQUAT_CPU_GPU
    inline Point2f sample_two_normal(Point2f u, Float mu = 0, Float sigma = 1)
    {
        Float r2 = -2 * std::log(1 - u[0]);
        return { mu + sigma * std::sqrt(r2 * std::cos(2 * PI * u[1])),
                mu + sigma * std::sqrt(r2 * std::sin(2 * PI * u[1])) };
    }

    LOQUAT_CPU_GPU
    inline Float logistic_PDF(Float x, Float s)
    {
        x = std::abs(x);
        return std::exp(-x / s) / (s * square(1 + std::exp(-x / s)));
    }

    LOQUAT_CPU_GPU
    inline Float sample_logistic(Float u, Float s)
    {
        return -s * std::log(1 / u - 1);
    }

    LOQUAT_CPU_GPU
    inline Float invert_logistic_sample(Float x, Float s)
    {
        return 1 / (1 + std::exp(-x / s));
    }

    LOQUAT_CPU_GPU
    inline Float trimmed_logistic_PDF(Float x, Float s, Float a, Float b)
    {
        if (x < a || x > b)
        {
            return 0;
        }
        auto P = [&](Float x) { return invert_logistic_sample(x, s); };
        return logistic(x, s) / (P(b) - P(a));
    }

    LOQUAT_CPU_GPU
    inline Float sample_trimmed_logistic(Float u, Float s, Float a, Float b)
    {
        LOG_ASSERT(a < b);
        auto P = [&](Float x) { return invert_logistic_sample(x, s); };
        u = lerp(u, P(a), P(b));
        Float x = sample_logistic(u, s);
        LOG_ASSERT(!is_NaN(x));
        return clamp(x, a, b);
    }

    LOQUAT_CPU_GPU
    inline Float invert_trimmed_logistic_sample(Float x, Float s, Float a,
        Float b)
    {
        LOG_ASSERT(a <= x && x <= b);
        auto P = [&](Float x) { return invert_logistic_sample(x, s); };
        return (P(x) - P(a)) / (P(b) - P(a));
    }

    LOQUAT_CPU_GPU
    inline Float smooth_step_PDF(Float x, Float a, Float b)
    {
        if (x < a || x > b)
        {
            return 0;
        }
        LOG_ASSERT(a < b);
        return (2 / (b - a)) * smooth_step(x, a, b);
    }

    LOQUAT_CPU_GPU
    inline Float sample_smooth_step(Float u, Float a, Float b)
    {
        LOG_ASSERT(a < b);
        auto cdfMinusU = [=](Float x) -> std::pair<Float, Float> {
            Float t = (x - a) / (b - a);
            Float P = 2 * pow<3>(t) - pow<4>(t);
            Float PDeriv = smooth_step_PDF(x, a, b);
            return { P - u, PDeriv };
            };
        return newton_bisection(a, b, cdfMinusU);
    }

    LOQUAT_CPU_GPU
    inline Float invert_smooth_step_sample(Float x, Float a, Float b)
    {
        Float t = (x - a) / (b - a);
        auto P = [&](Float x) { return 2 * pow<3>(t) - pow<4>(t); };
        return (P(x) - P(a)) / (P(b) - P(a));
    }

    LOQUAT_CPU_GPU
    inline Point2f sample_uniform_disk_polar(Point2f u)
    {
        Float r = std::sqrt(u[0]);
        Float theta = 2 * PI * u[1];
        return { r * std::cos(theta), r * std::sin(theta) };
    }

    LOQUAT_CPU_GPU
    inline Point2f invert_uniform_disk_polar_sample(Point2f p)
    {
        Float phi = std::atan2(p.y, p.x);
        if (phi < 0)
        {
            phi += 2 * PI;
        }
        return Point2f(square(p.x) + square(p.y), phi / (2 * PI));
    }

    LOQUAT_CPU_GPU
    inline Point2f sample_uniform_disk_concentric(Point2f u)
    {
        // Map _u_ to $[-1,1]^2$ and handle degeneracy at the origin
        Point2f u_offset = 2.0f * u - Vec2f(1, 1);
        if (u_offset.x == 0 && u_offset.y == 0)
        {
            return { 0, 0 };
        }

        // Apply concentric mapping to point
        Float theta, r;
        if (std::abs(u_offset.x) > std::abs(u_offset.y))
        {
            r = u_offset.x;
            theta = PI_OVER_4 * (u_offset.y / u_offset.x);
        }
        else
        {
            r = u_offset.y;
            theta = PI_OVER_2 - PI_OVER_4 * (u_offset.x / u_offset.y);
        }
        return r * Point2f(std::cos(theta), std::sin(theta));
    }

    LOQUAT_CPU_GPU
        inline Point2f invert_uniform_disk_concentric_sample(Point2f p)
    {
        Float theta = std::atan2(p.y, p.x);  // -pi -> pi
        Float r = std::sqrt(square(p.x) + square(p.y));

        Point2f uo;
        // TODO(ches): can we make this less branchy?
        if (std::abs(theta) < PI_OVER_4 || std::abs(theta) > 3 * PI_OVER_4)
        {
            uo.x = r = pstd::copysign(r, p.x);
            if (p.x < 0)
            {
                if (p.y < 0)
                {
                    uo.y = (PI + theta) * r / PI_OVER_4;
                }
                else {
                    uo.y = (theta - PI) * r / PI_OVER_4;
                }
            }
            else
            {
                uo.y = (theta * r) / PI_OVER_4;
            }
        }
        else
        {
            uo.y = r = pstd::copysign(r, p.y);
            if (p.y < 0)
            {
                uo.x = -(PI_OVER_2 + theta) * r / PI_OVER_4;
            }
            else
            {
                uo.x = (PI_OVER_2 - theta) * r / PI_OVER_4;
            }
        }

        return { (uo.x + 1) / 2, (uo.y + 1) / 2 };
    }

    LOQUAT_CPU_GPU
    inline Vec3f sample_uniform_hemisphere(Point2f u)
    {
        Float z = u[0];
        Float r = safe_square_root(1 - square(z));
        Float phi = 2 * PI * u[1];
        return { r * std::cos(phi), r * std::sin(phi), z };
    }

    LOQUAT_CPU_GPU
    inline Float uniform_hemisphere_PDF()
    {
        return INV_2PI;
    }

    LOQUAT_CPU_GPU
    inline Point2f invert_uniform_hemisphere_sample(Vec3f w)
    {
        Float phi = std::atan2(w.y, w.x);
        if (phi < 0)
        {
            phi += 2 * PI;
        }
        return Point2f(w.z, phi / (2 * PI));
    }

    LOQUAT_CPU_GPU
    inline Vec3f sample_uniform_sphere(Point2f u)
    {
        Float z = 1 - 2 * u[0];
        Float r = safe_square_root(1 - square(z));
        Float phi = 2 * PI * u[1];
        return { r * std::cos(phi), r * std::sin(phi), z };
    }

    LOQUAT_CPU_GPU
    inline Float uniform_sphere_PDF()
    {
        return INV_4PI;
    }

    LOQUAT_CPU_GPU
    inline Point2f invert_uniform_sphere_sample(Vec3f w)
    {
        Float phi = std::atan2(w.y, w.x);
        if (phi < 0)
        {
            phi += 2 * PI;
        }
        return Point2f((1 - w.z) / 2, phi / (2 * PI));
    }

    LOQUAT_CPU_GPU
    inline Vec3f sample_cosine_hemisphere(Point2f u)
    {
        Point2f d = sample_uniform_disk_concentric(u);
        Float z = safe_square_root(1 - square(d.x) - square(d.y));
        return Vec3f(d.x, d.y, z);
    }

    LOQUAT_CPU_GPU
    inline Float cosine_hemisphere_PDF(Float cosTheta)
    {
        return cosTheta * INV_PI;
    }

    LOQUAT_CPU_GPU
    inline Point2f invert_cosine_hemisphere_sample(Vec3f w)
    {
        return invert_uniform_disk_concentric_sample({ w.x, w.y });
    }

    LOQUAT_CPU_GPU
    inline Float uniform_cone_PDF(Float cos_theta_max)
    {
        return 1 / (2 * PI * (1 - cos_theta_max));
    }

    LOQUAT_CPU_GPU
    inline Vec3f sample_uniform_cone(Point2f u, Float cos_theta_max)
    {
        Float cosTheta = (1 - u[0]) + u[0] * cos_theta_max;
        Float sinTheta = safe_square_root(1 - square(cosTheta));
        Float phi = u[1] * 2 * PI;
        return spherical_direction(sinTheta, cosTheta, phi);
    }

    LOQUAT_CPU_GPU
    inline Point2f invert_uniform_cone_sample(Vec3f w, Float cos_theta_max)
    {
        Float cosTheta = w.z;
        Float phi = spherical_phi(w);
        return { (cosTheta - 1) / (cos_theta_max - 1), phi / (2 * PI) };
    }

    // sample from e^(-c x), x from 0 to x_max
    LOQUAT_CPU_GPU
    inline Float sample_trimmed_exponential(Float u, Float c, Float x_max)
    {
        return std::log(1 - u * (1 - std::exp(-c * x_max))) / -c;
    }

    LOQUAT_CPU_GPU
    inline Float trimmed_exponential_PDF(Float x, Float c, Float x_max)
    {
        if (x < 0 || x > x_max)
        {
            return 0;
        }
        return c / (1 - std::exp(-c * x_max)) * std::exp(-c * x);
    }

    LOQUAT_CPU_GPU
    inline Float invert_trimmed_exponential_sample(Float x, Float c,
        Float x_max)
    {
        LOG_ASSERT(x >= 0 && x <= x_max);
        return (1 - std::exp(-c * x)) / (1 - std::exp(-c * x_max));
    }

    LOQUAT_CPU_GPU
    inline Vec3f sample_uniform_hemisphere_concentric(Point2f u)
    {
        // Map uniform random numbers to $[-1,1]^2$
        Point2f u_offset = 2.f * u - Vec2f(1, 1);

        // Handle degeneracy at the origin
        if (u_offset.x == 0 && u_offset.y == 0)
        {
            return Vec3f(0, 0, 1);
        }

        // Apply concentric mapping to point
        Float theta, r;
        if (std::abs(u_offset.x) > std::abs(u_offset.y))
        {
            r = u_offset.x;
            theta = PI_OVER_4 * (u_offset.y / u_offset.x);
        }
        else
        {
            r = u_offset.y;
            theta = PI_OVER_2 - PI_OVER_4 * (u_offset.x / u_offset.y);
        }

        return Vec3f(std::cos(theta) * r * std::sqrt(2 - r * r),
            std::sin(theta) * r * std::sqrt(2 - r * r), 1 - r * r);
    }

    template <typename Float = Float>
    class VarianceEstimator
    {
    public:
        LOQUAT_CPU_GPU
        void add(Float x)
        {
            ++n;
            Float delta = x - mean;
            mean += delta / n;
            Float delta2 = x - mean;
            S += delta * delta2;
        }

        LOQUAT_CPU_GPU
        Float mean() const
        {
            return mean;
        }

        LOQUAT_CPU_GPU
        Float variance() const
        {
            return (n > 1) ? S / (n - 1) : 0;
        }

        LOQUAT_CPU_GPU
        int64_t count() const
        {
            return n;
        }

        LOQUAT_CPU_GPU
        Float relative_variance() const
        {
            return (n < 1 || mean == 0) ? 0 : variance() / mean();
        }

        LOQUAT_CPU_GPU
        void merge(const VarianceEstimator& ve)
        {
            if (ve.n == 0)
            {
                return;
            }
            S = S + ve.S + square(ve.mean - mean) * n * ve.n / (n + ve.n);
            mean = (n * mean + ve.n * ve.mean) / (n + ve.n);
            n += ve.n;
        }

    private:
        Float mean = 0;
        Float S = 0;
        int64_t n = 0;
    };

    template <typename T>
    class WeightedReservoirSampler
    {
    public:
        WeightedReservoirSampler() = default;
        LOQUAT_CPU_GPU
        WeightedReservoirSampler(uint64_t rngSeed)
            : rng(rngSeed)
        {}

        LOQUAT_CPU_GPU
        void seed(uint64_t seed)
        {
            rng.SetSequence(seed);
        }

        LOQUAT_CPU_GPU
        bool add(const T& sample, Float weight)
        {
            weight_sum += weight;
            // Randomly add _sample_ to reservoir
            Float p = weight / weight_sum;
            if (rng.uniform<Float>() < p)
            {
                reservoir = sample;
                resivoir_weight = weight;
                return true;
            }
            LOG_ASSERT(weight_sum < 1e80);
            return false;
        }

        template <typename F>
        LOQUAT_CPU_GPU
        bool add(F func, Float weight)
        {
            // Process weighted reservoir sample via callback
            weight_sum += weight;
            Float p = weight / weight_sum;
            if (rng.uniform<Float>() < p)
            {
                reservoir = func();
                resivoir_weight = weight;
                return true;
            }
            LOG_ASSERT(weight_sum < 1e80);
            return false;
        }

        LOQUAT_CPU_GPU
        void copy(const WeightedReservoirSampler& wrs)
        {
            weight_sum = wrs.weight_sum;
            reservoir = wrs.reservoir;
            resivoir_weight = wrs.resivoir_weight;
        }

        LOQUAT_CPU_GPU
        int has_sample() const
        {
            return weight_sum > 0;
        }

        LOQUAT_CPU_GPU
        const T& get_sample() const
        {
            return reservoir;
        }

        LOQUAT_CPU_GPU
        Float sample_probability() const
        {
            return resivoir_weight / weight_sum;
        }

        LOQUAT_CPU_GPU
        Float get_weight_sum() const
        {
            return weight_sum;
        }

        LOQUAT_CPU_GPU
        void reset()
        {
            resivoir_weight = weight_sum = 0;
        }

        LOQUAT_CPU_GPU
        void merge(const WeightedReservoirSampler& wrs)
        {
            LOG_ASSERT(weight_sum + wrs.get_weight_sum() <=  1e80);
            if (wrs.has_sample() && add(wrs.reservoir, wrs.weight_sum))
            {
                resivoir_weight = wrs.resivoir_weight;
            }
        }

        std::string to_string() const
        {
            return std::format("[ WeightedReservoirSampler rng: {} "
                "weight_sum: {} reservoir: {} resivoir_weight: {} ]",
                rng, weight_sum, reservoir, resivoir_weight);
        }

    private:
        RNG rng;
        Float weight_sum = 0;
        Float resivoir_weight = 0;
        T reservoir{};
    };

    class PiecewiseConstant1D {
    public:
        LOQUAT_CPU_GPU
        size_t bytes_used() const
        {
            return (func.capacity() + cdf.capacity()) * sizeof(Float);
        }

        static void test_compare_distributions(const PiecewiseConstant1D& da,
            const PiecewiseConstant1D& db, Float eps = 1e-5);

        std::string to_string() const
        {
            return std::format("[ PiecewiseConstant1D func: {} cdf: {} "
                "min: {} max: {} function_integral: {} ]", func, cdf, min, max, function_integral);
        }

        PiecewiseConstant1D() = default;
        PiecewiseConstant1D(Allocator alloc)
            : func(alloc)
            , cdf(alloc)
        {}
        PiecewiseConstant1D(pstd::span<const Float> f, Allocator alloc = {})
            : PiecewiseConstant1D(f, 0., 1., alloc)
        {}

        PiecewiseConstant1D(pstd::span<const Float> f, Float min, Float max,
            Allocator alloc = {})
            : func(f.begin(), f.end(), alloc)
            , cdf(f.size() + 1, alloc)
            , min(min)
            , max(max)
        {
            LOG_ASSERT(max > min);
            // Take absolute value of _func_
            for (Float& f : func)
            {
                f = std::abs(f);
            }

            // Compute integral of step function at $x_i$
            cdf[0] = 0;
            size_t n = f.size();
            for (size_t i = 1; i < n + 1; ++i)
            {
                LOG_ASSERT(func[i - 1] >= 0);
                cdf[i] = cdf[i - 1] + func[i - 1] * (max - min) / n;
            }

            // Transform step function integral into CDF
            function_integral = cdf[n];
            if (function_integral == 0)
            {
                for (size_t i = 1; i < n + 1; ++i)
                {
                    cdf[i] = Float(i) / Float(n);
                }
            }
            else
            {
                for (size_t i = 1; i < n + 1; ++i)
                {
                    cdf[i] /= function_integral;
                }
            }
        }

        LOQUAT_CPU_GPU
        Float integral() const
        {
            return function_integral;
        }
        LOQUAT_CPU_GPU
        size_t size() const
        {
            return func.size();
        }

        LOQUAT_CPU_GPU
        Float sample(Float u, Float* pdf = nullptr, int* offset = nullptr) const
        {
            // Find surrounding CDF segments and _offset_
            int o = find_interval((int)cdf.size(), [&](int index) { return cdf[index] <= u; });
            if (offset)
            {
                *offset = o;
            }

            // Compute offset along CDF segment
            Float du = u - cdf[o];
            if (cdf[o + 1] - cdf[o] > 0)
            {
                du /= cdf[o + 1] - cdf[o];
            }
            LOG_ASSERT(!is_NaN(du));

            // Compute PDF for sampled offset
            if (pdf)
            {
                *pdf = (function_integral > 0) ? func[o] / function_integral : 0;
            }

            // Return $x$ corresponding to sample
            return lerp((o + du) / size(), min, max);
        }

        LOQUAT_CPU_GPU
        pstd::optional<Float> invert(Float x) const
        {
            // Compute offset to CDF values that bracket $x$
            if (x < min || x > max)
            {
                return {};
            }
            Float c = (x - min) / (max - min) * func.size();
            int offset = clamp(int(c), 0, func.size() - 1);
            LOG_ASSERT(offset >= 0 && offset + 1 < cdf.size());

            // Linearly interpolate between adjacent CDF values to find sample value
            Float delta = c - offset;
            return lerp(delta, cdf[offset], cdf[offset + 1]);
        }

        pstd::vector<Float> func;
        pstd::vector<Float> cdf;
        Float min;
        Float max;
        Float function_integral = 0;
    };

    class PiecewiseConstant2D
    {
    public:
        PiecewiseConstant2D() = default;
        PiecewiseConstant2D(Allocator alloc)
            : pConditionalV(alloc)
            , pMarginal(alloc)
        {}
        PiecewiseConstant2D(pstd::span<const Float> data, int nx, int ny,
            Allocator alloc = {})
            : PiecewiseConstant2D(data, nx, ny, AABB2f(Point2f(0, 0), Point2f(1, 1)),
                alloc)
        {}
        explicit PiecewiseConstant2D(const Array2D<Float>& data, Allocator alloc = {})
            : PiecewiseConstant2D(pstd::span<const Float>(data), data.size_x(), data.size_y(),
                alloc)
        {}
        PiecewiseConstant2D(const Array2D<Float>& data, AABB2f domain, Allocator alloc = {})
            : PiecewiseConstant2D(pstd::span<const Float>(data), data.size_x(), data.size_y(),
                domain, alloc)
        {}

        LOQUAT_CPU_GPU
        size_t bytes_used() const
        {
            return pConditionalV.size() *
                (pConditionalV[0].bytes_used() + sizeof(pConditionalV[0])) +
                pMarginal.bytes_used();
        }

        LOQUAT_CPU_GPU
        AABB2f get_domain() const
        {
            return domain;
        }

        LOQUAT_CPU_GPU
        Point2i resolution() const
        {
            return { int(pConditionalV[0].size()), int(pMarginal.size()) };
        }

        std::string to_string() const
        {
            return std::format("[ PiecewiseConstant2D domain: {} "
                "pConditionalV: {} pMarginal: {} ]",
                domain, pConditionalV, pMarginal);
        }
        static void test_compare_distributions(const PiecewiseConstant2D& da,
            const PiecewiseConstant2D& db, Float eps = 1e-5);

        PiecewiseConstant2D(pstd::span<const Float> func, int nu, int nv, AABB2f domain,
            Allocator alloc = {})
            : domain(domain)
            , pConditionalV(alloc)
            , pMarginal(alloc)
        {
            LOG_ASSERT(func.size() == (size_t)nu * (size_t)nv);
            pConditionalV.reserve(nv);
            for (int v = 0; v < nv; ++v)
            {
                // Compute conditional sampling distribution for $\tilde{v}$
                pConditionalV.emplace_back(func.subspan(v * nu, nu), domain.min[0],
                    domain.max[0], alloc);
            }

            // Compute marginal sampling distribution $p[\tilde{v}]$
            pstd::vector<Float> marginalFunc;
            marginalFunc.reserve(nv);
            for (int v = 0; v < nv; ++v)
            {
                marginalFunc.push_back(pConditionalV[v].integral());
            }
            pMarginal =
                PiecewiseConstant1D(marginalFunc, domain.min[1], domain.max[1], alloc);
        }

        LOQUAT_CPU_GPU
        Float integral() const
        {
            return pMarginal.integral();
        }

        LOQUAT_CPU_GPU
        Point2f sample(Point2f u, Float* pdf = nullptr, Point2i* offset = nullptr) const
        {
            Float pdfs[2];
            Point2i uv;
            Float d1 = pMarginal.sample(u[1], &pdfs[1], &uv[1]);
            Float d0 = pConditionalV[uv[1]].sample(u[0], &pdfs[0], &uv[0]);
            if (pdf)
            {
                *pdf = pdfs[0] * pdfs[1];
            }
            if (offset)
            {
                *offset = uv;
            }
            return Point2f(d0, d1);
        }

        LOQUAT_CPU_GPU
        Float PDF(Point2f pr) const
        {
            Point2f p = Point2f(domain.offset(pr));
            int iu =
                clamp(int(p[0] * pConditionalV[0].size()), 0, pConditionalV[0].size() - 1);
            int iv = clamp(int(p[1] * pMarginal.size()), 0, pMarginal.size() - 1);
            return pConditionalV[iv].func[iu] / pMarginal.integral();
        }

        LOQUAT_CPU_GPU
        pstd::optional<Point2f> invert(Point2f p) const
        {
            pstd::optional<Float> mInv = pMarginal.invert(p[1]);
            if (!mInv)
            {
                return {};
            }
            Float p1o = (p[1] - domain.min[1]) / (domain.max[1] - domain.min[1]);
            if (p1o < 0 || p1o > 1)
            {
                return {};
            }
            int offset = clamp(p1o * pConditionalV.size(), 0, pConditionalV.size() - 1);
            pstd::optional<Float> cInv = pConditionalV[offset].invert(p[0]);
            if (!cInv)
            {
                return {};
            }
            return Point2f(*cInv, *mInv);
        }

    private:
        AABB2f domain;
        pstd::vector<PiecewiseConstant1D> pConditionalV;
        PiecewiseConstant1D pMarginal;
    };

    class AliasTable
    {
    public:
        AliasTable() = default;
        AliasTable(Allocator alloc = {})
            : bins(alloc)
        {}
        AliasTable(pstd::span<const Float> weights, Allocator alloc = {});

        LOQUAT_CPU_GPU
        int sample(Float u, Float* pmf = nullptr, Float* uRemapped = nullptr) const;
        std::string to_string() const;

        LOQUAT_CPU_GPU
        size_t size() const
        {
            return bins.size();
        }
        LOQUAT_CPU_GPU
        Float PMF(int index) const
        {
            return bins[index].p;
        }

    private:
        struct Bin
        {
            Float q;
            Float p;
            int alias;
        };
        pstd::vector<Bin> bins;
    };

    class SummedAreaTable
    {
    public:
        SummedAreaTable(Allocator alloc)
            : sum(alloc)
        {}
        SummedAreaTable(const Array2D<Float>& values, Allocator alloc = {})
            : sum(values.size_x(), values.size_y(), alloc)
        {
            sum(0, 0) = values(0, 0);
            // Compute sums along first row and column
            for (int x = 1; x < sum.size_x(); ++x)
            {
                sum(x, 0) = values(x, 0) + sum(x - 1, 0);
            }
            for (int y = 1; y < sum.size_y(); ++y)
            {
                sum(0, y) = values(0, y) + sum(0, y - 1);
            }

            // Compute sums for the remainder of the entries
            for (int y = 1; y < sum.size_y(); ++y)
            {
                for (int x = 1; x < sum.size_x(); ++x)
                {
                    sum(x, y) =
                        (values(x, y) + sum(x - 1, y) + sum(x, y - 1) - sum(x - 1, y - 1));
                }
            }
        }

        LOQUAT_CPU_GPU
        Float integral(AABB2f extent) const
        {
            double s = (((double)lookup(extent.max.x, extent.max.y) -
                (double)lookup(extent.min.x, extent.max.y)) +
                ((double)lookup(extent.min.x, extent.min.y) -
                    (double)lookup(extent.max.x, extent.min.y)));
            return std::max<Float>(s / (sum.size_x() * sum.size_y()), 0);
        }

        std::string to_string() const;

    private:
        LOQUAT_CPU_GPU
        Float lookup(Float x, Float y) const
        {
            // Rescale $(x,y)$ to table resolution and compute integer coordinates
            x *= sum.size_x();
            y *= sum.size_y();
            int x0 = (int)x, y0 = (int)y;

            // Bilinearly interpolate between surrounding table values
            Float v00 = lookup_int(x0, y0), v10 = lookup_int(x0 + 1, y0);
            Float v01 = lookup_int(x0, y0 + 1), v11 = lookup_int(x0 + 1, y0 + 1);
            Float dx = x - int(x), dy = y - int(y);
            return (1 - dx) * (1 - dy) * v00 
                + (1 - dx) * dy * v01 
                + dx * (1 - dy) * v10 
                + dx * dy * v11;
        }

        LOQUAT_CPU_GPU
        Float lookup_int(int x, int y) const
        {
            // Return zero at lower boundaries
            if (x == 0 || y == 0)
            {
                return 0;
            }

            // Reindex $(x,y)$ and return actual stored value
            x = std::min(x - 1, sum.size_x() - 1);
            y = std::min(y - 1, sum.size_y() - 1);
            return sum(x, y);
        }

        Array2D<double> sum;
    };

    class WindowedPiecewiseConstant2D
    {
    public:
        WindowedPiecewiseConstant2D(Allocator alloc)
            : sat(alloc)
            , func(alloc)
        {}
        WindowedPiecewiseConstant2D(Array2D<Float> f, Allocator alloc = {})
            : sat(f, alloc)
            , func(f, alloc)
        {}

        LOQUAT_CPU_GPU
        pstd::optional<Point2f> sample(Point2f u, AABB2f b, Float* pdf) const
        {
            // Handle zero-valued function for windowed sampling
            if (sat.integral(b) == 0)
                return {};

            // Define lambda function _Px_ for marginal cumulative distribution
            Float bInt = sat.integral(b);
            auto Px = [&, this](Float x) -> Float {
                AABB2f bx = b;
                bx.max.x = x;
                return sat.integral(bx) / bInt;
                };

            // sample marginal windowed function in $x$
            Point2f p;
            p.x = sample_bisection(Px, u[0], b.min.x, b.max.x, func.size_x());

            // sample conditional windowed function in $y$
            // Compute 2D bounds _bCond_ for conditional sampling
            int nx = func.size_x();
            AABB2f bCond(Point2f(pstd::floor(p.x * nx) / nx, b.min.y),
                Point2f(pstd::ceil(p.x * nx) / nx, b.max.y));
            if (bCond.min.x == bCond.max.x)
                bCond.max.x += 1.0f / nx;
            if (sat.integral(bCond) == 0)
                return {};

            // Define lambda function for conditional distribution and sample $y$
            Float condIntegral = sat.integral(bCond);
            auto Py = [&, this](Float y) -> Float {
                AABB2f by = bCond;
                by.max.y = y;
                return sat.integral(by) / condIntegral;
                };
            p.y = sample_bisection(Py, u[1], b.min.y, b.max.y, func.size_y());

            // Compute PDF and return point sampled from windowed function
            *pdf = eval(p) / bInt;
            return p;
        }

        LOQUAT_CPU_GPU
        Float PDF(Point2f p, const AABB2f& b) const
        {
            Float function_integral = sat.integral(b);
            if (function_integral == 0)
            {
                return 0;
            }
            return eval(p) / function_integral;
        }

    private:
        template <typename CDF>
        LOQUAT_CPU_GPU
        static Float sample_bisection(CDF P, Float u, Float min, Float max,
            int n)
        {
            // Apply bisection to bracket _u_
            while (pstd::ceil(n * max) - pstd::floor(n * min) > 1)
            {
                LOG_ASSERT(P(min) <=  u);
                LOG_ASSERT(P(max) >= u);
                Float mid = (min + max) / 2;
                if (P(mid) > u)
                    max = mid;
                else
                    min = mid;
            }

            // Find sample by interpolating between _min_ and _max_
            Float t = (u - P(min)) / (P(max) - P(min));
            return clamp(lerp(t, min, max), min, max);
        }

        LOQUAT_CPU_GPU
        Float eval(Point2f p) const
        {
            Point2i pi(std::min<int>(p[0] * func.size_x(), func.size_x() - 1),
                std::min<int>(p[1] * func.size_y(), func.size_y() - 1));
            return func[pi];
        }

        SummedAreaTable sat;
        Array2D<Float> func;
    };

    pstd::vector<Float> sample_1D_function(std::function<Float(Float)> f,
        int step_count, int sample_count, Float min = 0, Float max = 1,
        Allocator alloc = {});

    Array2D<Float> sample_2D_function(std::function<Float(Float, Float)> f,
        int nu, int nv, int sample_count,
        AABB2f domain = { Point2f(0, 0), Point2f(1, 1) },
        Allocator alloc = {});

    namespace detail
    {

        template <typename Iterator>
        class IndexingIterator {
        public:
            template <typename Generator>
            LOQUAT_CPU_GPU
            IndexingIterator(int i, int n, const Generator*)
                : i(i)
                , n(n)
            {}

            LOQUAT_CPU_GPU
            bool operator==(const Iterator& it) const
            {
                return i == it.i;
            }

            LOQUAT_CPU_GPU
            bool operator!=(const Iterator& it) const
            {
                return !(*this == it);
            }

            LOQUAT_CPU_GPU
            Iterator& operator++()
            {
                ++i;
                return (Iterator&)*this;
            }

            LOQUAT_CPU_GPU
            Iterator operator++(int) const
            {
                Iterator it = *this;
                return ++it;
            }

        protected:
            int i;
            int n;
        };

        template <typename Generator, typename Iterator>
        class IndexingGenerator
        {
        public:
            LOQUAT_CPU_GPU
            IndexingGenerator(int n)
                : n(n)
            {}
            LOQUAT_CPU_GPU
            Iterator begin() const
            {
                return Iterator(0, n, (const Generator*)this);
            }
            LOQUAT_CPU_GPU
            Iterator end() const
            {
                return Iterator(n, n, (const Generator*)this);
            }

        protected:
            int n;
        };

        class Uniform1DIter;
        class Uniform2DIter;
        class Uniform3DIter;
        class Hammersley2DIter;
        class Hammersley3DIter;
        class Stratified1DIter;
        class Stratified2DIter;
        class Stratified3DIter;
        template <typename Iterator>
        class RNGIterator;

        template <typename Generator, typename Iterator>
        class RNGGenerator : public IndexingGenerator<Generator, Iterator>
        {
        public:
            LOQUAT_CPU_GPU
            RNGGenerator(int n, uint64_t sequenceIndex = 0,
                uint64_t seed = PCG32_DEFAULT_STATE)
                : IndexingGenerator<Generator, Iterator>(n)
                , sequenceIndex(sequenceIndex)
                , seed(seed)
            {}

        protected:
            friend RNGIterator<Iterator>;
            uint64_t sequenceIndex;
            uint64_t seed;
        };

        template <typename Iterator>
        class RNGIterator : public IndexingIterator<Iterator>
        {
        public:
            template <typename Generator>
            LOQUAT_CPU_GPU
            RNGIterator(int i, int n,
                const RNGGenerator<Generator, Iterator>* generator)
                : IndexingIterator<Iterator>(i, n, generator)
                , rng(generator->sequenceIndex)
            {}

        protected:
            RNG rng;
        };

    }

    class Uniform1D : public detail::RNGGenerator<Uniform1D, detail::Uniform1DIter>
    {
    public:
        using detail::RNGGenerator<Uniform1D, detail::Uniform1DIter>::RNGGenerator;
    };

    class Uniform2D : public detail::RNGGenerator<Uniform2D, detail::Uniform2DIter>
    {
    public:
        using detail::RNGGenerator<Uniform2D, detail::Uniform2DIter>::RNGGenerator;
    };

    class Uniform3D : public detail::RNGGenerator<Uniform3D, detail::Uniform3DIter>
    {
    public:
        using detail::RNGGenerator<Uniform3D, detail::Uniform3DIter>::RNGGenerator;
    };

    class Hammersley2D
        : public detail::IndexingGenerator<Hammersley2D, detail::Hammersley2DIter>
    {
    public:
        using detail::IndexingGenerator<Hammersley2D,
            detail::Hammersley2DIter>::IndexingGenerator;
    };

    class Hammersley3D
        : public detail::IndexingGenerator<Hammersley3D, detail::Hammersley3DIter>
    {
    public:
        using detail::IndexingGenerator<Hammersley3D,
            detail::Hammersley3DIter>::IndexingGenerator;
    };

    class Stratified1D : public detail::RNGGenerator<Stratified1D, detail::Stratified1DIter>
    {
    public:
        using detail::RNGGenerator<Stratified1D, detail::Stratified1DIter>::RNGGenerator;
    };

    class Stratified2D : public detail::RNGGenerator<Stratified2D, detail::Stratified2DIter>
    {
    public:
        LOQUAT_CPU_GPU
            Stratified2D(int nx, int ny, uint64_t sequenceIndex = 0,
                uint64_t seed = PCG32_DEFAULT_STATE)
            : detail::RNGGenerator<Stratified2D, detail::Stratified2DIter>(
                nx* ny, sequenceIndex, seed)
            , nx(nx)
            , ny(ny)
        {}

    private:
        friend detail::Stratified2DIter;
        int nx;
        int ny;
    };

    class Stratified3D : public detail::RNGGenerator<Stratified3D, detail::Stratified3DIter>
    {
    public:
        LOQUAT_CPU_GPU
            Stratified3D(int nx, int ny, int nz, uint64_t sequenceIndex = 0,
                uint64_t seed = PCG32_DEFAULT_STATE)
            : detail::RNGGenerator<Stratified3D, detail::Stratified3DIter>(
                nx* ny* nz, sequenceIndex, seed)
            , nx(nx)
            , ny(ny)
            , nz(nz)
        {}

    private:
        friend detail::Stratified3DIter;
        int nx;
        int ny;
        int nz;
    };

    namespace detail
    {

        class Uniform1DIter : public RNGIterator<Uniform1DIter>
        {
        public:
            using RNGIterator<Uniform1DIter>::RNGIterator;
            LOQUAT_CPU_GPU
            Float operator*()
            {
                return rng.uniform<Float>();
            }
        };

        class Uniform2DIter : public RNGIterator<Uniform2DIter>
        {
        public:
            using RNGIterator<Uniform2DIter>::RNGIterator;
            LOQUAT_CPU_GPU
            Point2f operator*()
            {
                return { rng.uniform<Float>(), rng.uniform<Float>() };
            }
        };

        class Uniform3DIter : public RNGIterator<Uniform3DIter>
        {
        public:
            using RNGIterator<Uniform3DIter>::RNGIterator;
            LOQUAT_CPU_GPU
            Point3f operator*()
            {
                return { rng.uniform<Float>(), rng.uniform<Float>(), rng.uniform<Float>() };
            }
        };

        class Stratified1DIter : public RNGIterator<Stratified1DIter>
        {
        public:
            using RNGIterator<Stratified1DIter>::RNGIterator;
            LOQUAT_CPU_GPU
            Float operator*()
            {
                return (i + rng.uniform<Float>()) / n;
            }
        };

        class Stratified2DIter : public RNGIterator<Stratified2DIter>
        {
        public:
            LOQUAT_CPU_GPU
                Stratified2DIter(int i, int n, const Stratified2D* generator)
                : RNGIterator<Stratified2DIter>(i, n, generator)
                , nx(generator->nx)
                , ny(generator->ny)
            {}

            LOQUAT_CPU_GPU
            Point2f operator*()
            {
                int ix = i % nx, iy = i / nx;
                return { 
                    (ix + rng.uniform<Float>()) / nx,
                    (iy + rng.uniform<Float>()) / ny 
                };
            }

        private:
            int nx;
            int ny;
        };

        class Stratified3DIter : public RNGIterator<Stratified3DIter>
        {
        public:
            LOQUAT_CPU_GPU
            Stratified3DIter(int i, int n, const Stratified3D* generator)
                : RNGIterator<Stratified3DIter>(i, n, generator)
                , nx(generator->nx)
                , ny(generator->ny)
                , nz(generator->nz)
            {}

            LOQUAT_CPU_GPU
            Point3f operator*()
            {
                int ix = i % nx;
                int iy = (i / nx) % ny;
                int iz = i / (nx * ny);
                return { 
                    (ix + rng.uniform<Float>()) / nx,
                    (iy + rng.uniform<Float>()) / ny,
                    (iz + rng.uniform<Float>()) / nz 
                };
            }

        private:
            int nx;
            int ny;
            int nz;
        };

        class Hammersley2DIter : public IndexingIterator<Hammersley2DIter>
        {
        public:
            using IndexingIterator<Hammersley2DIter>::IndexingIterator;
            LOQUAT_CPU_GPU
            Point2f operator*()
            {
                return { Float(i) / Float(n), radical_inverse(0, i) };
            }
        };

        class Hammersley3DIter : public IndexingIterator<Hammersley3DIter>
        {
        public:
            using IndexingIterator<Hammersley3DIter>::IndexingIterator;
            LOQUAT_CPU_GPU
            Point3f operator*()
            {
                return {
                    Float(i) / Float(n),
                    radical_inverse(0, i),
                    radical_inverse(1, i)
                };
            }
        };

    }

    // Both PiecewiseConstant2D and Hierarchical2DWarp work for the warp here
#if 0
    template <typename W>
    Image warped_strata_visualization(const W& warp, int xs = 16, int ys = 16)
    {
        Image im(PixelFormat::Half, { warp.resolution().x / 2, warp.resolution().y / 2 }, { "R", "G", "B" });
        for (int y = 0; y < im.resolution().y; ++y)
        {
            for (int x = 0; x < im.resolution().x; ++x)
            {
                Point2f target = warp.get_domain().lerp(
                    { (x + .5f) / im.resolution().x,
                     (y + .5f) / im.resolution().y });
                if (warp.PDF(target) == 0) continue;

                pstd::optional<Point2f> u = warp.invert(target);
                if (!u.has_value())
                {
#if 0
                    LOG_WARNING(std::format("No value at target {}, though cont pdf = {}", target, tabdist.PDF(target)));
#endif
                    continue;
                }

#if 1
                int tile = int(u->x * xs) + xs * int(u->y * ys);
                Float rgb[3] = {
                    radical_inverse(0, tile), 
                    radical_inverse(1, tile),
                    radical_inverse(2, tile) 
                };
                im.set_channels({ x, int(y) }, { rgb[0], rgb[1], rgb[2] });
#else
                Float gray = ((int(u->x * xs) + int(u->y * ys)) & 1) ? 0.8 : 0.2;
                im.set_channel({ x, int(y) }, 0, gray);
#endif
            }
        }
        return im;
    }
#endif

    // PiecewiseLinear2D Implementation
    // *****************************************************************************
    // Marginal-conditional warp
    // *****************************************************************************

    /**
     * \brief Implements a marginal sample warping scheme for 2D distributions
     * with linear interpolation and an optional dependence on additional parameters
     *
     * This class takes a rectangular floating point array as input and constructs
     * internal data structures to efficiently map uniform variates from the unit
     * square <tt>[0, 1]^2</tt> to a function on <tt>[0, 1]^2</tt> that linearly
     * interpolates the input array.
     *
     * The mapping is constructed via the inversion method, which is applied to
     * a marginal distribution over rows, followed by a conditional distribution
     * over columns.
     *
     * The implementation also supports <em>conditional distributions</em>, i.e. 2D
     * distributions that depend on an arbitrary number of parameters (indicated
     * via the \c Dimension template parameter).
     *
     * In this case, the input array should have dimensions <tt>N0 x N1 x ... x Nn
     * x res[1] x res[0]</tt> (where the last dimension is contiguous in memory),
     * and the <tt>param_res</tt> should be set to <tt>{ N0, N1, ..., Nn }</tt>,
     * and <tt>param_values</tt> should contain the parameter values where the
     * distribution is discretized. Linear interpolation is used when sampling or
     * evaluating the distribution for in-between parameter values.
     */
    struct PLSample
    {
        Point2f p;
        Float pdf;
    };

    template <size_t Dimension = 0>
    class PiecewiseLinear2D
    {
    private:
        using FloatStorage = pstd::vector<float>;

#if !defined(_MSC_VER) && !defined(__CUDACC__)
        static constexpr size_t ArraySize = Dimension;
#else
        static constexpr size_t ArraySize = (Dimension != 0) ? Dimension : 1;
#endif

    public:
        PiecewiseLinear2D(Allocator alloc)
            : m_param_values(alloc)
            , m_data(alloc)
            , m_marginal_cdf(alloc)
            , m_conditional_cdf(alloc)
        {
            for (int i = 0; i < ArraySize; ++i)
            {
                m_param_values.emplace_back(alloc);
            }
        }

        /**
         * Construct a marginal sample warping scheme for floating point
         * data of resolution \c size.
         *
         * \c param_res and \c param_values are only needed for conditional
         * distributions (see the text describing the PiecewiseLinear2D class).
         *
         * If \c normalize is set to \c false, the implementation will not
         * re-scale the distribution so that it integrates to \c 1. It can
         * still be sampled (proportionally), but returned density values
         * will reflect the unnormalized values.
         *
         * If \c build_cdf is set to \c false, the implementation will not
         * construct the cdf needed for sample warping, which saves memory in case
         * this functionality is not needed (e.g. if only the interpolation in \c
         * eval() is used).
         */
        PiecewiseLinear2D(Allocator alloc, const float* data, int xSize, int ySize,
            pstd::array<int, Dimension> param_res = {},
            pstd::array<const float*, Dimension> param_values = {},
            bool normalize = true, bool build_cdf = true)
            : m_size(xSize, ySize)
            , m_patch_size(1.0f / (xSize - 1), 1.0f / (ySize - 1))
            , m_inv_patch_size(m_size - Vector2i(1, 1))
            , m_param_values(alloc)
            , m_data(alloc)
            , m_marginal_cdf(alloc)
            , m_conditional_cdf(alloc)
        {
            if (build_cdf && !normalize)
            {
                LOG_FATAL("PiecewiseLinear2D: build_cdf implies normalize=true");
            }

            /* Keep track of the dependence on additional parameters (optional) */
            uint32_t slices = 1;
            for (int i = 0; i < ArraySize; ++i)
            {
                m_param_values.emplace_back(alloc);
            }
            for (int i = (int)Dimension - 1; i >= 0; --i)
            {
                if (param_res[i] < 1)
                {
                    LOG_FATAL("PiecewiseLinear2D(): parameter resolution must be >= 1!");
                }

                m_param_size[i] = param_res[i];
                m_param_values[i] = FloatStorage(param_res[i]);
                memcpy(m_param_values[i].data(), param_values[i],
                    sizeof(float) * param_res[i]);
                m_param_strides[i] = param_res[i] > 1 ? slices : 0;
                slices *= m_param_size[i];
            }

            uint32_t n_values = xSize * ySize;

            m_data = FloatStorage(slices * n_values);

            if (build_cdf)
            {
                m_marginal_cdf = FloatStorage(slices * m_size.y);
                m_conditional_cdf = FloatStorage(slices * n_values);

                float* marginal_cdf = m_marginal_cdf.data();
                float* conditional_cdf = m_conditional_cdf.data();
                float* data_out = m_data.data();

                for (uint32_t slice = 0; slice < slices; ++slice)
                {
                    /* Construct conditional CDF */
                    for (int y = 0; y < m_size.y; ++y)
                    {
                        double sum = 0.0;
                        size_t i = y * xSize;
                        conditional_cdf[i] = 0.0f;
                        for (int x = 0; x < m_size.x - 1; ++x, ++i)
                        {
                            sum += 0.5 * ((double)data[i] + (double)data[i + 1]);
                            conditional_cdf[i + 1] = (float)sum;
                        }
                    }

                    /* Construct marginal CDF */
                    marginal_cdf[0] = 0.0f;
                    double sum = 0.0;
                    for (int y = 0; y < m_size.y - 1; ++y)
                    {
                        sum += 0.5 * ((double)conditional_cdf[(y + 1) * xSize - 1] +
                            (double)conditional_cdf[(y + 2) * xSize - 1]);
                        marginal_cdf[y + 1] = (float)sum;
                    }

                    /* Normalize CDFs and PDF (if requested) */
                    float normalization = 1.0f / marginal_cdf[m_size.y - 1];
                    for (size_t i = 0; i < n_values; ++i)
                    {
                        conditional_cdf[i] *= normalization;
                    }
                    for (size_t i = 0; i < m_size.y; ++i)
                    {
                        marginal_cdf[i] *= normalization;
                    }
                    for (size_t i = 0; i < n_values; ++i)
                    {
                        data_out[i] = data[i] * normalization;
                    }

                    marginal_cdf += m_size.y;
                    conditional_cdf += n_values;
                    data_out += n_values;
                    data += n_values;
                }
            }
            else
            {
                float* data_out = m_data.data();

                for (uint32_t slice = 0; slice < slices; ++slice)
                {
                    float normalization = 1.0f / h_prod(m_inv_patch_size);
                    if (normalize) {
                        double sum = 0.0;
                        for (int y = 0; y < m_size.y - 1; ++y) {
                            size_t i = y * xSize;
                            for (int x = 0; x < m_size.x - 1; ++x, ++i) {
                                float v00 = data[i], v10 = data[i + 1], v01 = data[i + xSize],
                                    v11 = data[i + 1 + xSize],
                                    avg = .25f * (v00 + v10 + v01 + v11);
                                sum += (double)avg;
                            }
                        }
                        normalization = float(1.0 / sum);
                    }

                    for (uint32_t k = 0; k < n_values; ++k)
                        data_out[k] = data[k] * normalization;

                    data += n_values;
                    data_out += n_values;
                }
            }
        }

        /**
         * \brief Given a uniformly distributed 2D sample, draw a sample from the
         * distribution (parameterized by \c param if applicable)
         *
         * Returns the warped sample and associated probability density.
         */
        template <typename... Ts>
        LOQUAT_CPU_GPU
        PLSample sample(Point2f sample, Ts... params) const
        {
            static_assert((std::is_arithmetic_v<Ts> && ...),
                "Additional parameters must be numeric values");
            static_assert(sizeof...(Ts) == Dimension,
                "Incorrect number of additional parameters passed");
            pstd::array<Float, Dimension> param = { params... };

            /* Avoid degeneracies at the extrema */
            sample[0] = clamp(sample[0], 1 - ONE_MINUS_EPSILON, ONE_MINUS_EPSILON);
            sample[1] = clamp(sample[1], 1 - ONE_MINUS_EPSILON, ONE_MINUS_EPSILON);

            /* Look up parameter-related indices and weights (if Dimension != 0) */
            float param_weight[2 * ArraySize];
            uint32_t slice_offset = 0u;
            for (size_t dim = 0; dim < Dimension; ++dim) {
                if (m_param_size[dim] == 1) {
                    param_weight[2 * dim] = 1.0f;
                    param_weight[2 * dim + 1] = 0.0f;
                    continue;
                }

                uint32_t param_index = find_interval(m_param_size[dim], [&](uint32_t idx) {
                    return m_param_values[dim].data()[idx] <= param[dim];
                    });

                Float p0 = m_param_values[dim][param_index],
                    p1 = m_param_values[dim][param_index + 1];

                param_weight[2 * dim + 1] = clamp((param[dim] - p0) / (p1 - p0), 0, 1);
                param_weight[2 * dim] = 1.0f - param_weight[2 * dim + 1];
                slice_offset += m_param_strides[dim] * param_index;
            }

            /* sample the row first */
            uint32_t offset = 0;
            if (Dimension != 0)
                offset = slice_offset * m_size.y;

            auto fetch_marginal = [&](uint32_t idx) -> float {
                return lookup<Dimension>(m_marginal_cdf.data(), offset + idx, m_size.y,
                    param_weight);
                };

            uint32_t row = find_interval(
                m_size.y, [&](uint32_t idx) { return fetch_marginal(idx) < sample.y; });

            sample.y -= fetch_marginal(row);

            uint32_t slice_size = h_prod(m_size);
            offset = row * m_size.x;
            if (Dimension != 0)
                offset += slice_offset * slice_size;

            Float r0 = lookup<Dimension>(m_conditional_cdf.data(), offset + m_size.x - 1,
                slice_size, param_weight),
                r1 =
                lookup<Dimension>(m_conditional_cdf.data(), offset + (m_size.x * 2 - 1),
                    slice_size, param_weight);

            bool is_const = std::abs(r0 - r1) < 1e-4f * (r0 + r1);
            sample.y = is_const ? (2.f * sample.y)
                : (r0 - safe_square_root(r0 * r0 - 2.f * sample.y * (r0 - r1)));
            sample.y /= is_const ? (r0 + r1) : (r0 - r1);

            /* sample the column next */
            sample.x *= (1.0f - sample.y) * r0 + sample.y * r1;

            auto fetch_conditional = [&](uint32_t idx) -> float {
                float v0 = lookup<Dimension>(m_conditional_cdf.data(), offset + idx,
                    slice_size, param_weight),
                    v1 = lookup<Dimension>(m_conditional_cdf.data() + m_size.x,
                        offset + idx, slice_size, param_weight);

                return (1.0f - sample.y) * v0 + sample.y * v1;
                };

            uint32_t col = find_interval(
                m_size.x, [&](uint32_t idx) { return fetch_conditional(idx) < sample.x; });

            sample.x -= fetch_conditional(col);

            offset += col;

            Float v00 = lookup<Dimension>(m_data.data(), offset, slice_size, param_weight),
                v10 =
                lookup<Dimension>(m_data.data() + 1, offset, slice_size, param_weight),
                v01 = lookup<Dimension>(m_data.data() + m_size.x, offset, slice_size,
                    param_weight),
                v11 = lookup<Dimension>(m_data.data() + m_size.x + 1, offset, slice_size,
                    param_weight),
                c0 = FMA((1.0f - sample.y), v00, sample.y * v01),
                c1 = FMA((1.0f - sample.y), v10, sample.y * v11);

            is_const = std::abs(c0 - c1) < 1e-4f * (c0 + c1);
            sample.x = is_const ? (2.f * sample.x)
                : (c0 - safe_square_root(c0 * c0 - 2.f * sample.x * (c0 - c1)));
            sample.x /= is_const ? (c0 + c1) : (c0 - c1);

            return {
                Point2f((col + sample.x) * m_patch_size.x, (row + sample.y) * m_patch_size.y),
                ((1.0f - sample.x) * c0 + sample.x * c1) * h_prod(m_inv_patch_size) };
        }

        /// Inverse of the mapping implemented in \c sample()
        template <typename... Ts>
        LOQUAT_CPU_GPU
        PLSample invert(Point2f sample, Ts... params) const
        {
            static_assert((std::is_arithmetic_v<Ts> && ...),
                "Additional parameters must be numeric values");
            static_assert(sizeof...(Ts) == Dimension,
                "Incorrect number of additional parameters passed");
            pstd::array<Float, Dimension> param = { params... };

            /* Look up parameter-related indices and weights (if Dimension != 0) */
            float param_weight[2 * ArraySize];
            uint32_t slice_offset = 0u;
            for (size_t dim = 0; dim < Dimension; ++dim)
            {
                if (m_param_size[dim] == 1)
                {
                    param_weight[2 * dim] = 1.0f;
                    param_weight[2 * dim + 1] = 0.0f;
                    continue;
                }

                uint32_t param_index = find_interval(m_param_size[dim], [&](uint32_t idx) {
                    return m_param_values[dim][idx] <= param[dim];
                    });

                float p0 = m_param_values[dim][param_index],
                    p1 = m_param_values[dim][param_index + 1];

                param_weight[2 * dim + 1] = clamp((param[dim] - p0) / (p1 - p0), 0.0f, 1.0f);
                param_weight[2 * dim] = 1.0f - param_weight[2 * dim + 1];
                slice_offset += m_param_strides[dim] * param_index;
            }

            /* Fetch values at corners of bilinear patch */
            sample.x *= m_inv_patch_size.x;
            sample.y *= m_inv_patch_size.y;
            Vector2i pos = Min(Vector2i(sample), m_size - Vector2i(2, 2));
            sample -= Vec2f(pos);

            uint32_t offset = pos.x + pos.y * m_size.x;
            uint32_t slice_size = h_prod(m_size);
            if (Dimension != 0)
                offset += slice_offset * slice_size;

            /* invert the X component */
            Float v00 = lookup<Dimension>(m_data.data(), offset, slice_size, param_weight),
                v10 =
                lookup<Dimension>(m_data.data() + 1, offset, slice_size, param_weight),
                v01 = lookup<Dimension>(m_data.data() + m_size.x, offset, slice_size,
                    param_weight),
                v11 = lookup<Dimension>(m_data.data() + m_size.x + 1, offset, slice_size,
                    param_weight);

            Vec2f w1 = Vec2f(sample), w0 = Vec2f(1, 1) - w1;

            Float c0 = FMA(w0.y, v00, w1.y * v01), c1 = FMA(w0.y, v10, w1.y * v11),
                pdf = FMA(w0.x, c0, w1.x * c1);

            sample.x *= c0 + .5f * sample.x * (c1 - c0);

            Float v0 = lookup<Dimension>(m_conditional_cdf.data(), offset, slice_size,
                param_weight),
                v1 = lookup<Dimension>(m_conditional_cdf.data() + m_size.x, offset,
                    slice_size, param_weight);

            sample.x += (1.0f - sample.y) * v0 + sample.y * v1;

            offset = pos.y * m_size.x;
            if (Dimension != 0)
                offset += slice_offset * slice_size;

            Float r0 = lookup<Dimension>(m_conditional_cdf.data(), offset + m_size.x - 1,
                slice_size, param_weight),
                r1 =
                lookup<Dimension>(m_conditional_cdf.data(), offset + (m_size.x * 2 - 1),
                    slice_size, param_weight);

            sample.x /= (1.0f - sample.y) * r0 + sample.y * r1;

            /* invert the Y component */
            sample.y *= r0 + .5f * sample.y * (r1 - r0);

            offset = pos.y;
            if (Dimension != 0)
                offset += slice_offset * m_size.y;

            sample.y +=
                lookup<Dimension>(m_marginal_cdf.data(), offset, m_size.y, param_weight);

            return { sample, pdf * h_prod(m_inv_patch_size) };
        }

        /**
         * \brief Evaluate the density at position \c pos. The distribution is
         * parameterized by \c param if applicable.
         */
        template <typename... Ts>
        LOQUAT_CPU_GPU
        float Evaluate(Point2f pos, Ts... params) const
        {
            static_assert((std::is_arithmetic_v<Ts> && ...),
                "Additional parameters must be numeric values");
            static_assert(sizeof...(Ts) == Dimension,
                "Incorrect number of additional parameters passed");
            pstd::array<Float, Dimension> param = { params... };

            /* Look up parameter-related indices and weights (if Dimension != 0) */
            float param_weight[2 * ArraySize];
            uint32_t slice_offset = 0u;

            for (size_t dim = 0; dim < Dimension; ++dim)
            {
                if (m_param_size[dim] == 1)
                {
                    param_weight[2 * dim] = 1.0f;
                    param_weight[2 * dim + 1] = 0.0f;
                    continue;
                }

                uint32_t param_index = find_interval(m_param_size[dim], [&](uint32_t idx) {
                    return m_param_values[dim][idx] <= param[dim];
                    });

                float p0 = m_param_values[dim][param_index],
                    p1 = m_param_values[dim][param_index + 1];

                param_weight[2 * dim + 1] = clamp((param[dim] - p0) / (p1 - p0), 0.0f, 1.0f);
                param_weight[2 * dim] = 1.0f - param_weight[2 * dim + 1];
                slice_offset += m_param_strides[dim] * param_index;
            }

            /* Compute linear interpolation weights */
            pos.x *= m_inv_patch_size.x;
            pos.y *= m_inv_patch_size.y;
            Vector2i offset = Min(Vector2i(pos), m_size - Vector2i(2, 2));

            Vec2f w1 = Vec2f(pos) - Vec2f(Vector2i(offset)),
                w0 = Vec2f(1, 1) - w1;

            uint32_t index = offset.x + offset.y * m_size.x;

            uint32_t size = h_prod(m_size);
            if (Dimension != 0)
                index += slice_offset * size;

            Float v00 = lookup<Dimension>(m_data.data(), index, size, param_weight),
                v10 = lookup<Dimension>(m_data.data() + 1, index, size, param_weight),
                v01 =
                lookup<Dimension>(m_data.data() + m_size.x, index, size, param_weight),
                v11 = lookup<Dimension>(m_data.data() + m_size.x + 1, index, size,
                    param_weight);

            return FMA(w0.y, FMA(w0.x, v00, w1.x * v10), w1.y * FMA(w0.x, v01, w1.x * v11)) *
                h_prod(m_inv_patch_size);
        }

        LOQUAT_CPU_GPU
        size_t bytes_used() const
        {
            size_t sum = 4 * (m_data.capacity() + m_marginal_cdf.capacity() +
                m_conditional_cdf.capacity());
            for (int i = 0; i < ArraySize; ++i)
                sum += m_param_values[i].capacity();
            return sum;
        }

    private:
        template <size_t Dim, std::enable_if_t<Dim != 0, int> = 0>
        LOQUAT_CPU_GPU
        Float lookup(const float* data, uint32_t i0, uint32_t size,
            const float* param_weight) const
        {
            uint32_t i1 = i0 + m_param_strides[Dim - 1] * size;

            Float w0 = param_weight[2 * Dim - 2], w1 = param_weight[2 * Dim - 1],
                v0 = lookup<Dim - 1>(data, i0, size, param_weight),
                v1 = lookup<Dim - 1>(data, i1, size, param_weight);

            return FMA(v0, w0, v1 * w1);
        }

        template <size_t Dim, std::enable_if_t<Dim == 0, int> = 0>
        LOQUAT_CPU_GPU
        Float lookup(const float* data, uint32_t index, uint32_t,
            const float*) const
        {
            return data[index];
        }

        /// resolution of the discretized density function
        Vector2i m_size;

        /// Size of a bilinear patch in the unit square
        Vec2f m_patch_size;
        Vec2f m_inv_patch_size;

        /// resolution of each parameter (optional)
        uint32_t m_param_size[ArraySize];

        /// Stride per parameter in units of sizeof(float)
        uint32_t m_param_strides[ArraySize];

        /// Discretization of each parameter domain
        pstd::vector<FloatStorage> m_param_values;

        /// Density values
        FloatStorage m_data;

        /// Marginal and conditional PDFs
        FloatStorage m_marginal_cdf;
        FloatStorage m_conditional_cdf;
    };
}

