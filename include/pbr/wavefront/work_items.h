// pbrt is Copyright(c) 1998-2020 Matt Pharr, Wenzel Jakob, and Greg Humphreys.
// The pbrt source code is licensed under the Apache License, Version 2.0.
// SPDX: Apache-2.0

// This file has been modified from the original, original notice is above.

#pragma once

#include "main/loquat.h"

#include "pbr/films.h"
#include "pbr/light_samplers.h"
#include "pbr/lights.h"
#include "pbr/materials.h"
#include "pbr/base/sampler.h"
#include "pbr/math/ray.h"
#include "pbr/struct/containers.h"
#include "pbr/struct/soa.h"
#include "pbr/util/pstd.h"
#include "pbr/wavefront/work_queue.h"

namespace loquat
{
    struct RaySamples
    {
        struct {
            Point2f u;
            Float uc;
        } direct;
        struct {
            Float uc, rr;
            Point2f u;
        } indirect;
        bool have_subsurface;
        struct {
            Float uc;
            Point2f u;
        } subsurface;
    };

    template <>
    struct SOA<RaySamples>
    {
      public:
        SOA() = default;

        SOA(int size, Allocator alloc)
        {
            direct = alloc.allocate_object<Float4>(size);
            indirect = alloc.allocate_object<Float4>(size);
            subsurface = alloc.allocate_object<Float4>(size);
            media_dist = alloc.allocate_object<Float>(size);
            media_mode = alloc.allocate_object<Float>(size);
        }

        LOQUAT_CPU_GPU
        RaySamples operator[](int i) const
        {
            RaySamples rs;
            Float4 dir = load_4(direct + i);
            rs.direct.u = Point2f(dir.v[0], dir.v[1]);
            rs.direct.uc = dir.v[2];

            rs.have_subsurface = int(dir.v[3]) & 1;

            Float4 ind = load_4(indirect + i);
            rs.indirect.uc = ind.v[0];
            rs.indirect.rr = ind.v[1];
            rs.indirect.u = Point2f(ind.v[2], ind.v[3]);

            if (rs.have_subsurface) {
                Float4 ss = load_4(subsurface + i);
                rs.subsurface.uc = ss.v[0];
                rs.subsurface.u = Point2f(ss.v[1], ss.v[2]);
            }

            return rs;
        }

        struct GetSetIndirector
        {
            LOQUAT_CPU_GPU
            operator RaySamples() const
            {
                return (*(const SOA*)soa)[index];
            }

            LOQUAT_CPU_GPU
            void operator=(RaySamples rs)
            {
                int flags = rs.have_subsurface ? 1 : 0;
                soa->direct[index] = Float4{
                    rs.direct.u[0],
                    rs.direct.u[1],
                    rs.direct.uc,
                    Float(flags)
                };
                soa->indirect[index] = Float4{
                    rs.indirect.uc,
                    rs.indirect.rr,
                    rs.indirect.u[0],
                    rs.indirect.u[1]
                };
                if (rs.have_subsurface)
                {
                    soa->subsurface[index] = Float4{
                        rs.subsurface.uc,
                        rs.subsurface.u.x,
                        rs.subsurface.u.y,
                        0.f
                    };
                }
            }

            SOA* soa;
            int index;
        };

        LOQUAT_CPU_GPU
        GetSetIndirector operator[](int i)
        {
            return GetSetIndirector{this, i};
        }

      private:
        Float4* LOQUAT_RESTRICT direct;
        Float4* LOQUAT_RESTRICT indirect;
        Float4* LOQUAT_RESTRICT subsurface;
        Float* LOQUAT_RESTRICT media_dist;
        Float* LOQUAT_RESTRICT media_mode;
    };

    struct PixelSampleState
    {
        Point2i p_pixel;
        SampledSpectrum L;
        SampledWavelengths lambda;
        Float filter_weight;
        VisibleSurface visible_surface;
        SampledSpectrum camera_ray_weight;
        RaySamples samples;
    };

    struct RayWorkItem
    {
        Ray ray;
        int depth;
        SampledWavelengths lambda;
        int pixel_index;
        SampledSpectrum beta;
        SampledSpectrum r_u;
        SampledSpectrum r_l;
        LightSampleContext prev_intr_ctx;
        Float eta_scale;
        int specular_bounce;
        int any_non_specular_bounces;
    };

    struct EscapedRayWorkItem
    {
        Point3f rayo;
        Vec3f rayd;
        int depth;
        SampledWavelengths lambda;
        int pixel_index;
        SampledSpectrum beta;
        int specular_bounce;
        SampledSpectrum r_u;
        SampledSpectrum r_l;
        LightSampleContext prev_intr_ctx;
    };

    struct HitAreaLightWorkItem
    {
        Light area_light;
        Point3f p;
        Normal3f normal;
        Point2f uv;
        Vec3f outgoing;
        SampledWavelengths lambda;
        int depth;
        SampledSpectrum beta;
        SampledSpectrum r_u;
        SampledSpectrum r_l;
        LightSampleContext prev_intr_ctx;
        int specular_bounce;
        int pixel_index;
    };

    using HitAreaLightQueue = WorkQueue<HitAreaLightWorkItem>;

    struct ShadowRayWorkItem
    {
        Ray ray;
        Float t_max;
        SampledWavelengths lambda;
        SampledSpectrum Ld;
        SampledSpectrum r_u;
        SampledSpectrum r_l;
        int pixel_index;
    };

    struct GetBSSRDFAndProbeRayWorkItem
    {
        LOQUAT_CPU_GPU
        MaterialEvalContext get_material_eval_context() const
        {
            MaterialEvalContext ctx;
            ctx.outgoing = outgoing;
            ctx.normal = normal;
            ctx.shading_normal = shading_normal;
            ctx.dpdus = dpdus;
            ctx.p = p;
            ctx.uv = uv;
            return ctx;
        }

        Material material;
        SampledWavelengths lambda;
        SampledSpectrum beta;
        SampledSpectrum r_u;
        Point3f p;
        Vec3f outgoing;
        Normal3f normal;
        Normal3f shading_normal;
        Vec3f dpdus;
        Point2f uv;
        int depth;
        MediumInterface medium_interface;
        Float eta_scale;
        int pixel_index;
    };

    struct SubsurfaceScatterWorkItem
    {
        Point3f p0;
        Point3f p1;
        int depth;
        Material material;
        TabulatedBSSRDF bssrdf;
        SampledWavelengths lambda;
        SampledSpectrum beta;
        SampledSpectrum r_u;
        Float reservoir_PDF;
        Float u_light;
        SubsurfaceInteraction ssi;
        MediumInterface medium_interface;
        Float eta_scale;
        int pixel_index;
    };

    struct MediumSampleWorkItem
    {
        // Both enqueue types (have mtl and no hit)
        Ray ray;
        int depth;
        Float t_max;
        SampledWavelengths lambda;
        SampledSpectrum beta;
        SampledSpectrum r_u;
        SampledSpectrum r_l;
        int pixel_index;
        LightSampleContext prev_intr_ctx;
        int specular_bounce;
        int any_non_specular_bounces;
        Float eta_scale;

        // Have a hit material as well
        Light area_light;
        Point3fi pi;
        Normal3f normal;
        Vec3f dpdu;
        Vec3f dpdv;
        Vec3f outgoing;
        Point2f uv;
        Material material;
        Normal3f shading_normal;
        Vec3f dpdus;
        Vec3f dpdvs;
        Normal3f dndus;
        Normal3f dndvs;
        int face_index;
        MediumInterface medium_interface;
    };

    template <typename PhaseFunction>
    struct MediumScatterWorkItem
    {
        Point3f p;
        int depth;
        SampledWavelengths lambda;
        SampledSpectrum beta;
        SampledSpectrum r_u;
        const PhaseFunction* phase;
        Vec3f outgoing;
        Float time;
        Float eta_scale;
        Medium medium;
        int pixel_index;
    };

    template <typename ConcreteMaterial>
    struct MaterialEvalWorkItem
    {
        LOQUAT_CPU_GPU
        NormalBumpEvalContext get_normal_bump_eval_context(Float dudx,
            Float dudy, Float dvdx, Float dvdy) const
        {
            NormalBumpEvalContext ctx;
            ctx.p = Point3f(pi);
            ctx.uv = uv;
            ctx.dudx = dudx;
            ctx.dudy = dudy;
            ctx.dvdx = dvdx;
            ctx.dvdy = dvdy;
            ctx.shading.normal = shading_normal;
            ctx.shading.dpdu = dpdus;
            ctx.shading.dpdv = dpdvs;
            ctx.shading.dndu = dndus;
            ctx.shading.dndv = dndvs;
            ctx.face_index = face_index;
            return ctx;
        }

        LOQUAT_CPU_GPU
        MaterialEvalContext get_material_eval_context(Float dudx, Float dudy,
            Float dvdx, Float dvdy, Normal3f shading_normal, Vec3f dpdus) const
        {
            MaterialEvalContext ctx;
            ctx.outgoing = outgoing;
            ctx.normal = normal;
            ctx.shading_normal = shading_normal;
            ctx.dpdus = dpdus;
            ctx.p = Point3f(pi);
            ctx.uv = uv;
            ctx.dudx = dudx;
            ctx.dudy = dudy;
            ctx.dvdx = dvdx;
            ctx.dvdy = dvdy;
            ctx.face_index = face_index;
            return ctx;
        }

        const ConcreteMaterial* material;
        Point3fi pi;
        Normal3f normal;
        Vec3f dpdu;
        Vec3f dpdv;
        Float time;
        int depth;
        Normal3f shading_normal;
        Vec3f dpdus;
        Vec3f dpdvs;
        Normal3f dndus;
        Normal3f dndvs;
        Point2f uv;
        int face_index;
        SampledWavelengths lambda;
        int pixel_index;
        int any_non_specular_bounces;
        Vec3f outgoing;
        SampledSpectrum beta;
        SampledSpectrum r_u;
        Float eta_scale;
        MediumInterface medium_interface;
    };

    #include "wavefront_work_items_soa.h"

    class RayQueue : public WorkQueue<RayWorkItem>
    {
      public:
        using WorkQueue::WorkQueue;

        LOQUAT_CPU_GPU
        int push_camera_ray(const Ray& ray, const SampledWavelengths& lambda,
            int pixel_index);

        LOQUAT_CPU_GPU
        int push_indirect_ray(const Ray& ray, int depth,
            const LightSampleContext& prev_intr_ctx,
            const SampledSpectrum& beta, const SampledSpectrum& r_u,
            const SampledSpectrum& r_l, const SampledWavelengths& lambda,
            Float eta_scale, bool specular_bounce,
            bool any_non_specular_bounces, int pixel_index);
    };

    LOQUAT_CPU_GPU
    inline int RayQueue::push_camera_ray(const Ray& ray,
        const SampledWavelengths& lambda, int pixel_index)
    {
        int index = allocate_entry();
        LOG_ASSERT(!ray.has_NaN());
        this->ray[index] = ray;
        this->depth[index] = 0;
        this->pixel_index[index] = pixel_index;
        this->lambda[index] = lambda;
        this->beta[index] = SampledSpectrum(1.f);
        this->eta_scale[index] = 1.f;
        this->any_non_specular_bounces[index] = false;
        this->r_u[index] = SampledSpectrum(1.f);
        this->r_l[index] = SampledSpectrum(1.f);
        this->specular_bounce[index] = false;
        return index;
    }

    LOQUAT_CPU_GPU
    inline int RayQueue::push_indirect_ray(
        const Ray& ray, int depth, const LightSampleContext& prev_intr_ctx,
        const SampledSpectrum& beta, const SampledSpectrum& r_u,
        const SampledSpectrum& r_l, const SampledWavelengths& lambda, Float eta_scale,
        bool specular_bounce, bool any_non_specular_bounces, int pixel_index) {
        int index = allocate_entry();
        LOG_ASSERT(!ray.has_NaN());
        this->ray[index] = ray;
        this->depth[index] = depth;
        this->pixel_index[index] = pixel_index;
        this->prev_intr_ctx[index] = prev_intr_ctx;
        this->beta[index] = beta;
        this->r_u[index] = r_u;
        this->r_l[index] = r_l;
        this->lambda[index] = lambda;
        this->any_non_specular_bounces[index] = any_non_specular_bounces;
        this->specular_bounce[index] = specular_bounce;
        this->eta_scale[index] = eta_scale;
        return index;
    }

    using ShadowRayQueue = WorkQueue<ShadowRayWorkItem>;

    class EscapedRayQueue : public WorkQueue<EscapedRayWorkItem>
    {
      public:
          LOQUAT_CPU_GPU
          int push(RayWorkItem r);

          using WorkQueue::WorkQueue;

          using WorkQueue::push;
      };

    LOQUAT_CPU_GPU
    inline int EscapedRayQueue::push(RayWorkItem r)
    {
        return push(EscapedRayWorkItem{r.ray.origin, r.ray.direction,
            r.depth, r.lambda, r.pixel_index, r.beta, (int)r.specular_bounce,
            r.r_u, r.r_l, r.prev_intr_ctx});
    }

    class GetBSSRDFAndProbeRayQueue : public WorkQueue<GetBSSRDFAndProbeRayWorkItem>
    {
    public:
        using WorkQueue::WorkQueue;

        LOQUAT_CPU_GPU
        int push(Material material, SampledWavelengths lambda,
            SampledSpectrum beta, SampledSpectrum r_u, Point3f p,
            Vec3f outgoing, Normal3f normal, Normal3f shading_normal,
            Vec3f dpdus, Point2f uv, int depth,
            MediumInterface medium_interface, Float eta_scale, int pixel_index)
        {
            int index = allocate_entry();
            this->material[index] = material;
            this->lambda[index] = lambda;
            this->beta[index] = beta;
            this->r_u[index] = r_u;
            this->p[index] = p;
            this->outgoing[index] = outgoing;
            this->normal[index] = normal;
            this->shading_normal[index] = shading_normal;
            this->dpdus[index] = dpdus;
            this->uv[index] = uv;
            this->depth[index] = depth;
            this->medium_interface[index] = medium_interface;
            this->eta_scale[index] = eta_scale;
            this->pixel_index[index] = pixel_index;
            return index;
        }
    };

    class SubsurfaceScatterQueue : public WorkQueue<SubsurfaceScatterWorkItem>
    {
    public:
        using WorkQueue::WorkQueue;

        LOQUAT_CPU_GPU
        int push(Point3f p0, Point3f p1, int depth, Material material,
            TabulatedBSSRDF bssrdf, SampledWavelengths lambda,
            SampledSpectrum beta, SampledSpectrum r_u,
            MediumInterface medium_interface, Float eta_scale,
            int pixel_index)
        {
            int index = allocate_entry();
            this->p0[index] = p0;
            this->p1[index] = p1;
            this->depth[index] = depth;
            this->material[index] = material;
            this->bssrdf[index] = bssrdf;
            this->lambda[index] = lambda;
            this->beta[index] = beta;
            this->r_u[index] = r_u;
            this->medium_interface[index] = medium_interface;
            this->eta_scale[index] = eta_scale;
            this->pixel_index[index] = pixel_index;
            return index;
        }
    };

    class MediumSampleQueue : public WorkQueue<MediumSampleWorkItem>
    {
    public:
        using WorkQueue::WorkQueue;

        using WorkQueue::push;

        LOQUAT_CPU_GPU
        int push(Ray ray, Float t_max, SampledWavelengths lambda,
            SampledSpectrum beta, SampledSpectrum r_u, SampledSpectrum r_l,
            int pixel_index, LightSampleContext prev_intr_ctx,
            int specular_bounce, int any_non_specular_bounces,
            Float eta_scale)
        {
            int index = allocate_entry();
            this->ray[index] = ray;
            this->t_max[index] = t_max;
            this->lambda[index] = lambda;
            this->beta[index] = beta;
            this->r_u[index] = r_u;
            this->r_l[index] = r_l;
            this->pixel_index[index] = pixel_index;
            this->prev_intr_ctx[index] = prev_intr_ctx;
            this->specular_bounce[index] = specular_bounce;
            this->any_non_specular_bounces[index] = any_non_specular_bounces;
            this->eta_scale[index] = eta_scale;
            return index;
        }

        LOQUAT_CPU_GPU
        int push(RayWorkItem r, Float t_max)
        {
            return push(r.ray, t_max, r.lambda, r.beta, r.r_u, r.r_l,
                r.pixel_index, r.prev_intr_ctx, r.specular_bounce,
                r.any_non_specular_bounces, r.eta_scale);
        }
    };

    using MediumScatterQueue = MultiWorkQueue<
        typename MapType<MediumScatterWorkItem, typename PhaseFunction::Types>::type>;

    using MaterialEvalQueue = MultiWorkQueue<
        typename MapType<MaterialEvalWorkItem, typename Material::Types>::type>;

}