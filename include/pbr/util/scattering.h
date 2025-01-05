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
    inline Vec3f reflect(Vec3f wo, Vec3f n)
    {
        return -wo + 2 * dot(wo, n) * n;
    }

    LOQUAT_CPU_GPU
    inline bool refract(Vec3f wi, Normal3f n, Float eta, Float* etap,
        Vec3f* wt)
    {
        Float cosTheta_i = dot(n, wi);
        // Potentially flip interface orientation for Snell's law
        if (cosTheta_i < 0) {
            eta = 1 / eta;
            cosTheta_i = -cosTheta_i;
            n = -n;
        }

        // Compute $\cos\,\theta_\roman{t}$ using Snell's law
        Float sin2Theta_i = std::max<Float>(0, 1 - square(cosTheta_i));
        Float sin2Theta_t = sin2Theta_i / square(eta);
        // Handle total internal reflection case
        if (sin2Theta_t >= 1)
            return false;

        Float cosTheta_t = std::sqrt(1 - sin2Theta_t);

        *wt = -wi / eta + (cosTheta_i / eta - cosTheta_t) * Vec3f(n);
        // Provide relative IOR along ray to caller
        if (etap)
            *etap = eta;

        return true;
    }

    LOQUAT_CPU_GPU
    inline Float henyey_greenstein(Float cosTheta, Float g)
    {
        // The Henyey-Greenstein phase function isn't suitable for |g| \approx
        // 1 so we clamp it before it becomes numerically instable. (It's an
        // analogous situation to BSDFs: if the BSDF is perfectly specular, one
        // should use one based on a Dirac delta distribution rather than a
        // very smooth microfacet distribution...)
        g = clamp(g, -.99, .99);
        Float denom = 1 + square(g) + 2 * g * cosTheta;
        return INV_4PI * (1 - square(g)) / (denom * safe_square_root(denom));
    }

    LOQUAT_CPU_GPU
    inline Float fr_dielectric(Float cosTheta_i, Float eta)
    {
        cosTheta_i = clamp(cosTheta_i, -1, 1);
        // Potentially flip interface orientation for Fresnel equations
        if (cosTheta_i < 0) {
            eta = 1 / eta;
            cosTheta_i = -cosTheta_i;
        }

        // Compute $\cos\,\theta_\roman{t}$ for Fresnel equations using Snell's law
        Float sin2Theta_i = 1 - square(cosTheta_i);
        Float sin2Theta_t = sin2Theta_i / square(eta);
        if (sin2Theta_t >= 1)
            return 1.f;
        Float cosTheta_t = safe_square_root(1 - sin2Theta_t);

        Float r_parl = (eta * cosTheta_i - cosTheta_t) / (eta * cosTheta_i + cosTheta_t);
        Float r_perp = (cosTheta_i - eta * cosTheta_t) / (cosTheta_i + eta * cosTheta_t);
        return (square(r_parl) + square(r_perp)) / 2;
    }

    LOQUAT_CPU_GPU
    inline Float fr_complex(Float cosTheta_i, pstd::complex<Float> eta)
    {
        using Complex = pstd::complex<Float>;
        cosTheta_i = clamp(cosTheta_i, 0, 1);
        // Compute complex $\cos\,\theta_\roman{t}$ for Fresnel equations using Snell's law
        Float sin2Theta_i = 1 - square(cosTheta_i);
        Complex sin2Theta_t = sin2Theta_i / square(eta);
        Complex cosTheta_t = pstd::sqrt(1 - sin2Theta_t);

        Complex r_parl = (eta * cosTheta_i - cosTheta_t) / (eta * cosTheta_i + cosTheta_t);
        Complex r_perp = (cosTheta_i - eta * cosTheta_t) / (cosTheta_i + eta * cosTheta_t);
        return (pstd::norm(r_parl) + pstd::norm(r_perp)) / 2;
    }

    LOQUAT_CPU_GPU
    inline SampledSpectrum fr_complex(Float cosTheta_i, SampledSpectrum eta,
        SampledSpectrum k)
    {
        SampledSpectrum result;
        for (int i = 0; i < SPECTRUM_SAMPLE_COUNT; ++i)
            result[i] = fr_complex(cosTheta_i, pstd::complex<Float>(eta[i], k[i]));
        return result;
    }

    LOQUAT_CPU_GPU
    Float fresnel_moment_1(Float invEta);
    LOQUAT_CPU_GPU
    Float fresnel_moment_2(Float invEta);

    class TrowbridgeReitzDistribution {
    public:
        TrowbridgeReitzDistribution() = default;
        LOQUAT_CPU_GPU
        TrowbridgeReitzDistribution(Float ax, Float ay)
            : alpha_x(ax)
            , alpha_y(ay)
        {
            if (!effectively_smooth()) {
                // If one direction has some roughness, then the other can't
                // have zero (or very low) roughness; the computation of |e| in
                // d() blows up in that case.
                alpha_x = std::max<Float>(alpha_x, 1e-4f);
                alpha_y = std::max<Float>(alpha_y, 1e-4f);
            }
        }

        LOQUAT_CPU_GPU
        inline Float d(Vec3f wm) const
        {
            Float tan2Theta = tan2_theta(wm);
            if (is_inf(tan2Theta))
                return 0;
            Float cos4Theta = square(cos2_theta(wm));
            if (cos4Theta < 1e-16f)
                return 0;
            Float e = tan2Theta * (square(cos_phi(wm) / alpha_x) + square(sin_phi(wm) / alpha_y));
            return 1 / (PI * alpha_x * alpha_y * cos4Theta * square(1 + e));
        }

        LOQUAT_CPU_GPU
        bool effectively_smooth() const
        {
            return std::max(alpha_x, alpha_y) < 1e-3f;
        }

        LOQUAT_CPU_GPU
        Float G1(Vec3f w) const
        {
            return 1 / (1 + lambda(w));
        }

        LOQUAT_CPU_GPU
        Float lambda(Vec3f w) const
        {
            Float tan2Theta = tan2_theta(w);
            if (is_inf(tan2Theta))
                return 0;
            Float alpha2 = square(cos_phi(w) * alpha_x) + square(sin_phi(w) * alpha_y);
            return (std::sqrt(1 + alpha2 * tan2Theta) - 1) / 2;
        }

        LOQUAT_CPU_GPU
        Float G(Vec3f wo, Vec3f wi) const
        {
            return 1 / (1 + lambda(wo) + lambda(wi));
        }

        LOQUAT_CPU_GPU
        Float d(Vec3f w, Vec3f wm) const
        {
            return G1(w) / abs_cos_theta(w) * d(wm) * absolute_dot(w, wm);
        }

        LOQUAT_CPU_GPU
        Float PDF(Vec3f w, Vec3f wm) const
        {
            return d(w, wm);
        }

        LOQUAT_CPU_GPU
        Vec3f sample_wm(Vec3f w, Point2f u) const
        {
            // Transform _w_ to hemispherical configuration
            Vec3f wh = normalize(Vec3f(alpha_x * w.x, alpha_y * w.y, w.z));
            if (wh.z < 0)
                wh = -wh;

            // Find orthonormal basis for visible normal sampling
            Vec3f T1 = (wh.z < 0.99999f) ? normalize(cross(Vec3f(0, 0, 1), wh))
                : Vec3f(1, 0, 0);
            Vec3f T2 = cross(wh, T1);

            // Generate uniformly distributed points on the unit disk
            Point2f p = sample_uniform_disk_polar(u);

            // Warp hemispherical projection for visible normal sampling
            Float h = std::sqrt(1 - square(p.x));
            p.y = lerp((1 + wh.z) / 2, h, p.y);

            // Reproject to hemisphere and transform normal to ellipsoid configuration
            Float pz = std::sqrt(std::max<Float>(0, 1 - length_squared(Vec2f(p))));
            Vec3f nh = p.x * T1 + p.y * T2 + pz * wh;
            return normalize(
                Vec3f(alpha_x * nh.x, alpha_y * nh.y, std::max<Float>(1e-6f, nh.z)));
        }

        std::string to_string() const;

        LOQUAT_CPU_GPU
        static Float roughness_to_alpha(Float roughness)
        {
            return std::sqrt(roughness);
        }

        LOQUAT_CPU_GPU
        void regularize()
        {
            if (alpha_x < 0.3f)
                alpha_x = clamp(2 * alpha_x, 0.1f, 0.3f);
            if (alpha_y < 0.3f)
                alpha_y = clamp(2 * alpha_y, 0.1f, 0.3f);
        }

    private:
        Float alpha_x;
        Float alpha_y;
    };
}

//TODO(ches) fix this