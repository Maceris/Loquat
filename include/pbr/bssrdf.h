// pbrt is Copyright(c) 1998-2020 Matt Pharr, Wenzel Jakob, and Greg Humphreys.
// The pbrt source code is licensed under the Apache License, Version 2.0.
// SPDX: Apache-2.0

// This file has been modified from the original, original notice is above.

#pragma once

#include <string>

#include "main/loquat.h"

#include "pbr/base/bssrdf.h"
#include "pbr/bsdf.h"
#include "pbr/math/vector_math.h"
#include "pbr/struct/interaction.h"
#include "pbr/util/pstd.h"
#include "pbr/util/scattering.h"
#include "pbr/util/spectrum.h"
#include "pbr/util/tagged_pointer.h"

namespace loquat
{
    struct BSSRDFSample
    {
        SampledSpectrum spatial_distribution;
        SampledSpectrum pdf;
        BSDF boundary_BSDF;
        Vec3f outgoing;
    };

    struct SubsurfaceInteraction
    {
        SubsurfaceInteraction() = default;

        LOQUAT_CPU_GPU
        SubsurfaceInteraction(const SurfaceInteraction& si)
            : point(si.point),
            normal(si.normal),
            dpdu(si.dpdu),
            dpdv(si.dpdv),
            shading_normal(si.shading.normal),
            dpdus(si.shading.dpdu),
            dpdvs(si.shading.dpdv) {}

        LOQUAT_CPU_GPU
        operator SurfaceInteraction() const {
            SurfaceInteraction si;
            si.point = point;
            si.normal = normal;
            si.dpdu = dpdu;
            si.dpdv = dpdv;
            si.shading.normal = shading_normal;
            si.shading.dpdu = dpdus;
            si.shading.dpdv = dpdvs;
            return si;
        }

        LOQUAT_CPU_GPU
        Point3f p() const { return Point3f(point); }

        Point3fi point;
        Normal3f normal;
        Normal3f shading_normal;
        Vec3f dpdu;
        Vec3f dpdv;
        Vec3f dpdus;
        Vec3f dpdvs;
    };

    Float beam_diffusion_ss(Float sigma_s, Float sigma_a, Float g, Float eta, Float r);
    Float beam_diffusion_ms(Float sigma_s, Float sigma_a, Float g, Float eta, Float r);

    void compute_beam_diffusion_BSSRDF(Float g, Float eta, BSSRDFTable* t);

    struct BSSRDFTable
    {
        pstd::vector<Float> rho_samples;
        pstd::vector<Float> radius_samples;
        pstd::vector<Float> profile;
        pstd::vector<Float> rho_effect;
        pstd::vector<Float> profile_CDF;

        BSSRDFTable(int rho_sample_count, int radius_sample_count, Allocator alloc);

        std::string to_string() const;

        LOQUAT_CPU_GPU
        Float evaluate_profile(int rho_index, int radius_index) const
        {
            LOG_ASSERT(rho_index >= 0 && rho_index < rho_samples.size());
            LOG_ASSERT(radius_index >= 0 && radius_index < radius_samples.size());
            return profile[rho_index * radius_samples.size() + radius_index];
        }
    };

    struct BSSRDFProbeSegment
    {
        BSSRDFProbeSegment() = default;
        LOQUAT_CPU_GPU
        BSSRDFProbeSegment(Point3f p0, Point3f p1)
            : p0(p0)
            , p1(p1)
        {}

        Point3f p0;
        Point3f p1;
    };

    class TabulatedBSSRDF {
    public:
        using BxDF = NormalizedFresnelBxDF;

        TabulatedBSSRDF() = default;
        LOQUAT_CPU_GPU
        TabulatedBSSRDF(Point3f po, Normal3f shading_normal, Vec3f outgoing, Float eta,
            const SampledSpectrum& sigma_a, const SampledSpectrum& sigma_s,
            const BSSRDFTable* table)
            : po(po)
            , outgoing(outgoing)
            , eta(eta)
            , shading_normal(shading_normal)
            , table(table)
        {
            sigma_t = sigma_a + sigma_s;
            rho = safe_divide(sigma_s, sigma_t);
        }

        LOQUAT_CPU_GPU
        SampledSpectrum spatial_distribution(Point3f point) const
        {
            return scattering_profile(loquat::distance(po, point));
        }

        LOQUAT_CPU_GPU
        SampledSpectrum scattering_profile(Float r) const
        {
            SampledSpectrum scattering_profile(0.0f);
            for (int i = 0; i < SPECTRUM_SAMPLE_COUNT; ++i)
            {
                // Convert $r$ into unitless optical radius $r_{\roman{optical}}$
                Float rOptical = r * sigma_t[i];

                // Compute spline weights to interpolate BSSRDF at _i_th wavelength
                int rhoOffset;
                int radiusOffset;
                Float rhoWeights[4];
                Float radiusWeights[4];
                if (!catmull_rom_weights(table->rho_samples, rho[i], &rhoOffset, rhoWeights) 
                    || !catmull_rom_weights(table->radius_samples, rOptical, &radiusOffset,
                        radiusWeights))
                {
                    continue;
                }

                // Set BSSRDF value _Sr[i]_ using tensor spline interpolation
                Float sr = 0;
                for (int j = 0; j < 4; ++j)
                {
                    for (int k = 0; k < 4; ++k)
                    {
                        // Accumulate contribution of $(j,k)$ table sample
                        if (Float weight = rhoWeights[j] * radiusWeights[k]; weight != 0)
                        {
                            sr += weight * table->evaluate_profile(rhoOffset + j, radiusOffset + k);
                        }
                    }
                }
                // Cancel marginal PDF factor from tabulated BSSRDF profile
                if (rOptical != 0)
                    sr /= 2 * PI * rOptical;

                scattering_profile[i] = sr;
            }
            // Transform BSSRDF value into rendering space units
            scattering_profile *= square(sigma_t);

            return clamp_zero(scattering_profile);
        }

        LOQUAT_CPU_GPU
        pstd::optional<Float> sample_scattering_profile(Float u) const
        {
            if (sigma_t[0] == 0)
            {
                return {};
            }
            return sample_catmull_rom_2D(table->rho_samples,
                table->radius_samples, table->profile,
                table->profile_CDF, rho[0], u) 
                / sigma_t[0];
        }

        LOQUAT_CPU_GPU
        SampledSpectrum PDF_scattering_profile(Float r) const
        {
            SampledSpectrum pdf(0.f);
            for (int i = 0; i < SPECTRUM_SAMPLE_COUNT; ++i)
            {
                // Convert $r$ into unitless optical radius $r_{\roman{optical}}$
                Float rOptical = r * sigma_t[i];

                // Compute spline weights to interpolate BSSRDF at _i_th wavelength
                int rhoOffset;
                int radiusOffset;
                Float rhoWeights[4];
                Float radiusWeights[4];
                if (!catmull_rom_weights(table->rho_samples, rho[i], &rhoOffset, rhoWeights) 
                    || !catmull_rom_weights(table->radius_samples, rOptical, &radiusOffset,
                        radiusWeights))
                {
                    continue;
                }

                // Set BSSRDF profile probability density for wavelength
                Float sr = 0, rho_effect = 0;
                for (int j = 0; j < 4; ++j)
                {
                    if (rhoWeights[j] != 0) 
                    {
                        // Update _rhoEff_ and _sr_ for wavelength
                        rho_effect += table->rho_effect[rhoOffset + j] * rhoWeights[j];
                        for (int k = 0; k < 4; ++k)
                        {
                            if (radiusWeights[k] != 0)
                            {
                                sr += table->evaluate_profile(rhoOffset + j, radiusOffset + k) *
                                    rhoWeights[j] * radiusWeights[k];
                            }
                        }
                    }
                }
                // Cancel marginal PDF factor from tabulated BSSRDF profile
                if (rOptical != 0)
                {
                    sr /= 2 * PI * rOptical;
                }

                pdf[i] = sr * square(sigma_t[i]) / rho_effect;
            }
            return clamp_zero(pdf);
        }

        LOQUAT_CPU_GPU
        pstd::optional<BSSRDFProbeSegment> sample_spatial(Float u1, Point2f u2) const 
        {
            // Choose projection axis for BSSRDF sampling
            Frame f;
            if (u1 < 0.25f)
            {
                f = Frame::from_x(shading_normal);
            }
            else if (u1 < 0.5f)
            {
                f = Frame::from_y(shading_normal);
            }
            else
            {
                f = Frame::from_z(shading_normal);
            }

            // Sample BSSRDF profile in polar coordinates
            pstd::optional<Float> r = sample_scattering_profile(u2[0]);
            if (!r)
                return {};
            Float phi = 2 * PI * u2[1];

            // Compute BSSRDF profile bounds and intersection height
            pstd::optional<Float> r_max = sample_scattering_profile(0.999f);
            if (!r_max || *r >= *r_max)
                return {};
            Float l = 2 * std::sqrt(square(*r_max) - square(*r));

            // Return BSSRDF sampling ray segment
            Point3f pStart =
                po + *r * (f.x * std::cos(phi) + f.y * std::sin(phi)) - l * f.z / 2.0f;
            Point3f pTarget = pStart + l * f.z;
            return BSSRDFProbeSegment{ pStart, pTarget };
        }

        LOQUAT_CPU_GPU
        SampledSpectrum pdf_spatial(Point3f point, Normal3f ni) const
        {
            // Express $\pti-\pto$ and $\N{}_\roman{i}$ with respect to local coordinates at
            // $\pto$
            Vec3f d = point - po;
            Frame f = Frame::from_z(shading_normal);
            Vec3f dLocal = f.to_local(d);
            Normal3f nLocal = f.to_local(ni);

            // Compute BSSRDF profile radius under projection along each axis
            Float rProj[3] = { std::sqrt(square(dLocal.y) + square(dLocal.z)),
                              std::sqrt(square(dLocal.z) + square(dLocal.x)),
                              std::sqrt(square(dLocal.x) + square(dLocal.y)) };

            // Return combined probability from all BSSRDF sampling strategies
            SampledSpectrum pdf(0.f);
            Float axisProb[3] = { .25f, .25f, .5f };
            for (int axis = 0; axis < 3; ++axis)
                pdf += PDF_scattering_profile(rProj[axis]) * std::abs(nLocal[axis]) * axisProb[axis];
            return pdf;
        }

        LOQUAT_CPU_GPU
        BSSRDFSample probe_intersection_to_sample(const SubsurfaceInteraction& si,
                NormalizedFresnelBxDF* bxdf) const
        {
            *bxdf = NormalizedFresnelBxDF(eta);
            Vec3f outgoing = Vec3f(si.shading_normal);
            BSDF bsdf(si.shading_normal, si.dpdus, bxdf);
            return BSSRDFSample{ spatial_distribution(si.p()), pdf_spatial(si.p(), si.normal), bsdf, outgoing };
        }

        std::string to_string() const;

    private:
        friend struct SOA<TabulatedBSSRDF>;
        Point3f po;
        Vec3f outgoing;
        Normal3f shading_normal;
        Float eta;
        SampledSpectrum sigma_t;
        SampledSpectrum rho;
        const BSSRDFTable* table;
    };

    LOQUAT_CPU_GPU
    inline void subsurface_from_diffuse(const BSSRDFTable& t,
        const SampledSpectrum& rho_effect,
        const SampledSpectrum& mfp,
        SampledSpectrum* sigma_a,
        SampledSpectrum* sigma_s) {
        for (int c = 0; c < SPECTRUM_SAMPLE_COUNT; ++c) {
            Float rho = invert_catmull_rom(t.rho_samples, t.rho_effect, rho_effect[c]);
            (*sigma_s)[c] = rho / mfp[c];
            (*sigma_a)[c] = (1 - rho) / mfp[c];
        }
    }

    LOQUAT_CPU_GPU
    inline pstd::optional<BSSRDFProbeSegment> BSSRDF::sample(Float u1, Point2f u2) const
    {
        auto sample = [&](auto ptr) { return ptr->sample_spatial(u1, u2); };
        return dispatch(sample);
    }

    inline BSSRDFSample BSSRDF::probe_intersection_to_sample(
        const SubsurfaceInteraction& si, ScratchBuffer& scratchBuffer) const
    {
        auto pits = [&](auto ptr) {
            using BxDF = typename std::remove_reference_t<decltype(*ptr)>::BxDF;
            BxDF* bxdf = (BxDF*)scratchBuffer.alloc(sizeof(BxDF), alignof(BxDF));
            return ptr->probe_intersection_to_sample(si, bxdf);
            };
        return dispatchCPU(pits);
    }
}
