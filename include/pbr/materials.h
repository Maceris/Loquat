// pbrt is Copyright(c) 1998-2020 Matt Pharr, Wenzel Jakob, and Greg Humphreys.
// The pbrt source code is licensed under the Apache License, Version 2.0.
// SPDX: Apache-2.0

// This file has been modified from the original, original notice is above.

#pragma once

#include <memory>
#include <string>
#include <type_traits>

#include "main/loquat.h"

#include "pbr/base/bssrdf.h"
#include "pbr/base/material.h"
#include "pbr/bsdf.h"
#include "pbr/bssrdf.h"
#include "pbr/textures.h"
#include "pbr/math/transform.h"
#include "pbr/struct/interaction.h"
#include "pbr/util/spectrum.h"
#include "pbr/util/tagged_pointer.h"

namespace loquat
{

    struct MaterialEvalContext : public TextureEvalContext
    {
        MaterialEvalContext() = default;

        LOQUAT_CPU_GPU
        MaterialEvalContext(const SurfaceInteraction& si)
        : TextureEvalContext(si)
            , outgoing(si.outgoing)
            , normal(si.shading.normal)
            , dpdus(si.shading.dpdu)
        {}

        std::string to_string() const;

        Vec3f outgoing;
        Normal3f normal;
        Vec3f dpdus;
    };

    struct NormalBumpEvalContext
    {
        NormalBumpEvalContext() = default;

        LOQUAT_CPU_GPU
        NormalBumpEvalContext(const SurfaceInteraction& si)
            : p(si.p())
            , uv(si.uv)
            , normal(si.normal)
            , dudx(si.dudx)
            , dudy(si.dudy)
            , dvdx(si.dvdx)
            , dvdy(si.dvdy)
            , dpdx(si.dpdx)
            , dpdy(si.dpdy)
            , face_index(si.face_index) 
        {
            shading.normal = si.shading.normal;
            shading.dpdu = si.shading.dpdu;
            shading.dpdv = si.shading.dpdv;
            shading.dndu = si.shading.dndu;
            shading.dndv = si.shading.dndv;
        }
        std::string to_string() const;

        LOQUAT_CPU_GPU
        operator TextureEvalContext() const
        {
            return TextureEvalContext(p, dpdx, dpdy, normal, uv, dudx, dudy,
                dvdx, dvdy, face_index);
        }

        Point3f p;
        Point2f uv;
        Normal3f normal;
        struct {
            Normal3f normal;
            Vec3f dpdu;
            Vec3f dpdv;
            Normal3f dndu;
            Normal3f dndv;
        } shading;
        Float dudx = 0;
        Float dudy = 0;
        Float dvdx = 0;
        Float dvdy = 0;
        Vec3f dpdx;
        Vec3f dpdy;
        int face_index = 0;
    };

    inline LOQUAT_CPU_GPU void normal_map(const Image& normal_map,
        const NormalBumpEvalContext& ctx, Vec3f* dpdu,  Vec3f* dpdv)
    {
        // Get normalized normal vector from normal map
        WrapMode2D wrap(WrapMode::Repeat);
        Point2f uv(ctx.uv[0], 1 - ctx.uv[1]);
        Vec3f normal(2 * normal_map.bilerp_channel(uv, 0, wrap) - 1,
            2 * normal_map.bilerp_channel(uv, 1, wrap) - 1,
            2 * normal_map.bilerp_channel(uv, 2, wrap) - 1);
        normal = normalize(normal);

        // Transform tangent-space normal to rendering space
        Frame frame = Frame::from_xz(normalize(ctx.shading.dpdu), Vec3f(ctx.shading.normal));
        normal = frame.from_local(normal);

        // Find $\dpdu$ and $\dpdv$ that give shading normal
        Float ulen = length(ctx.shading.dpdu), vlen = length(ctx.shading.dpdv);
        *dpdu = normalize(gram_schmidt(ctx.shading.dpdu, normal)) * ulen;
        *dpdv = normalize(cross(normal, *dpdu)) * vlen;
    }

    // Bump Mapping Function Definitions
    template <typename TextureEvaluator>
    LOQUAT_CPU_GPU
    void bump_map(TextureEvaluator tex_eval, FloatTexture displacement,
        const NormalBumpEvalContext& ctx, Vec3f* dpdu,
        Vec3f* dpdv)
    {
        LOG_ASSERT(tex_eval.can_evaluate({ displacement }, {}));
        // Compute offset positions and evaluate displacement texture
        TextureEvalContext shiftedCtx = ctx;
        // Shift _shiftedCtx_ _du_ in the $u$ direction
        Float du = .5f * (std::abs(ctx.dudx) + std::abs(ctx.dudy));
        if (du == 0)
            du = .0005f;
        shiftedCtx.p = ctx.p + du * ctx.shading.dpdu;
        shiftedCtx.uv = ctx.uv + Vector2f(du, 0.f);

        Float uDisplace = tex_eval(displacement, shiftedCtx);
        // Shift _shiftedCtx_ _dv_ in the $v$ direction
        Float dv = .5f * (std::abs(ctx.dvdx) + std::abs(ctx.dvdy));
        if (dv == 0)
            dv = .0005f;
        shiftedCtx.p = ctx.p + dv * ctx.shading.dpdv;
        shiftedCtx.uv = ctx.uv + Vector2f(0.f, dv);

        Float vDisplace = tex_eval(displacement, shiftedCtx);
        Float displace = tex_eval(displacement, ctx);

        // Compute bump-mapped differential geometry
        *dpdu = ctx.shading.dpdu + (uDisplace - displace) / du * Vec3f(ctx.shading.normal) +
            displace * Vec3f(ctx.shading.dndu);
        *dpdv = ctx.shading.dpdv + (vDisplace - displace) / dv * Vec3f(ctx.shading.normal) +
            displace * Vec3f(ctx.shading.dndv);
    }

    class DielectricMaterial {
    public:
        using BxDF = DielectricBxDF;
        using BSSRDF = void;

        DielectricMaterial(FloatTexture u_roughness, FloatTexture v_roughness,
            Spectrum eta, FloatTexture displacement, Image* normal_map,
            bool remap_roughness)
            : normal_map(normal_map)
            , displacement(displacement)
            , u_roughness(u_roughness)
            , v_roughness(v_roughness)
            , eta(eta)
            , remap_roughness(remap_roughness)
        {}

        static const char* get_name() { return "DielectricMaterial"; }

        template <typename TextureEvaluator>
        LOQUAT_CPU_GPU bool can_evaluate_textures(TextureEvaluator tex_eval) const {
            return tex_eval.can_evaluate({ u_roughness, v_roughness }, {});
        }

        LOQUAT_CPU_GPU
            FloatTexture get_displacement() const { return displacement; }
        LOQUAT_CPU_GPU
            const Image* get_normal_map() const { return normal_map; }

        static DielectricMaterial* create(const TextureParameterDictionary& parameters,
            Image* normal_map, const FileLoc* loc,
            Allocator alloc);

        std::string to_string() const;

        template <typename TextureEvaluator>
        LOQUAT_CPU_GPU
        void get_BSSRDF(TextureEvaluator tex_eval, MaterialEvalContext ctx,
            SampledWavelengths& lambda) const
        {}

        LOQUAT_CPU_GPU
        static constexpr bool has_subsurface_scattering()
        {
            return false;
        }

        template <typename TextureEvaluator>
        LOQUAT_CPU_GPU
        DielectricBxDF get_BxDF(TextureEvaluator tex_eval,
            MaterialEvalContext ctx, SampledWavelengths& lambda) const
        {
            // Compute index of refraction for dielectric material
            Float sampledEta = eta(lambda[0]);
            if (!eta.template Is<ConstantSpectrum>())
                lambda.TerminateSecondary();
            // Handle edge case in case lambda[0] is beyond the wavelengths stored by the
            // Spectrum.
            if (sampledEta == 0)
                sampledEta = 1;

            // create microfacet distribution for dielectric material
            Float urough = tex_eval(u_roughness, ctx), vrough = tex_eval(v_roughness, ctx);
            if (remap_roughness) {
                urough = TrowbridgeReitzDistribution::RoughnessToAlpha(urough);
                vrough = TrowbridgeReitzDistribution::RoughnessToAlpha(vrough);
            }
            TrowbridgeReitzDistribution distrib(urough, vrough);

            // Return BSDF for dielectric material
            return DielectricBxDF(sampledEta, distrib);
        }

    private:
        Image* normal_map;
        FloatTexture displacement;
        FloatTexture u_roughness;
        FloatTexture v_roughness;
        bool remap_roughness;
        Spectrum eta;
    };

    class ThinDielectricMaterial
    {
    public:
        using BxDF = ThinDielectricBxDF;
        using BSSRDF = void;

        template <typename TextureEvaluator>
        LOQUAT_CPU_GPU
        bool can_evaluate_textures(TextureEvaluator tex_eval) const
        {
            return true;
        }

        template <typename TextureEvaluator>
        LOQUAT_CPU_GPU
        ThinDielectricBxDF get_BxDF(TextureEvaluator tex_eval,
            MaterialEvalContext ctx, SampledWavelengths& lambda) const
        {
            // Compute index of refraction for dielectric material
            Float sampledEta = eta(lambda[0]);
            if (!eta.template Is<ConstantSpectrum>())
                lambda.TerminateSecondary();
            // Handle edge case in case lambda[0] is beyond the wavelengths stored by the
            // Spectrum.
            if (sampledEta == 0)
                sampledEta = 1;

            // Return BxDF for _ThinDielectricMaterial_
            return ThinDielectricBxDF(sampledEta);
        }

        ThinDielectricMaterial(Spectrum eta, FloatTexture displacement, Image* normal_map)
            : displacement(displacement)
            , normal_map(normal_map)
            , eta(eta)
        {}

        static const char* get_name()
        {
            return "ThinDielectricMaterial";
        }

        LOQUAT_CPU_GPU
        FloatTexture get_displacement() const { return displacement; }
        LOQUAT_CPU_GPU
        const Image* get_normal_map() const { return normal_map; }

        static ThinDielectricMaterial* create(
            const TextureParameterDictionary& parameters,
            Image* normal_map, const FileLoc* loc,
            Allocator alloc);

        template <typename TextureEvaluator>
        LOQUAT_CPU_GPU
        void get_BSSRDF(TextureEvaluator tex_eval, MaterialEvalContext ctx,
            SampledWavelengths& lambda) const {}

        LOQUAT_CPU_GPU
        static constexpr bool has_subsurface_scattering()
        {
            return false;
        }

        std::string to_string() const;

    private:
        FloatTexture displacement;
        Image* normal_map;
        Spectrum eta;
    };

    class MixMaterial {
    public:
        using BxDF = void;
        using BSSRDF = void;

        MixMaterial(Material m[2], FloatTexture amount)
            : amount(amount)
        {
            materials[0] = m[0];
            materials[1] = m[1];
        }

        template <typename TextureEvaluator>
        LOQUAT_CPU_GPU
        Material choose_material(TextureEvaluator tex_eval,
            MaterialEvalContext ctx) const
        {
            Float amt = tex_eval(amount, ctx);
            if (amt <= 0)
                return materials[0];
            if (amt >= 1)
                return materials[1];
            Float u = HashFloat(ctx.p, ctx.outgoing, materials[0], materials[1]);
            return (amt < u) ? materials[0] : materials[1];
        }

        LOQUAT_CPU_GPU
        Material get_material(int i) const { return materials[i]; }

        static const char* get_name() { return "MixMaterial"; }

        LOQUAT_CPU_GPU
        FloatTexture get_displacement() const
        {
#ifndef LOQUAT_IS_GPU_CODE
            LOG_FATAL("Shouldn't be called");
#endif
            return nullptr;
        }

        LOQUAT_CPU_GPU
        const Image* get_normal_map() const
        {
#ifndef LOQUAT_IS_GPU_CODE
            LOG_FATAL("Shouldn't be called");
#endif
            return nullptr;
        }

        static MixMaterial* create(Material materials[2],
            const TextureParameterDictionary& parameters,
            const FileLoc* loc, Allocator alloc);

        template <typename TextureEvaluator>
        LOQUAT_CPU_GPU
        void get_BSSRDF(TextureEvaluator tex_eval, MaterialEvalContext ctx,
            SampledWavelengths& lambda) const
        {
#ifndef LOQUAT_IS_GPU_CODE
            LOG_FATAL("Shouldn't be called");
#endif
        }

        LOQUAT_CPU_GPU
        static constexpr bool has_subsurface_scattering() { return false; }

        std::string to_string() const;

        template <typename TextureEvaluator>
        LOQUAT_CPU_GPU
        bool can_evaluate_textures(TextureEvaluator tex_eval) const
        {
            return tex_eval.can_evaluate({ amount }, {});
        }

        template <typename TextureEvaluator>
        LOQUAT_CPU_GPU
        void get_BxDF(TextureEvaluator tex_eval,
            MaterialEvalContext ctx, SampledWavelengths& lambda) const
        {
#ifndef LOQUAT_IS_GPU_CODE
            LOG_FATAL("MixMaterial::get_BxDF() shouldn't be called");
#endif
        }

    private:
        FloatTexture amount;
        Material materials[2];
    };

    class HairMaterial
    {
    public:
        using BxDF = HairBxDF;
        using BSSRDF = void;

        HairMaterial(SpectrumTexture sigma_a, SpectrumTexture color, FloatTexture eumelanin,
            FloatTexture pheomelanin, FloatTexture eta, FloatTexture beta_m,
            FloatTexture beta_n, FloatTexture alpha)
            : sigma_a(sigma_a)
            , color(color)
            , eumelanin(eumelanin)
            , pheomelanin(pheomelanin)
            , eta(eta)
            , beta_m(beta_m)
            , beta_n(beta_n)
            , alpha(alpha)
        {}

        static const char* get_name() { return "HairMaterial"; }

        template <typename TextureEvaluator>
        LOQUAT_CPU_GPU
        bool can_evaluate_textures(TextureEvaluator tex_eval) const
        {
            return tex_eval.can_evaluate({ eumelanin, pheomelanin, eta, beta_m, beta_n, alpha },
                { sigma_a, color });
        }

        template <typename TextureEvaluator>
        LOQUAT_CPU_GPU
        HairBxDF get_BxDF(TextureEvaluator tex_eval, MaterialEvalContext ctx,
            SampledWavelengths& lambda) const
        {
            Float bm = std::max<Float>(1e-2, std::min<Float>(1.0, tex_eval(beta_m, ctx)));
            Float bn = std::max<Float>(1e-2, std::min<Float>(1.0, tex_eval(beta_n, ctx)));
            Float a = tex_eval(alpha, ctx);
            Float e = tex_eval(eta, ctx);

            SampledSpectrum sig_a;
            if (sigma_a)
            {
                sig_a = ClampZero(tex_eval(sigma_a, ctx, lambda));
            }
            else if (color)
            {
                SampledSpectrum c = clamp(tex_eval(color, ctx, lambda), 0, 1);
                sig_a = HairBxDF::SigmaAFromReflectance(c, bn, lambda);
            }
            else
            {
                LOG_ASSERT(eumelanin || pheomelanin);
                sig_a = HairBxDF::SigmaAFromConcentration(
                    std::max(Float(0), eumelanin ? tex_eval(eumelanin, ctx) : 0),
                    std::max(Float(0), pheomelanin ? tex_eval(pheomelanin, ctx) : 0))
                    .Sample(lambda);
            }

            // Offset along width
            Float h = -1 + 2 * ctx.uv[1];
            return HairBxDF(h, e, sig_a, bm, bn, a);
        }

        static HairMaterial* create(const TextureParameterDictionary& parameters,
            const FileLoc* loc, Allocator alloc);

        LOQUAT_CPU_GPU
        FloatTexture get_displacement() const { return nullptr; }
        LOQUAT_CPU_GPU
        const Image* get_normal_map() const { return nullptr; }

        template <typename TextureEvaluator>
        LOQUAT_CPU_GPU
        void get_BSSRDF(TextureEvaluator tex_eval, MaterialEvalContext ctx,
            SampledWavelengths& lambda) const
        {}

        LOQUAT_CPU_GPU
        static constexpr bool has_subsurface_scattering() { return false; }

        std::string to_string() const;

    private:
        SpectrumTexture sigma_a;
        SpectrumTexture color;
        /// <summary>
        /// Brown-black pigment.
        /// </summary>
        FloatTexture eumelanin;
        /// <summary>
        /// Red-yellow pigment.
        /// </summary>
        FloatTexture pheomelanin;
        FloatTexture eta;
        FloatTexture beta_m;
        FloatTexture beta_n;
        FloatTexture alpha;
    };

    class DiffuseMaterial {
    public:
        using BxDF = DiffuseBxDF;
        using BSSRDF = void;

        static const char* get_name() { return "DiffuseMaterial"; }

        LOQUAT_CPU_GPU
        FloatTexture get_displacement() const { return displacement; }
        LOQUAT_CPU_GPU
        const Image* get_normal_map() const { return normal_map; }

        static DiffuseMaterial* create(
            const TextureParameterDictionary& parameters,
            Image* normal_map, const FileLoc* loc, Allocator alloc);

        template <typename TextureEvaluator>
        LOQUAT_CPU_GPU
        void get_BSSRDF(TextureEvaluator tex_eval, MaterialEvalContext ctx,
            SampledWavelengths& lambda, void*) const
        {}

        LOQUAT_CPU_GPU
        static constexpr bool has_subsurface_scattering()
        {
            return false;
        }

        std::string to_string() const;

        DiffuseMaterial(SpectrumTexture reflectance, FloatTexture displacement,
            Image* normal_map)
            : normal_map(normal_map)
            , displacement(displacement)
            , reflectance(reflectance)
        {}

        template <typename TextureEvaluator>
        LOQUAT_CPU_GPU
        bool can_evaluate_textures(TextureEvaluator tex_eval) const
        {
            return tex_eval.can_evaluate({}, { reflectance });
        }

        template <typename TextureEvaluator>
        LOQUAT_CPU_GPU
        DiffuseBxDF get_BxDF(TextureEvaluator tex_eval,
            MaterialEvalContext ctx, SampledWavelengths& lambda) const
        {
            SampledSpectrum r = clamp(tex_eval(reflectance, ctx, lambda), 0, 1);
            return DiffuseBxDF(r);
        }

    private:
        Image* normal_map;
        FloatTexture displacement;
        SpectrumTexture reflectance;
    };

    class ConductorMaterial
    {
    public:
        using BxDF = ConductorBxDF;
        using BSSRDF = void;

        template <typename TextureEvaluator>
        LOQUAT_CPU_GPU
        bool can_evaluate_textures(TextureEvaluator tex_eval) const
        {
            return tex_eval.can_evaluate({ u_roughness, v_roughness }, { eta, k, reflectance });
        }

        template <typename TextureEvaluator>
        LOQUAT_CPU_GPU
        ConductorBxDF get_BxDF(TextureEvaluator tex_eval,
            MaterialEvalContext ctx, SampledWavelengths& lambda) const
        {
            // Return BSDF for _ConductorMaterial_
            Float uRough = tex_eval(u_roughness, ctx), vRough = tex_eval(v_roughness, ctx);
            if (remap_roughness)
            {
                uRough = TrowbridgeReitzDistribution::RoughnessToAlpha(uRough);
                vRough = TrowbridgeReitzDistribution::RoughnessToAlpha(vRough);
            }
            SampledSpectrum etas, ks;
            if (eta)
            {
                etas = tex_eval(eta, ctx, lambda);
                ks = tex_eval(k, ctx, lambda);
            }
            else
            {
                // Avoid r==0 NaN case...
                SampledSpectrum r = clamp(tex_eval(reflectance, ctx, lambda), 0, 0.9999);
                etas = SampledSpectrum(1.f);
                ks = 2 * Sqrt(r) / Sqrt(ClampZero(SampledSpectrum(1) - r));
            }
            TrowbridgeReitzDistribution distrib(uRough, vRough);
            return ConductorBxDF(distrib, etas, ks);
        }

        ConductorMaterial(SpectrumTexture eta, SpectrumTexture k,
            SpectrumTexture reflectance, FloatTexture u_roughness,
            FloatTexture v_roughness, FloatTexture displacement,
            Image* normal_map, bool remap_roughness)
            : displacement(displacement)
            , normal_map(normal_map)
            , eta(eta)
            , k(k)
            , reflectance(reflectance)
            , u_roughness(u_roughness)
            , v_roughness(v_roughness)
            , remap_roughness(remap_roughness)
        {}

        static const char* get_name()
        {
            return "ConductorMaterial";
        }

        LOQUAT_CPU_GPU
        FloatTexture get_displacement() const { return displacement; }
        LOQUAT_CPU_GPU
        const Image* get_normal_map() const { return normal_map; }

        static ConductorMaterial* create(
            const TextureParameterDictionary& parameters,
            Image* normal_map, const FileLoc* loc,
            Allocator alloc);

        template <typename TextureEvaluator>
        LOQUAT_CPU_GPU
        void get_BSSRDF(TextureEvaluator tex_eval, MaterialEvalContext ctx,
            SampledWavelengths& lambda) const
        {}

        LOQUAT_CPU_GPU
        static constexpr bool has_subsurface_scattering()
        {
            return false;
        }

        std::string to_string() const;

    private:
        FloatTexture displacement;
        Image* normal_map;
        SpectrumTexture eta;
        SpectrumTexture k;
        SpectrumTexture reflectance;
        FloatTexture u_roughness;
        FloatTexture v_roughness;
        bool remap_roughness;
    };

    class CoatedDiffuseMaterial {
    public:
        using BxDF = CoatedDiffuseBxDF;
        using BSSRDF = void;

        CoatedDiffuseMaterial(SpectrumTexture reflectance,
            FloatTexture u_roughness, FloatTexture v_roughness,
            FloatTexture thickness, SpectrumTexture albedo, FloatTexture g,
            Spectrum eta, FloatTexture displacement, Image* normal_map,
            bool remap_roughness, int max_depth, int sample_count)
            : displacement(displacement)
            , normal_map(normal_map)
            , reflectance(reflectance)
            , u_roughness(u_roughness)
            , v_roughness(v_roughness)
            , thickness(thickness)
            , albedo(albedo)
            , g(g)
            , eta(eta)
            , remap_roughness(remap_roughness)
            , max_depth(max_depth)
            , sample_count(sample_count)
        {}

        static const char* get_name()
        {
            return "CoatedDiffuseMaterial";
        }

        template <typename TextureEvaluator>
        LOQUAT_CPU_GPU
        bool can_evaluate_textures(TextureEvaluator tex_eval) const
        {
            return tex_eval.can_evaluate({ u_roughness, v_roughness, thickness, g },
                { reflectance, albedo });
        }

        template <typename TextureEvaluator>
        
        LOQUAT_CPU_GPU
        CoatedDiffuseBxDF get_BxDF(TextureEvaluator tex_eval,
            const MaterialEvalContext& ctx,
            SampledWavelengths& lambda) const;

        LOQUAT_CPU_GPU
        FloatTexture get_displacement() const
        {
            return displacement;
        }
        
        LOQUAT_CPU_GPU
        const Image* get_normal_map() const
        {
            return normal_map;
        }

        static CoatedDiffuseMaterial* create(const TextureParameterDictionary& parameters,
            Image* normal_map, const FileLoc* loc,
            Allocator alloc);

        template <typename TextureEvaluator>
        LOQUAT_CPU_GPU
        void get_BSSRDF(TextureEvaluator tex_eval,
            const MaterialEvalContext& ctx, SampledWavelengths& lambda) const
        {}

        LOQUAT_CPU_GPU
        static constexpr bool has_subsurface_scattering()
        {
            return false;
        }

        std::string to_string() const;

    private:
        FloatTexture displacement;
        Image* normal_map;
        SpectrumTexture reflectance;
        SpectrumTexture albedo;
        FloatTexture u_roughness;
        FloatTexture v_roughness;
        FloatTexture thickness;
        FloatTexture g;
        Spectrum eta;
        bool remap_roughness;
        int max_depth;
        int sample_count;
    };

    class CoatedConductorMaterial
    {
    public:
        using BxDF = CoatedConductorBxDF;
        using BSSRDF = void;

        CoatedConductorMaterial(FloatTexture interface_u_roughness,
            FloatTexture interface_v_roughness, FloatTexture thickness,
            Spectrum interface_eta, FloatTexture g, SpectrumTexture albedo,
            FloatTexture conductor_u_roughness,
            FloatTexture conductor_v_rougness,
            SpectrumTexture conductor_eta, SpectrumTexture k,
            SpectrumTexture reflectance, FloatTexture displacement,
            Image* normal_map, bool remap_roughness, int max_depth,
            int sample_count)
            : displacement(displacement)
            , normal_map(normal_map)
            , interface_u_roughness(interface_u_roughness)
            , interface_v_roughness(interface_v_roughness)
            , thickness(thickness)
            , interface_eta(interface_eta)
            , albedo(albedo)
            , g(g)
            , conductor_u_roughness(conductor_u_roughness)
            , conductor_v_rougness(conductor_v_rougness)
            , conductor_eta(conductor_eta)
            , k(k)
            , reflectance(reflectance)
            , remap_roughness(remap_roughness)
            , max_depth(max_depth)
            , sample_count(sample_count)
        {}

        static const char* get_name()
        {
            return "CoatedConductorMaterial";
        }

        template <typename TextureEvaluator>
        LOQUAT_CPU_GPU
        bool can_evaluate_textures(TextureEvaluator tex_eval) const
        {
            return tex_eval.can_evaluate(
                { interface_u_roughness, interface_v_roughness, thickness,
                    g, conductor_u_roughness, conductor_v_rougness },
                { conductor_eta, k, reflectance, albedo });
        }

        template <typename TextureEvaluator>
        LOQUAT_CPU_GPU
        CoatedConductorBxDF get_BxDF(TextureEvaluator tex_eval,
            const MaterialEvalContext& ctx,
            SampledWavelengths& lambda) const;

        LOQUAT_CPU_GPU
        FloatTexture get_displacement() const
        {
            return displacement;
        }
        
        LOQUAT_CPU_GPU
        const Image* get_normal_map() const
        {
            return normal_map;
        }

        static CoatedConductorMaterial* create(
            const TextureParameterDictionary& parameters,
            Image* normal_map, const FileLoc* loc,
            Allocator alloc);

        template <typename TextureEvaluator>
        LOQUAT_CPU_GPU
        void get_BSSRDF(TextureEvaluator tex_eval,
            const MaterialEvalContext& ctx,
            SampledWavelengths& lambda, void*) const
        {}

        LOQUAT_CPU_GPU
        static constexpr bool has_subsurface_scattering()
        {
            return false;
        }

        std::string to_string() const;

    private:
        FloatTexture displacement;
        Image* normal_map;
        FloatTexture interface_u_roughness;
        FloatTexture interface_v_roughness;
        FloatTexture thickness;
        Spectrum interface_eta;
        FloatTexture g;
        SpectrumTexture albedo;
        FloatTexture conductor_u_roughness;
        FloatTexture conductor_v_rougness;
        SpectrumTexture conductor_eta;
        SpectrumTexture k;
        SpectrumTexture reflectance;
        bool remap_roughness;
        int max_depth;
        int sample_count;
    };

    class SubsurfaceMaterial
    {
    public:
        using BxDF = DielectricBxDF;
        using BSSRDF = TabulatedBSSRDF;

        SubsurfaceMaterial(Float scale, SpectrumTexture sigma_a, SpectrumTexture sigma_s,
            SpectrumTexture reflectance, SpectrumTexture mfp, Float g,
            Float eta, FloatTexture u_roughness, FloatTexture v_roughness,
            FloatTexture displacement, Image* normal_map, bool remap_roughness,
            Allocator alloc)
            : displacement(displacement)
            , normal_map(normal_map)
            , scale(scale)
            , sigma_a(sigma_a)
            , sigma_s(sigma_s)
            , reflectance(reflectance)
            , mfp(mfp)
            , u_roughness(u_roughness)
            , v_roughness(v_roughness)
            , eta(eta)
            , remap_roughness(remap_roughness)
            , table(100, 64, alloc)
        {
            compute_beam_diffusion_BSSRDF(g, eta, &table);
        }

        static const char* get_name()
        {
            return "SubsurfaceMaterial";
        }

        template <typename TextureEvaluator>
        LOQUAT_CPU_GPU
        bool can_evaluate_textures(TextureEvaluator tex_eval) const
        {
            return tex_eval.can_evaluate({ u_roughness, v_roughness }, { sigma_a, sigma_s });
        }

        template <typename TextureEvaluator>
        LOQUAT_CPU_GPU
        DielectricBxDF get_BxDF(TextureEvaluator tex_eval,
            const MaterialEvalContext& ctx, SampledWavelengths& lambda) const
        {
            // Initialize BSDF for _SubsurfaceMaterial_

            Float urough = tex_eval(u_roughness, ctx), vrough = tex_eval(v_roughness, ctx);
            if (remap_roughness)
            {
                urough = TrowbridgeReitzDistribution::RoughnessToAlpha(urough);
                vrough = TrowbridgeReitzDistribution::RoughnessToAlpha(vrough);
            }
            TrowbridgeReitzDistribution distrib(urough, vrough);

            // Initialize _bsdf_ for smooth or rough dielectric
            return DielectricBxDF(eta, distrib);
        }

        template <typename TextureEvaluator>
        LOQUAT_CPU_GPU
        TabulatedBSSRDF get_BSSRDF(TextureEvaluator tex_eval,
            const MaterialEvalContext& ctx,
            SampledWavelengths& lambda) const
        {
            SampledSpectrum sig_a, sig_s;
            if (sigma_a && sigma_s)
            {
                // Evaluate textures for $\sigma_\roman{a}$ and $\sigma_\roman{s}$
                sig_a = ClampZero(scale * tex_eval(sigma_a, ctx, lambda));
                sig_s = ClampZero(scale * tex_eval(sigma_s, ctx, lambda));

            }
            else
            {
                // Compute _sig_a_ and _sig_s_ from reflectance and mfp
                LOG_ASSERT(reflectance && mfp);
                SampledSpectrum mfree = ClampZero(scale * tex_eval(mfp, ctx, lambda));
                SampledSpectrum r = clamp(tex_eval(reflectance, ctx, lambda), 0, 1);
                SubsurfaceFromDiffuse(table, r, mfree, &sig_a, &sig_s);
            }
            return TabulatedBSSRDF(ctx.p, ctx.normal, ctx.outgoing, eta, sig_a, sig_s, &table);
        }

        LOQUAT_CPU_GPU
        FloatTexture get_displacement() const
        {
            return displacement;
        }

        LOQUAT_CPU_GPU
        const Image* get_normal_map() const
        {
            return normal_map;
        }

        LOQUAT_CPU_GPU
        static constexpr bool has_subsurface_scattering()
        {
            return true;
        }

        static SubsurfaceMaterial* create(
            const TextureParameterDictionary& parameters,
            Image* normal_map, const FileLoc* loc,
            Allocator alloc);

        std::string to_string() const;

    private:
        FloatTexture displacement;
        Image* normal_map;
        SpectrumTexture sigma_a;
        SpectrumTexture sigma_s;
        SpectrumTexture reflectance;
        SpectrumTexture mfp;
        Float scale, eta;
        FloatTexture u_roughness;
        FloatTexture v_roughness;
        bool remap_roughness;
        BSSRDFTable table;
    };

    class DiffuseTransmissionMaterial
    {
    public:
        using BxDF = DiffuseTransmissionBxDF;
        using BSSRDF = void;

        DiffuseTransmissionMaterial(SpectrumTexture reflectance,
            SpectrumTexture transmittance, FloatTexture displacement,
            Image* normal_map, Float scale)
            : displacement(displacement)
            , normal_map(normal_map)
            , reflectance(reflectance)
            , transmittance(transmittance)
            , scale(scale)
        {}

        static const char* get_name()
        {
            return "DiffuseTransmissionMaterial";
        }

        template <typename TextureEvaluator>
        LOQUAT_CPU_GPU
        bool can_evaluate_textures(TextureEvaluator tex_eval) const
        {
            return tex_eval.can_evaluate({}, { reflectance, transmittance });
        }

        template <typename TextureEvaluator>
        LOQUAT_CPU_GPU
        DiffuseTransmissionBxDF get_BxDF(TextureEvaluator tex_eval,
            MaterialEvalContext ctx, SampledWavelengths& lambda) const
        {
            SampledSpectrum r = clamp(scale * tex_eval(reflectance, ctx, lambda), 0, 1);
            SampledSpectrum t = clamp(scale * tex_eval(transmittance, ctx, lambda), 0, 1);
            return DiffuseTransmissionBxDF(r, t);
        }

        LOQUAT_CPU_GPU
        FloatTexture get_displacement() const
        {
            return displacement;
        }

        LOQUAT_CPU_GPU
        const Image* get_normal_map() const
        {
            return normal_map;
        }

        static DiffuseTransmissionMaterial* create(
            const TextureParameterDictionary& parameters, Image* normal_map,
            const FileLoc* loc, Allocator alloc);

        template <typename TextureEvaluator>
        LOQUAT_CPU_GPU
        void get_BSSRDF(TextureEvaluator tex_eval,
            MaterialEvalContext ctx, SampledWavelengths& lambda) const
        {}

        LOQUAT_CPU_GPU
        static constexpr bool has_subsurface_scattering()
        {
            return false;
        }

        std::string to_string() const;

    private:
        FloatTexture displacement;
        Image* normal_map;
        SpectrumTexture reflectance;
        SpectrumTexture transmittance;
        Float scale;
    };

    class MeasuredMaterial {
    public:
        using BxDF = MeasuredBxDF;
        using BSSRDF = void;

        template <typename TextureEvaluator>
        LOQUAT_CPU_GPU
            MeasuredBxDF get_BxDF(TextureEvaluator tex_eval,
                MaterialEvalContext ctx, SampledWavelengths& lambda) const
        {
            return MeasuredBxDF(brdf, lambda);
        }

        MeasuredMaterial(const std::string& filename, FloatTexture displacement,
            Image* normal_map, Allocator alloc);

        static const char* get_name()
        {
            return "MeasuredMaterial";
        }

        template <typename TextureEvaluator>
        LOQUAT_CPU_GPU
        bool can_evaluate_textures(TextureEvaluator tex_eval) const
        {
            return true;
        }

        LOQUAT_CPU_GPU
        FloatTexture get_displacement() const
        {
            return displacement;
        }

        LOQUAT_CPU_GPU
        const Image* get_normal_map() const
        {
            return normal_map;
        }

        static MeasuredMaterial* create(
            const TextureParameterDictionary& parameters,
            Image* normal_map, const FileLoc* loc, Allocator alloc);

        template <typename TextureEvaluator>
        LOQUAT_CPU_GPU
        void get_BSSRDF(TextureEvaluator tex_eval, MaterialEvalContext ctx,
            SampledWavelengths& lambda, void*) const
        {}

        LOQUAT_CPU_GPU
        static constexpr bool has_subsurface_scattering()
        {
            return false;
        }

        std::string to_string() const;

    private:
        FloatTexture displacement;
        Image* normal_map;
        const MeasuredBxDFData* brdf;
    };

    template <typename TextureEvaluator>
    inline BSDF Material::get_BSDF(TextureEvaluator tex_eval, MaterialEvalContext ctx,
        SampledWavelengths& lambda,
        ScratchBuffer& scratchBuffer) const
    {
        // Define _getBSDF_ lambda function for _Material::get_BSDF()_
        auto getBSDF = [&](auto mtl) -> BSDF {
            using ConcreteMtl = typename std::remove_reference_t<decltype(*mtl)>;
            using ConcreteBxDF = typename ConcreteMtl::BxDF;
            if constexpr (std::is_same_v<ConcreteBxDF, void>)
            {
                return BSDF();
            }
            else
            {
                // Allocate memory for _ConcreteBxDF_ and return _BSDF_ for material
                ConcreteBxDF* bxdf = scratchBuffer.Alloc<ConcreteBxDF>();
                *bxdf = mtl->get_BxDF(tex_eval, ctx, lambda);
                return BSDF(ctx.normal, ctx.dpdus, bxdf);
            }
            };

        return dispatchCPU(getBSDF);
    }

    template <typename TextureEvaluator>
    LOQUAT_CPU_GPU
    inline bool Material::can_evaluate_textures(TextureEvaluator tex_eval) const
    {
        auto eval = [&](auto ptr) { return ptr->can_evaluate_textures(tex_eval); };
        return dispatch(eval);
    }

    template <typename TextureEvaluator>
    inline BSSRDF Material::get_BSSRDF(TextureEvaluator tex_eval,
        MaterialEvalContext ctx, SampledWavelengths& lambda,
        ScratchBuffer& scratchBuffer) const
    {
        auto get = [&](auto mtl) -> BSSRDF {
            using Material = typename std::remove_reference_t<decltype(*mtl)>;
            using MaterialBSSRDF = typename Material::BSSRDF;
            if constexpr (std::is_same_v<MaterialBSSRDF, void>)
                return nullptr;
            else {
                MaterialBSSRDF* bssrdf = scratchBuffer.Alloc<MaterialBSSRDF>();
                *bssrdf = mtl->get_BSSRDF(tex_eval, ctx, lambda);
                return BSSRDF(bssrdf);
            }
            };
        return dispatchCPU(get);
    }

    LOQUAT_CPU_GPU
    inline bool Material::has_subsurface_scattering() const
    {
        auto has = [&](auto ptr) { return ptr->has_subsurface_scattering(); };
        return dispatch(has);
    }

    LOQUAT_CPU_GPU
    inline FloatTexture Material::get_displacement() const
    {
        auto disp = [&](auto ptr) { return ptr->get_displacement(); };
        return dispatch(disp);
    }

    LOQUAT_CPU_GPU
    inline const Image* Material::get_normal_map() const
    {
        auto nmap = [&](auto ptr) { return ptr->get_normal_map(); };
        return dispatch(nmap);
    }

}
