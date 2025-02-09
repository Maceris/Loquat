// pbrt is Copyright(c) 1998-2020 Matt Pharr, Wenzel Jakob, and Greg Humphreys.
// The pbrt source code is licensed under the Apache License, Version 2.0.
// SPDX: Apache-2.0

// This file has been modified from the original, original notice is above.

#pragma once

#include <algorithm>
#include <cmath>
#include <format>
#include <limits>
#include <string>

#include "main/loquat.h"

#include "pbr/base/bxdf.h"
#include "pbr/struct/interaction.h"
#include "pbr/media.h"
#include "pbr/options.h"
#include "pbr/math/math.h"
#include "pbr/math/vector_math.h"
#include "pbr/util/memory.h"
#include "pbr/util/pstd.h"
#include "pbr/util/scattering.h"
#include "pbr/util/spectrum.h"
#include "pbr/util/tagged_pointer.h"

namespace loquat
{
    class DiffuseBxDF
    {
    public:
        DiffuseBxDF() = default;
        LOQUAT_CPU_GPU
        DiffuseBxDF(SampledSpectrum R) : R(R) {}

        LOQUAT_CPU_GPU
        SampledSpectrum f(Vec3f outgoing, Vec3f incoming, TransportMode mode) const
        {
            if (!same_hemisphere(outgoing, incoming))
            {
                return SampledSpectrum(0.0f);
            }
            return R * INV_PI;
        }

        LOQUAT_CPU_GPU
        pstd::optional<BSDFSample> sample_f(
            Vec3f outgoing, Float uc, Point2f u, TransportMode mode,
            BxDFReflTransFlags sample_flags = BxDFReflTransFlags::All) const
        {
            if (!(sample_flags & BxDFReflTransFlags::Reflection))
            {
                return {};
            }
            // Sample cosine-weighted hemisphere to compute _incoming_ and _pdf_
            Vec3f incoming = sample_cosine_hemisphere(u);
            if (outgoing.z < 0)
            {
                incoming.z *= -1;
            }
            Float pdf = cosine_hemisphere_PDF(absolute_cos_theta(incoming));

            return BSDFSample(R * INV_PI, incoming, pdf,
                BxDFFlags::DiffuseReflection);
        }

        LOQUAT_CPU_GPU
        Float PDF(Vec3f outgoing, Vec3f incoming, TransportMode mode,
                BxDFReflTransFlags sample_flags = BxDFReflTransFlags::All) const
        {
            if (!(sample_flags & BxDFReflTransFlags::Reflection) 
                || !same_hemisphere(outgoing, incoming))
            {
                return 0;
            }
            return cosine_hemisphere_PDF(absolute_cos_theta(incoming));
        }

        LOQUAT_CPU_GPU
        static constexpr const char* get_name()
        {
            return "DiffuseBxDF";
        }

        [[nodiscard]]
        std::string to_string() const;

        LOQUAT_CPU_GPU
        void regularize()
        {}

        LOQUAT_CPU_GPU
        BxDFFlags get_flags() const
        {
            return R ? BxDFFlags::DiffuseReflection : BxDFFlags::Unset;
        }

    private:
        SampledSpectrum R;
    };


    class DiffuseTransmissionBxDF {
    public:
        DiffuseTransmissionBxDF() = default;
        LOQUAT_CPU_GPU
        DiffuseTransmissionBxDF(SampledSpectrum R, SampledSpectrum T)
            : R(R)
            , T(T)
        {}

        LOQUAT_CPU_GPU
        SampledSpectrum f(Vec3f wo, Vec3f incoming, TransportMode mode) const
        {
            return same_hemisphere(wo, incoming) ? (R * INV_PI) : (T * INV_PI);
        }

        LOQUAT_CPU_GPU
        pstd::optional<BSDFSample> sample_f(
            Vec3f wo, Float uc, Point2f u, TransportMode mode,
            BxDFReflTransFlags sample_flags = BxDFReflTransFlags::All) const
        {
            // Compute reflection and transmission probabilities for diffuse BSDF
            Float pr = R.max_component_value(), pt = T.max_component_value();
            if (!(sample_flags & BxDFReflTransFlags::Reflection))
            {
                pr = 0;
            }
            if (!(sample_flags & BxDFReflTransFlags::Transmission))
            {
                pt = 0;
            }
            if (pr == 0 && pt == 0)
            {
                return {};
            }

            // Randomly sample diffuse BSDF reflection or transmission
            if (uc < pr / (pr + pt))
            {
                // Sample diffuse BSDF reflection
                Vec3f incoming = sample_cosine_hemisphere(u);
                if (wo.z < 0)
                {
                    incoming.z *= -1;
                }
                Float pdf = cosine_hemisphere_PDF(abs_cos_theta(incoming)) * pr / (pr + pt);
                return BSDFSample(f(wo, incoming, mode), incoming, pdf, BxDFFlags::DiffuseReflection);

            }
            else
            {
                // Sample diffuse BSDF transmission
                Vec3f incoming = sample_cosine_hemisphere(u);
                if (wo.z > 0)
                {
                    incoming.z *= -1;
                }
                Float pdf = cosine_hemisphere_PDF(abs_cos_theta(incoming)) * pt / (pr + pt);
                return BSDFSample(f(wo, incoming, mode), incoming, pdf, BxDFFlags::DiffuseTransmission);
            }
        }

        LOQUAT_CPU_GPU
        Float PDF(Vec3f wo, Vec3f incoming, TransportMode mode,
            BxDFReflTransFlags sample_flags = BxDFReflTransFlags::All) const
        {
            // Compute reflection and transmission probabilities for diffuse BSDF
            Float pr = R.max_component_value(), pt = T.max_component_value();
            if (!(sample_flags & BxDFReflTransFlags::Reflection))
            {
                pr = 0;
            }
            if (!(sample_flags & BxDFReflTransFlags::Transmission))
            {
                pt = 0;
            }
            if (pr == 0 && pt == 0)
            {
                return {};
            }

            if (same_hemisphere(wo, incoming))
            {
                return pr / (pr + pt) * cosine_hemisphere_PDF(abs_cos_theta(incoming));
            }
            else
            {
                return pt / (pr + pt) * cosine_hemisphere_PDF(abs_cos_theta(incoming));
            }
        }

        LOQUAT_CPU_GPU
        static constexpr const char* name() { return "DiffuseTransmissionBxDF"; }

        std::string to_string() const;

        LOQUAT_CPU_GPU
        void regularize() {}

        LOQUAT_CPU_GPU
        BxDFFlags get_flags() const
        {
            return ((R ? BxDFFlags::DiffuseReflection : BxDFFlags::Unset) |
                (T ? BxDFFlags::DiffuseTransmission : BxDFFlags::Unset));
        }

    private:
        SampledSpectrum R;
        SampledSpectrum T;
    };

    class DielectricBxDF {
    public:
        DielectricBxDF() = default;
        LOQUAT_CPU_GPU
         DielectricBxDF(Float eta, TrowbridgeReitzDistribution mfDistrib)
            : eta(eta)
            , mfDistrib(mfDistrib)
        {}

        LOQUAT_CPU_GPU
        BxDFFlags get_flags() const
        {
            BxDFFlags get_flags = (eta == 1) ? BxDFFlags::Transmission
                : (BxDFFlags::Reflection | BxDFFlags::Transmission);
            return get_flags |
                (mfDistrib.effectively_smooth() ? BxDFFlags::Specular : BxDFFlags::Glossy);
        }

        LOQUAT_CPU_GPU
        pstd::optional<BSDFSample> sample_f(
            Vec3f wo, Float uc, Point2f u, TransportMode mode,
            BxDFReflTransFlags sample_flags = BxDFReflTransFlags::All) const;

        LOQUAT_CPU_GPU
        SampledSpectrum f(Vec3f wo, Vec3f incoming, TransportMode mode) const;

        LOQUAT_CPU_GPU
        Float PDF(Vec3f wo, Vec3f incoming, TransportMode mode,
            BxDFReflTransFlags sample_flags = BxDFReflTransFlags::All) const;

        LOQUAT_CPU_GPU
        static constexpr const char* name() { return "DielectricBxDF"; }

        std::string to_string() const;

        LOQUAT_CPU_GPU
        void regularize() { mfDistrib.regularize(); }

    private:
        Float eta;
        TrowbridgeReitzDistribution mfDistrib;
    };

    class ThinDielectricBxDF {
    public:
        ThinDielectricBxDF() = default;
        LOQUAT_CPU_GPU
        ThinDielectricBxDF(Float eta)
            : eta(eta)
        {}

        LOQUAT_CPU_GPU
        SampledSpectrum f(Vec3f wo, Vec3f incoming, TransportMode mode) const
        {
            return SampledSpectrum(0);
        }

        LOQUAT_CPU_GPU
        pstd::optional<BSDFSample> sample_f(Vec3f wo, Float uc, Point2f u,
            TransportMode mode,
            BxDFReflTransFlags sample_flags) const
        {
            Float R = fr_dielectric(abs_cos_theta(wo), eta), T = 1 - R;
            // Compute _R_ and _T_ accounting for scattering between interfaces
            if (R < 1)
            {
                R += square(T) * R / (1 - square(R));
                T = 1 - R;
            }

            // Compute probabilities _pr_ and _pt_ for sampling reflection and transmission
            Float pr = R, pt = T;
            if (!(sample_flags & BxDFReflTransFlags::Reflection))
            {
                pr = 0;
            }
            if (!(sample_flags & BxDFReflTransFlags::Transmission))
            {
                pt = 0;
            }
            if (pr == 0 && pt == 0)
            {
                return {};
            }

            if (uc < pr / (pr + pt))
            {
                // Sample perfect specular dielectric BRDF
                Vec3f incoming(-wo.x, -wo.y, wo.z);
                SampledSpectrum fr(R / abs_cos_theta(incoming));
                return BSDFSample(fr, incoming, pr / (pr + pt), BxDFFlags::SpecularReflection);

            }
            else
            {
                // Sample perfect specular transmission at thin dielectric interface
                Vec3f incoming = -wo;
                SampledSpectrum ft(T / abs_cos_theta(incoming));
                return BSDFSample(ft, incoming, pt / (pr + pt), BxDFFlags::SpecularTransmission);
            }
        }

        LOQUAT_CPU_GPU
        Float PDF(Vec3f wo, Vec3f incoming, TransportMode mode,
            BxDFReflTransFlags sample_flags) const
        {
            return 0;
        }

        LOQUAT_CPU_GPU
        static constexpr const char* name() { return "ThinDielectricBxDF"; }

        std::string to_string() const;

        LOQUAT_CPU_GPU
        void regularize()
        { 
            //TODO(ches) complete this
        }

        LOQUAT_CPU_GPU
        BxDFFlags get_flags() const
        {
            return (BxDFFlags::Reflection | BxDFFlags::Transmission | BxDFFlags::Specular);
        }

    private:
        Float eta;
    };

    class ConductorBxDF {
    public:
        ConductorBxDF() = default;
        LOQUAT_CPU_GPU
        ConductorBxDF(const TrowbridgeReitzDistribution& mfDistrib,
            SampledSpectrum eta, SampledSpectrum k)
            : mfDistrib(mfDistrib)
            , eta(eta)
            , k(k)
        {}

        LOQUAT_CPU_GPU
        BxDFFlags get_flags() const
        {
            return mfDistrib.effectively_smooth() ? BxDFFlags::SpecularReflection
                : BxDFFlags::GlossyReflection;
        }

        LOQUAT_CPU_GPU
        pstd::optional<BSDFSample> sample_f(
            Vec3f wo, Float uc, Point2f u, TransportMode mode,
            BxDFReflTransFlags sample_flags = BxDFReflTransFlags::All) const
        {
            if (!(sample_flags & BxDFReflTransFlags::Reflection))
            {
                return {};
            }
            if (mfDistrib.effectively_smooth()) {
                // Sample perfect specular conductor BRDF
                Vec3f incoming(-wo.x, -wo.y, wo.z);
                SampledSpectrum f = fr_complex(abs_cos_theta(incoming), eta, k) / abs_cos_theta(incoming);
                return BSDFSample(f, incoming, 1, BxDFFlags::SpecularReflection);
            }
            // Sample rough conductor BRDF
            // Sample microfacet normal $\wm$ and reflected direction $\wi$
            if (wo.z == 0)
            {
                return {};
            }
            Vec3f wm = mfDistrib.sample_wm(wo, u);
            Vec3f incoming = reflect(wo, wm);
            if (!same_hemisphere(wo, incoming))
            {
                return {};
            }

            // Compute PDF of _wi_ for microfacet reflection
            Float pdf = mfDistrib.PDF(wo, wm) / (4 * absolute_dot(wo, wm));

            Float cosTheta_o = abs_cos_theta(wo), cosTheta_i = abs_cos_theta(incoming);
            if (cosTheta_i == 0 || cosTheta_o == 0)
            {
                return {};
            }
            // Evaluate Fresnel factor _F_ for conductor BRDF
            SampledSpectrum F = fr_complex(absolute_dot(wo, wm), eta, k);

            SampledSpectrum f =
                mfDistrib.d(wm) * F * mfDistrib.G(wo, incoming) / (4 * cosTheta_i * cosTheta_o);
            return BSDFSample(f, incoming, pdf, BxDFFlags::GlossyReflection);
        }

        LOQUAT_CPU_GPU
        SampledSpectrum f(Vec3f wo, Vec3f incoming, TransportMode mode) const
        {
            if (!same_hemisphere(wo, incoming))
            {
                return {};
            }
            if (mfDistrib.effectively_smooth())
            {
                return {};
            }
            // Evaluate rough conductor BRDF
            // Compute cosines and $\wm$ for conductor BRDF
            Float cosTheta_o = abs_cos_theta(wo), cosTheta_i = abs_cos_theta(incoming);
            if (cosTheta_i == 0 || cosTheta_o == 0)
            {
                return {};
            }
            Vec3f wm = incoming + wo;
            if (length_squared(wm) == 0)
            {
                return {};
            }
            wm = normalize(wm);

            // Evaluate Fresnel factor _F_ for conductor BRDF
            SampledSpectrum F = fr_complex(absolute_dot(wo, wm), eta, k);

            return mfDistrib.d(wm) * F * mfDistrib.G(wo, incoming) / (4 * cosTheta_i * cosTheta_o);
        }

        LOQUAT_CPU_GPU
        Float PDF(Vec3f wo, Vec3f incoming, TransportMode mode,
            BxDFReflTransFlags sample_flags) const
        {
            if (!(sample_flags & BxDFReflTransFlags::Reflection))
            {
                return 0;
            }
            if (!same_hemisphere(wo, incoming))
            {
                return 0;
            }
            if (mfDistrib.effectively_smooth())
            {
                return 0;
            }
            // Evaluate sampling PDF of rough conductor BRDF
            Vec3f wm = wo + incoming;
            //LOG_ASSERT(length_squared(wm) == 0);
            if (length_squared(wm) == 0)
            {
                return 0;
            }
            wm = face_forward(normalize(wm), Normal3f(0, 0, 1));
            return mfDistrib.PDF(wo, wm) / (4 * absolute_dot(wo, wm));
        }

        LOQUAT_CPU_GPU
        static constexpr const char* name() { return "ConductorBxDF"; }
        std::string to_string() const;

        LOQUAT_CPU_GPU
        void regularize() { mfDistrib.regularize(); }

    private:
        TrowbridgeReitzDistribution mfDistrib;
        SampledSpectrum eta;
        SampledSpectrum k;
    };

    template <typename TopBxDF, typename BottomBxDF>
    class TopOrBottomBxDF
    {
    public:
        TopOrBottomBxDF() = default;
        LOQUAT_CPU_GPU
        TopOrBottomBxDF& operator=(const TopBxDF* t)
        {
            top = t;
            bottom = nullptr;
            return *this;
        }
        LOQUAT_CPU_GPU
        TopOrBottomBxDF& operator=(const BottomBxDF* b)
        {
            bottom = b;
            top = nullptr;
            return *this;
        }

        LOQUAT_CPU_GPU
        SampledSpectrum f(Vec3f wo, Vec3f incoming, TransportMode mode) const
        {
            return top ? top->f(wo, incoming, mode) : bottom->f(wo, incoming, mode);
        }

        LOQUAT_CPU_GPU
        pstd::optional<BSDFSample> sample_f(
            Vec3f wo, Float uc, Point2f u, TransportMode mode,
            BxDFReflTransFlags sample_flags = BxDFReflTransFlags::All) const
        {
            return top ? top->sample_f(wo, uc, u, mode, sample_flags)
                : bottom->sample_f(wo, uc, u, mode, sample_flags);
        }

        LOQUAT_CPU_GPU
        Float PDF(Vec3f wo, Vec3f incoming, TransportMode mode,
            BxDFReflTransFlags sample_flags = BxDFReflTransFlags::All) const
        {
            return top ? top->PDF(wo, incoming, mode, sample_flags)
                : bottom->PDF(wo, incoming, mode, sample_flags);
        }

        LOQUAT_CPU_GPU
        BxDFFlags get_flags() const { return top ? top->get_flags() : bottom->get_flags(); }

    private:
        const TopBxDF* top = nullptr;
        const BottomBxDF* bottom = nullptr;
    };

    template <typename TopBxDF, typename BottomBxDF, bool twoSided>
    class LayeredBxDF
    {
    public:
        LayeredBxDF() = default;
        LOQUAT_CPU_GPU
        LayeredBxDF(TopBxDF top, BottomBxDF bottom, Float thickness,
            const SampledSpectrum& albedo, Float g, int max_depth, int sample_count)
            : top(top)
            , bottom(bottom)
            , thickness(std::max(thickness, std::numeric_limits<Float>::min()))
            , g(g)
            , albedo(albedo)
            , max_depth(max_depth)
            , sample_count(sample_count)
        {}

        std::string to_string() const;

        LOQUAT_CPU_GPU
        void regularize()
        {
            top.regularize();
            bottom.regularize();
        }

        LOQUAT_CPU_GPU
        BxDFFlags get_flags() const
        {
            BxDFFlags topFlags = top.get_flags(), bottomFlags = bottom.get_flags();
            LOG_ASSERT(is_transmissive(topFlags) ||
                is_transmissive(bottomFlags));  // otherwise, why bother?

            BxDFFlags get_flags = BxDFFlags::Reflection;
            if (is_specular(topFlags))
            {
                get_flags = get_flags | BxDFFlags::Specular;
            }

            if (is_diffuse(topFlags) || is_diffuse(bottomFlags) || albedo)
            {
                get_flags = get_flags | BxDFFlags::Diffuse;
            }
            else if (is_glossy(topFlags) || is_glossy(bottomFlags))
            {
                get_flags = get_flags | BxDFFlags::Glossy;
            }

            if (is_transmissive(topFlags) && is_transmissive(bottomFlags))
            {
                get_flags = get_flags | BxDFFlags::Transmission;
            }

            return get_flags;
        }

        LOQUAT_CPU_GPU
        SampledSpectrum f(Vec3f wo, Vec3f incoming, TransportMode mode) const
        {
            SampledSpectrum f(0.0f);
            // Estimate _LayeredBxDF_ value _f_ using random sampling
            // Set _wo_ and _wi_ for layered BSDF evaluation
            if (twoSided&& wo.z < 0)
            {
                wo = -wo;
                incoming = -incoming;
            }

            // Determine entrance interface for layered BSDF
            TopOrBottomBxDF<TopBxDF, BottomBxDF> enterInterface;
            bool enteredTop = twoSided || wo.z > 0;
            if (enteredTop)
            {
                enterInterface = &top;
            }
            else
            {
                enterInterface = &bottom;
            }

            // Determine exit interface and exit $z$ for layered BSDF
            TopOrBottomBxDF<TopBxDF, BottomBxDF> exitInterface, nonExitInterface;
            if (same_hemisphere(wo, incoming) ^ enteredTop)
            {
                exitInterface = &bottom;
                nonExitInterface = &top;
            }
            else
            {
                exitInterface = &top;
                nonExitInterface = &bottom;
            }
            Float exitZ = (same_hemisphere(wo, incoming) ^ enteredTop) ? 0 : thickness;

            // Account for reflection at the entrance interface
            if (same_hemisphere(wo, incoming))
            {
                f = sample_count * enterInterface.f(wo, incoming, mode);
            }

            // Declare _RNG_ for layered BSDF evaluation
            RNG rng(hash(get_options().seed, wo), hash(incoming));
            auto r = [&rng]() {
                return std::min<Float>(rng.uniform<Float>(), ONE_MINUS_EPSILON);
                };

            for (int s = 0; s < sample_count; ++s)
            {
                // Sample random walk through layers to estimate BSDF value
                // Sample transmission direction through entrance interface
                Float uc = r();
                pstd::optional<BSDFSample> wos = enterInterface.sample_f(
                    wo, uc, Point2f(r(), r()), mode, BxDFReflTransFlags::Transmission);
                if (!wos || !wos->spectrum || wos->pdf == 0 || wos->incoming.z == 0)
                {
                    continue;
                }

                // Sample BSDF for virtual light from _wi_
                uc = r();
                pstd::optional<BSDFSample> wis = exitInterface.sample_f(
                    incoming, uc, Point2f(r(), r()), !mode, BxDFReflTransFlags::Transmission);
                if (!wis || !wis->spectrum || wis->pdf == 0 || wis->incoming.z == 0)
                {
                    continue;
                }

                // Declare state for random walk through BSDF layers
                SampledSpectrum beta = wos->spectrum * abs_cos_theta(wos->incoming) / wos->pdf;
                Float z = enteredTop ? thickness : 0;
                Vec3f w = wos->incoming;
                HGPhaseFunction phase(g);

                for (int depth = 0; depth < max_depth; ++depth)
                {
                    // Sample next event for layered BSDF evaluation random walk

                    LOG_INFO(std::format("beta: {} {} {} {}, w: {} {} {}, f: {} {} {} {}\n",
                        beta[0], beta[1], beta[2], beta[3], w.x, w.y, w.z,
                        f[0], f[1], f[2], f[3]));
                    // Possibly terminate layered BSDF random walk with Russian roulette
                    if (depth > 3 && beta.max_component_value() < 0.25f) {
                        Float q = std::max<Float>(0, 1 - beta.max_component_value());
                        if (r() < q)
                        {
                            break;
                        }
                        beta /= 1 - q;
                        LOG_INFO(std::format("After RR with q = {}, beta: {} {} {} {}\n", q, beta[0],
                            beta[1], beta[2], beta[3]));
                    }

                    // Account for media between layers and possibly scatter
                    if (!albedo)
                    {
                        // Advance to next layer boundary and update _beta_ for transmittance
                        z = (z == thickness) ? 0 : thickness;
                        beta *= tr(thickness, w);

                    }
                    else
                    {
                        // Sample medium scattering for layered BSDF evaluation
                        Float sigma_t = 1;
                        Float dz = sample_exponential(r(), sigma_t / std::abs(w.z));
                        Float zp = w.z > 0 ? (z + dz) : (z - dz);
                        //LOG_ASSERT(z == zp);
                        if (z == zp)
                        {
                            continue;
                        }
                        if (0 < zp && zp < thickness)
                        {
                            // Handle scattering event in layered BSDF medium
                            // Account for scattering through _exitInterface_ using _wis_
                            Float wt = 1;
                            if (!is_specular(exitInterface.get_flags()))
                            {
                                wt = power_heuristic(1, wis->pdf, 1, phase.PDF(-w, -wis->incoming));
                            }
                            f += beta * albedo * phase.phase(-w, -wis->incoming) * wt *
                                tr(zp - exitZ, wis->incoming) * wis->spectrum / wis->pdf;

                            // Sample phase function and update layered path state
                            Point2f u{ r(), r() };
                            pstd::optional<PhaseFunctionSample> ps = phase.sample_phase(-w, u);
                            if (!ps || ps->pdf == 0 || ps->incoming.z == 0)
                            {
                                continue;
                            }
                            beta *= albedo * ps->probability / ps->pdf;
                            w = ps->incoming;
                            z = zp;

                            // Possibly account for scattering through _exitInterface_
                            if (((z < exitZ && w.z > 0) || (z > exitZ && w.z < 0)) &&
                                !is_specular(exitInterface.get_flags())) {
                                // Account for scattering through _exitInterface_
                                SampledSpectrum fExit = exitInterface.f(-w, incoming, mode);
                                if (fExit) {
                                    Float exitPDF = exitInterface.PDF(
                                        -w, incoming, mode, BxDFReflTransFlags::Transmission);
                                    Float wt = power_heuristic(1, ps->pdf, 1, exitPDF);
                                    f += beta * tr(zp - exitZ, ps->incoming) * fExit * wt;
                                }
                            }

                            continue;
                        }
                        z = clamp(zp, 0, thickness);
                    }

                    // Account for scattering at appropriate interface
                    if (z == exitZ)
                    {
                        // Account for reflection at _exitInterface_
                        Float uc = r();
                        pstd::optional<BSDFSample> bs = exitInterface.sample_f(
                            -w, uc, Point2f(r(), r()), mode, BxDFReflTransFlags::Reflection);
                        if (!bs || !bs->spectrum || bs->pdf == 0 || bs->incoming.z == 0)
                            break;
                        beta *= bs->spectrum * abs_cos_theta(bs->incoming) / bs->pdf;
                        w = bs->incoming;

                    }
                    else
                    {
                        // Account for scattering at _nonExitInterface_
                        if (!is_specular(nonExitInterface.get_flags()))
                        {
                            // Add NEE contribution along presampled _wis_ direction
                            Float wt = 1;
                            if (!is_specular(exitInterface.get_flags()))
                            {
                                wt = power_heuristic(1, wis->pdf, 1,
                                    nonExitInterface.PDF(-w, -wis->incoming, mode));
                            }
                            f += beta * nonExitInterface.f(-w, -wis->incoming, mode) *
                                abs_cos_theta(wis->incoming) * wt * tr(thickness, wis->incoming) * wis->spectrum /
                                wis->pdf;
                        }
                        // Sample new direction using BSDF at _nonExitInterface_
                        Float uc = r();
                        Point2f u(r(), r());
                        pstd::optional<BSDFSample> bs = nonExitInterface.sample_f(
                            -w, uc, u, mode, BxDFReflTransFlags::Reflection);
                        if (!bs || !bs->spectrum || bs->pdf == 0 || bs->incoming.z == 0)
                        {
                            break;
                        }
                        beta *= bs->spectrum * abs_cos_theta(bs->incoming) / bs->pdf;
                        w = bs->incoming;

                        if (!is_specular(exitInterface.get_flags()))
                        {
                            // Add NEE contribution along direction from BSDF sample
                            SampledSpectrum fExit = exitInterface.f(-w, incoming, mode);
                            if (fExit)
                            {
                                Float wt = 1;
                                if (!is_specular(nonExitInterface.get_flags()))
                                {
                                    Float exitPDF = exitInterface.PDF(
                                        -w, incoming, mode, BxDFReflTransFlags::Transmission);
                                    wt = power_heuristic(1, bs->pdf, 1, exitPDF);
                                }
                                f += beta * tr(thickness, bs->incoming) * fExit * wt;
                            }
                        }
                    }
                }
            }

            return f / sample_count;
        }

        LOQUAT_CPU_GPU
        pstd::optional<BSDFSample> sample_f(
            Vec3f wo, Float uc, Point2f u, TransportMode mode,
            BxDFReflTransFlags sample_flags = BxDFReflTransFlags::All) const
        {
            LOG_ASSERT(sample_flags == BxDFReflTransFlags::All);  // for now
            // Set _wo_ for layered BSDF sampling
            bool flipWi = false;
            if (twoSided&& wo.z < 0)
            {
                wo = -wo;
                flipWi = true;
            }

            // Sample BSDF at entrance interface to get initial direction _w_
            bool enteredTop = twoSided || wo.z > 0;
            pstd::optional<BSDFSample> bs =
                enteredTop ? top.sample_f(wo, uc, u, mode) : bottom.sample_f(wo, uc, u, mode);
            if (!bs || !bs->spectrum || bs->pdf == 0 || bs->incoming.z == 0)
            {
                return {};
            }
            if (bs->is_reflection())
            {
                if (flipWi)
                {
                    bs->incoming = -bs->incoming;
                }
                bs->pdf_is_proportional = true;
                return bs;
            }
            Vec3f w = bs->incoming;
            bool specularPath = bs->is_specular();

            // Declare _RNG_ for layered BSDF sampling
            RNG rng(hash(get_options().seed, wo), hash(uc, u));
            auto r = [&rng]() {
                return std::min<Float>(rng.uniform<Float>(), ONE_MINUS_EPSILON);
                };

            // Declare common variables for layered BSDF sampling
            SampledSpectrum f = bs->spectrum * abs_cos_theta(bs->incoming);
            Float pdf = bs->pdf;
            Float z = enteredTop ? thickness : 0;
            HGPhaseFunction phase(g);

            for (int depth = 0; depth < max_depth; ++depth)
            {
                // Follow random walk through layers to sample layered BSDF
                // Possibly terminate layered BSDF sampling with Russian Roulette
                Float rrBeta = f.max_component_value() / pdf;
                if (depth > 3 && rrBeta < 0.25f)
                {
                    Float q = std::max<Float>(0, 1 - rrBeta);
                    if (r() < q)
                    {
                        return {};
                    }
                    pdf *= 1 - q;
                }
                if (w.z == 0)
                {
                    return {};
                }

                if (albedo)
                {
                    // Sample potential scattering event in layered medium
                    Float sigma_t = 1;
                    Float dz = sample_exponential(r(), sigma_t / abs_cos_theta(w));
                    Float zp = w.z > 0 ? (z + dz) : (z - dz);
                    //LOG_ASSERT(zp == z);
                    if (zp == z)
                    {
                        return {};
                    }
                    if (0 < zp && zp < thickness)
                    {
                        // Update path state for valid scattering event between interfaces
                        pstd::optional<PhaseFunctionSample> ps =
                            phase.sample_phase(-w, Point2f(r(), r()));
                        if (!ps || ps->pdf == 0 || ps->incoming.z == 0)
                        {
                            return {};
                        }
                        f *= albedo * ps->probability;
                        pdf *= ps->pdf;
                        specularPath = false;
                        w = ps->incoming;
                        z = zp;

                        continue;
                    }
                    z = clamp(zp, 0, thickness);
                    if (z == 0)
                    {
                        LOG_ASSERT(w.z < 0);
                    }
                    else
                    {
                        LOG_ASSERT(w.z > 0);
                    }

                }
                else
                {
                    // Advance to the other layer interface
                    z = (z == thickness) ? 0 : thickness;
                    f *= tr(thickness, w);
                }
                // Initialize _interface_ for current interface surface
#ifdef interface  // That's enough out of you, Windows.
#undef interface
#endif
                TopOrBottomBxDF<TopBxDF, BottomBxDF> interface;
                if (z == 0)
                {
                    interface = &bottom;
                }
                else
                {
                    interface = &top;
                }

                // Sample interface BSDF to determine new path direction
                Float uc = r();
                Point2f u(r(), r());
                pstd::optional<BSDFSample> bs = interface.sample_f(-w, uc, u, mode);
                if (!bs || !bs->spectrum || bs->pdf == 0 || bs->incoming.z == 0)
                {
                    return {};
                }
                f *= bs->spectrum;
                pdf *= bs->pdf;
                specularPath &= bs->is_specular();
                w = bs->incoming;

                // Return _BSDFSample_ if path has left the layers
                if (bs->is_transmissive())
                {
                    BxDFFlags get_flags = same_hemisphere(wo, w) ? BxDFFlags::Reflection
                        : BxDFFlags::Transmission;
                    get_flags |= specularPath ? BxDFFlags::Specular : BxDFFlags::Glossy;
                    if (flipWi)
                        w = -w;
                    return BSDFSample(f, w, pdf, get_flags, 1.f, true);
                }

                // Scale _f_ by cosine term after scattering at the interface
                f *= abs_cos_theta(bs->incoming);
            }
            return {};
        }

        LOQUAT_CPU_GPU
        Float PDF(Vec3f wo, Vec3f incoming, TransportMode mode,
            BxDFReflTransFlags sample_flags = BxDFReflTransFlags::All) const
        {
            LOG_ASSERT(sample_flags == BxDFReflTransFlags::All);  // for now
            // Set _wo_ and _wi_ for layered BSDF evaluation
            if (twoSided&& wo.z < 0)
            {
                wo = -wo;
                incoming = -incoming;
            }

            // Declare _RNG_ for layered PDF evaluation
            RNG rng(hash(get_options().seed, incoming), hash(wo));
            auto r = [&rng]() {
                return std::min<Float>(rng.uniform<Float>(), ONE_MINUS_EPSILON);
                };

            // Update _pdfSum_ for reflection at the entrance layer
            bool enteredTop = twoSided || wo.z > 0;
            Float pdfSum = 0;
            if (same_hemisphere(wo, incoming))
            {
                auto reflFlag = BxDFReflTransFlags::Reflection;
                pdfSum += enteredTop ? sample_count * top.PDF(wo, incoming, mode, reflFlag)
                    : sample_count * bottom.PDF(wo, incoming, mode, reflFlag);
            }

            for (int s = 0; s < sample_count; ++s)
            {
                // Evaluate layered BSDF PDF sample
                if (same_hemisphere(wo, incoming))
                {
                    // Evaluate TRT term for PDF estimate
                    TopOrBottomBxDF<TopBxDF, BottomBxDF> rInterface, tInterface;
                    if (enteredTop) {
                        rInterface = &bottom;
                        tInterface = &top;
                    }
                    else
                    {
                        rInterface = &top;
                        tInterface = &bottom;
                    }
                    // Sample _tInterface_ to get direction into the layers
                    auto trans = BxDFReflTransFlags::Transmission;
                    pstd::optional<BSDFSample> wos, wis;
                    wos = tInterface.sample_f(wo, r(), { r(), r() }, mode, trans);
                    wis = tInterface.sample_f(incoming, r(), { r(), r() }, !mode, trans);

                    // Update _pdfSum_ accounting for TRT scattering events
                    if (wos && wos->spectrum && wos->pdf > 0 && wis && wis->spectrum && wis->pdf > 0)
                    {
                        if (!is_non_specular(tInterface.get_flags()))
                        {
                            pdfSum += rInterface.PDF(-wos->incoming, -wis->incoming, mode);
                        }
                        else
                        {
                            // Use multiple importance sampling to estimate PDF product
                            pstd::optional<BSDFSample> rs =
                                rInterface.sample_f(-wos->incoming, r(), { r(), r() }, mode);
                            if (rs && rs->spectrum && rs->pdf > 0)
                            {
                                if (!is_non_specular(rInterface.get_flags()))
                                {
                                    pdfSum += tInterface.PDF(-rs->incoming, incoming, mode);
                                }
                                else
                                {
                                    // Compute MIS-weighted estimate of Equation
                                    // (\ref{eq:pdf-triple-canceled-one})
                                    Float rPDF = rInterface.PDF(-wos->incoming, -wis->incoming, mode);
                                    Float wt = power_heuristic(1, wis->pdf, 1, rPDF);
                                    pdfSum += wt * rPDF;

                                    Float tPDF = tInterface.PDF(-rs->incoming, incoming, mode);
                                    wt = power_heuristic(1, rs->pdf, 1, tPDF);
                                    pdfSum += wt * tPDF;
                                }
                            }
                        }
                    }

                }
                else
                {
                    // Evaluate TT term for PDF estimate
                    TopOrBottomBxDF<TopBxDF, BottomBxDF> toInterface, tiInterface;
                    if (enteredTop)
                    {
                        toInterface = &top;
                        tiInterface = &bottom;
                    }
                    else
                    {
                        toInterface = &bottom;
                        tiInterface = &top;
                    }

                    Float uc = r();
                    Point2f u(r(), r());
                    pstd::optional<BSDFSample> wos = toInterface.sample_f(wo, uc, u, mode);
                    if (!wos || !wos->spectrum || wos->pdf == 0 || wos->incoming.z == 0 ||
                        wos->is_reflection())
                    {
                        continue;
                    }

                    uc = r();
                    u = Point2f(r(), r());
                    pstd::optional<BSDFSample> wis = tiInterface.sample_f(incoming, uc, u, !mode);
                    if (!wis || !wis->spectrum || wis->pdf == 0 || wis->incoming.z == 0 ||
                        wis->is_reflection())
                    {
                        continue;
                    }

                    if (is_specular(toInterface.get_flags()))
                    {
                        pdfSum += tiInterface.PDF(-wos->incoming, incoming, mode);
                    }
                    else if (is_specular(tiInterface.get_flags()))
                    {
                        pdfSum += toInterface.PDF(wo, -wis->incoming, mode);
                    }
                    else
                    {
                        pdfSum += (toInterface.PDF(wo, -wis->incoming, mode) +
                            tiInterface.PDF(-wos->incoming, incoming, mode)) /
                            2;
                    }
                }
            }
            // Return mixture of PDF estimate and constant PDF
            return lerp(0.9f, 1 / (4 * PI), pdfSum / sample_count);
        }

    private:
        LOQUAT_CPU_GPU
        static Float tr(Float dz, Vec3f w)
        {
            if (std::abs(dz) <= std::numeric_limits<Float>::min())
            {
                return 1;
            }
            return fast_exp(-std::abs(dz / w.z));
        }

        TopBxDF top;
        BottomBxDF bottom;
        Float thickness;
        Float g;
        SampledSpectrum albedo;
        int max_depth;
        int sample_count;
    };

    class CoatedDiffuseBxDF : public LayeredBxDF<DielectricBxDF, DiffuseBxDF, true>
    {
    public:
        using LayeredBxDF::LayeredBxDF;
        LOQUAT_CPU_GPU
        static constexpr const char* name() { return "CoatedDiffuseBxDF"; }
    };

    class CoatedConductorBxDF : public LayeredBxDF<DielectricBxDF, ConductorBxDF, true>
    {
    public:
        LOQUAT_CPU_GPU
        static constexpr const char* name() { return "CoatedConductorBxDF"; }
        using LayeredBxDF::LayeredBxDF;
    };

    class HairBxDF {
    public:
        HairBxDF() = default;
        LOQUAT_CPU_GPU
        HairBxDF(Float h, Float eta, const SampledSpectrum& sigma_a, Float beta_m,
            Float beta_n, Float alpha);
        LOQUAT_CPU_GPU
        SampledSpectrum f(Vec3f wo, Vec3f incoming, TransportMode mode) const;
        LOQUAT_CPU_GPU
        pstd::optional<BSDFSample> sample_f(Vec3f wo, Float uc, Point2f u,
            TransportMode mode,
            BxDFReflTransFlags sample_flags) const;
        LOQUAT_CPU_GPU
        Float PDF(Vec3f wo, Vec3f incoming, TransportMode mode,
            BxDFReflTransFlags sample_flags) const;

        LOQUAT_CPU_GPU
        void regularize() {}

        LOQUAT_CPU_GPU
        static constexpr const char* name() { return "HairBxDF"; }
        std::string to_string() const;

        LOQUAT_CPU_GPU
        BxDFFlags get_flags() const { return BxDFFlags::GlossyReflection; }

        LOQUAT_CPU_GPU
        static RGBUnboundedSpectrum sigma_a_from_concentration(Float ce, Float cp);
        LOQUAT_CPU_GPU
        static SampledSpectrum sigma_a_from_reflectance(const SampledSpectrum& c, Float beta_n,
            const SampledWavelengths& lambda);

    private:
        static constexpr int pMax = 3;

        LOQUAT_CPU_GPU
        static Float mp(Float cosTheta_i, Float cosTheta_o, Float sinTheta_i,
            Float sinTheta_o, Float v)
        {
            Float a = cosTheta_i * cosTheta_o / v, b = sinTheta_i * sinTheta_o / v;
            Float mp = (v <= .1)
                ? (fast_exp(log_i0(a) - b - 1 / v + 0.6931f + std::log(1 / (2 * v))))
                : (fast_exp(-b) * i0(a)) / (std::sinh(1 / v) * 2 * v);
            LOG_ASSERT(!is_inf(mp) && !is_NaN(mp));
            return mp;
        }

        LOQUAT_CPU_GPU
        static pstd::array<SampledSpectrum, pMax + 1> ap(Float cosTheta_o,
            Float eta, Float h, SampledSpectrum T)
        {
            pstd::array<SampledSpectrum, pMax + 1> ap;
            // Compute $p=0$ attenuation at initial cylinder intersection
            Float cosGamma_o = safe_square_root(1 - square(h));
            Float cosTheta = cosTheta_o * cosGamma_o;
            Float f = fr_dielectric(cosTheta, eta);
            ap[0] = SampledSpectrum(f);

            // Compute $p=1$ attenuation term
            ap[1] = square(1 - f) * T;

            // Compute attenuation terms up to $p=_pMax_$
            for (int p = 2; p < pMax; ++p)
            {
                ap[p] = ap[p - 1] * T * f;
            }

            // Compute attenuation term accounting for remaining orders of scattering
            if (1 - T * f)
            {
                ap[pMax] = ap[pMax - 1] * f * T / (1 - T * f);
            }

            return ap;
        }

        LOQUAT_CPU_GPU
        static inline Float calc_phi(int p, Float gamma_o, Float gamma_t)
        {
            return 2 * p * gamma_t - 2 * gamma_o + p * PI;
        }

        LOQUAT_CPU_GPU
        static inline Float np(Float phi, int p, Float s, Float gamma_o,
            Float gamma_t)
        {
            Float dphi = phi - calc_phi(p, gamma_o, gamma_t);
            // Remap _dphi_ to $[-\pi,\pi]$
            while (dphi > PI)
            {
                dphi -= 2 * PI;
            }
            while (dphi < -PI)
            {
                dphi += 2 * PI;
            }

            return trimmed_logistic(dphi, s, -PI, PI);
        }

        LOQUAT_CPU_GPU
        pstd::array<Float, pMax + 1> ap_PDF(Float cosTheta_o) const;

        // HairBxDF Private Members
        Float h;
        Float eta;
        SampledSpectrum sigma_a;
        Float beta_m;
        Float beta_n;
        Float v[pMax + 1];
        Float s;
        Float sin2kAlpha[pMax];
        Float cos2kAlpha[pMax];
    };

    class MeasuredBxDF {
    public:
        MeasuredBxDF() = default;
        LOQUAT_CPU_GPU
        MeasuredBxDF(const MeasuredBxDFData* brdf, const SampledWavelengths& lambda)
            : brdf(brdf)
            , lambda(lambda)
        {}

        static MeasuredBxDFData* BRDF_data_from_file(const std::string& filename,
            Allocator alloc);

        LOQUAT_CPU_GPU
        SampledSpectrum f(Vec3f wo, Vec3f incoming, TransportMode mode) const;

        LOQUAT_CPU_GPU
        pstd::optional<BSDFSample> sample_f(Vec3f wo, Float uc, Point2f u,
            TransportMode mode,
            BxDFReflTransFlags sample_flags) const;
        LOQUAT_CPU_GPU
        Float PDF(Vec3f wo, Vec3f incoming, TransportMode mode,
            BxDFReflTransFlags sample_flags) const;

        LOQUAT_CPU_GPU
        void regularize() {}

        LOQUAT_CPU_GPU
        static constexpr const char* name() { return "MeasuredBxDF"; }

        std::string to_string() const;

        LOQUAT_CPU_GPU
        BxDFFlags get_flags() const { return (BxDFFlags::Reflection | BxDFFlags::Glossy); }

    private:
        LOQUAT_CPU_GPU
        static Float theta2u(Float theta) { return std::sqrt(theta * (2 / PI)); }
        LOQUAT_CPU_GPU
        static Float phi2u(Float phi) { return phi * (1 / (2 * PI)) + .5f; }

        LOQUAT_CPU_GPU
        static Float u2theta(Float u) { return square(u) * (PI / 2.f); }
        LOQUAT_CPU_GPU
        static Float u2phi(Float u) { return (2.f * u - 1.f) * PI; }

        const MeasuredBxDFData* brdf;
        SampledWavelengths lambda;
    };

    class NormalizedFresnelBxDF {
    public:
        NormalizedFresnelBxDF() = default;
        LOQUAT_CPU_GPU
        NormalizedFresnelBxDF(Float eta) : eta(eta) {}

        LOQUAT_CPU_GPU
        BSDFSample sample_f(Vec3f wo, Float uc, Point2f u, TransportMode mode,
            BxDFReflTransFlags sample_flags) const
        {
            if (!(sample_flags & BxDFReflTransFlags::Reflection))
            {
                return {};
            }

            // Cosine-sample the hemisphere, flipping the direction if necessary
            Vec3f incoming = sample_cosine_hemisphere(u);
            if (wo.z < 0)
            {
                incoming.z *= -1;
            }
            return BSDFSample(f(wo, incoming, mode), incoming, PDF(wo, incoming, mode, sample_flags),
                BxDFFlags::DiffuseReflection);
        }

        LOQUAT_CPU_GPU
        Float PDF(Vec3f wo, Vec3f incoming, TransportMode mode,
            BxDFReflTransFlags sample_flags) const
        {
            if (!(sample_flags & BxDFReflTransFlags::Reflection))
            {
                return 0;
            }
            return same_hemisphere(wo, incoming) ? abs_cos_theta(incoming) * INV_PI : 0;
        }

        LOQUAT_CPU_GPU
        void regularize() {}

        LOQUAT_CPU_GPU
        static constexpr const char* name() { return "NormalizedFresnelBxDF"; }

        std::string to_string() const;

        LOQUAT_CPU_GPU
        BxDFFlags get_flags() const
        {
            return BxDFFlags(BxDFFlags::Reflection | BxDFFlags::Diffuse);
        }

        LOQUAT_CPU_GPU
        SampledSpectrum f(Vec3f wo, Vec3f incoming, TransportMode mode) const
        {
            if (!same_hemisphere(wo, incoming))
            {
                return SampledSpectrum(0.0f);
            }
            // Compute $\Sw$ factor for BSSRDF value
            Float c = 1 - 2 * fresnel_moment_1(1 / eta);
            SampledSpectrum f((1 - fr_dielectric(cos_theta(incoming), eta)) / (c * PI));

            // Update BSSRDF transmission term to account for adjoint light transport
            if (mode == TransportMode::Radiance)
            {
                f *= square(eta);
            }

            return f;
        }

    private:
        Float eta;
    };

    LOQUAT_CPU_GPU
    inline SampledSpectrum BxDF::f(Vec3f wo, Vec3f incoming, TransportMode mode) const
    {
        auto f = [&](auto ptr) -> SampledSpectrum { return ptr->f(wo, incoming, mode); };
        return dispatch(f);
    }

    LOQUAT_CPU_GPU
    inline pstd::optional<BSDFSample> BxDF::sample_f(Vec3f wo, Float uc, Point2f u,
        TransportMode mode,
        BxDFReflTransFlags sample_flags) const
    {
        auto sample_f = [&](auto ptr) -> pstd::optional<BSDFSample> {
            return ptr->sample_f(wo, uc, u, mode, sample_flags);
            };
        return dispatch(sample_f);
    }

    LOQUAT_CPU_GPU
    inline Float BxDF::PDF(Vec3f wo, Vec3f incoming, TransportMode mode,
        BxDFReflTransFlags sample_flags) const
    {
        auto pdf = [&](auto ptr) { return ptr->PDF(wo, incoming, mode, sample_flags); };
        return dispatch(pdf);
    }

    LOQUAT_CPU_GPU
    inline BxDFFlags BxDF::get_flags() const
    {
        auto get_flags = [&](auto ptr) { return ptr->get_flags(); };
        return dispatch(get_flags);
    }

    LOQUAT_CPU_GPU
    inline void BxDF::regularize()
    {
        auto regularize = [&](auto ptr) { ptr->regularize(); };
        return dispatch(regularize);
    }

    extern template class LayeredBxDF<DielectricBxDF, DiffuseBxDF, true>;
    extern template class LayeredBxDF<DielectricBxDF, ConductorBxDF, true>;

}
