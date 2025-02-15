// pbrt is Copyright(c) 1998-2020 Matt Pharr, Wenzel Jakob, and Greg Humphreys.
// The pbrt source code is licensed under the Apache License, Version 2.0.
// SPDX: Apache-2.0

// This file has been modified from the original, original notice is above.

#pragma once

#include <format>

#include "main/loquat.h"

#include "pbr/util/spectrum.h"
#include "pbr/wavefront/work_items.h"

namespace loquat
{
    inline LOQUAT_CPU_GPU void enqueue_work_after_miss(RayWorkItem r,
        MediumSampleQueue* medium_sample_queue,
        EscapedRayQueue* escaped_ray_queue) {
        if (r.ray.medium)
        {
            LOG_INFO(std::format("Adding miss ray to medium_sample_queue. "
                "ray {} {} {} d {} {} {} beta {} {} {} {}\n",
                r.ray.origin.x, r.ray.origin.y, r.ray.origin.z,
                r.ray.direction.x, r.ray.direction.y, r.ray.direction.z,
                r.beta[0], r.beta[1], r.beta[2], r.beta[3]));
            medium_sample_queue->push(r, INFINITY);
        }
        else if (escaped_ray_queue)
        {
            LOG_INFO(
                std::format("Adding ray to escaped_ray_queue pixel index {}\n",
                r.pixel_index));
            escaped_ray_queue->push(r);
        }
    }

    inline LOQUAT_CPU_GPU void record_shadow_ray_result(
        const ShadowRayWorkItem w, SOA<PixelSampleState>* pixel_sample_state,
        bool found_intersection)
    {
        if (found_intersection)
        {
            LOG_INFO("Shadow ray was occluded\n");
            return;
        }
        SampledSpectrum Ld = w.Ld / (w.r_u + w.r_l).average();
        LOG_INFO(std::format("Unoccluded shadow ray. Final Ld {} {} {} {} "
            "(sr.Ld {} {} {} {} r_u {} {} {} {} r_l {} {} {} {})\n",
            Ld[0], Ld[1], Ld[2], Ld[3], w.Ld[0], w.Ld[1], w.Ld[2], w.Ld[3], w.r_u[0],
            w.r_u[1], w.r_u[2], w.r_u[3], w.r_l[0], w.r_l[1], w.r_l[2], w.r_l[3]));

        SampledSpectrum Lpixel = pixel_sample_state->L[w.pixel_index];
        pixel_sample_state->L[w.pixel_index] = Lpixel + Ld;
    }

    inline LOQUAT_CPU_GPU void enqueue_work_after_intersection(
        RayWorkItem r, Medium ray_medium, float tMax, SurfaceInteraction intr,
        MediumSampleQueue* medium_sample_queue, RayQueue* next_ray_queue,
        HitAreaLightQueue* hit_area_light_queue,
        MaterialEvalQueue* basic_eval_material_queue,
        MaterialEvalQueue* universal_eval_material_queue)
    {
        MediumInterface medium_interface =
            intr.medium_interface ? *intr.medium_interface : MediumInterface(ray_medium);

        if (ray_medium)
        {
            assert(medium_sample_queue);
            LOG_INFO("Enqueuing into medium sample queue\n");
            medium_sample_queue->push(MediumSampleWorkItem{ r.ray,
                                                         r.depth,
                                                         tMax,
                                                         r.lambda,
                                                         r.beta,
                                                         r.r_u,
                                                         r.r_l,
                                                         r.pixel_index,
                                                         r.prev_intr_ctx,
                                                         r.specular_bounce,
                                                         r.any_non_specular_bounces,
                                                         r.eta_scale,
                                                         intr.area_light,
                                                         intr.point,
                                                         intr.normal,
                                                         intr.dpdu,
                                                         intr.dpdv,
                                                         -r.ray.direction,
                                                         intr.uv,
                                                         intr.material,
                                                         intr.shading.normal,
                                                         intr.shading.dpdu,
                                                         intr.shading.dpdv,
                                                         intr.shading.dndu,
                                                         intr.shading.dndv,
                                                         intr.face_index,
                                                         medium_interface });
            return;
        }

        // FIXME: this is all basically duplicate code w/medium.cpp
        Material material = intr.material;

        const MixMaterial* mix = material.cast_or_nullptr<MixMaterial>();
        while (mix) {
            MaterialEvalContext ctx(intr);
            material = mix->choose_material(BasicTextureEvaluator(), ctx);
            mix = material.cast_or_nullptr<MixMaterial>();
        }

        if (!material) {
            LOG_INFO(std::format("Enqueuing into medium transition queue: pixel index {} \n",
                r.pixel_index));
            Ray newRay = intr.spawn_ray(r.ray.direction);
            next_ray_queue->push_indirect_ray(newRay, r.depth, r.prev_intr_ctx, r.beta, r.r_u,
                r.r_l, r.lambda, r.eta_scale, r.specular_bounce,
                r.any_non_specular_bounces, r.pixel_index);
            return;
        }

        if (intr.area_light) {
            LOG_INFO(std::format("Ray hit an area light: adding to hit_area_light_queue pixel index {}\n",
                r.pixel_index));
            // TODO: intr.outgoing == -ray.direction?
            hit_area_light_queue->push(HitAreaLightWorkItem{
                intr.area_light, intr.p(), intr.normal, intr.uv, intr.outgoing, r.lambda, r.depth, r.beta,
                r.r_u, r.r_l, r.prev_intr_ctx, (int)r.specular_bounce, r.pixel_index });
        }

        FloatTexture displacement = material.get_displacement();

        MaterialEvalQueue* q =
            (material.can_evaluate_textures(BasicTextureEvaluator()) &&
                (!displacement || BasicTextureEvaluator().can_evaluate({ displacement }, {})))
            ? basic_eval_material_queue
            : universal_eval_material_queue;

        LOG_INFO(std::format("Enqueuing for material eval, mtl tag {}\n", material.tag()));

        auto enqueue = [=](auto ptr) {
            using Material = typename std::remove_reference_t<decltype(*ptr)>;
            q->push(MaterialEvalWorkItem<Material>{ptr,
                intr.point,
                intr.normal,
                intr.dpdu,
                intr.dpdv,
                intr.time,
                r.depth,
                intr.shading.normal,
                intr.shading.dpdu,
                intr.shading.dpdv,
                intr.shading.dndu,
                intr.shading.dndv,
                intr.uv,
                intr.face_index,
                r.lambda,
                r.pixel_index,
                r.any_non_specular_bounces,
                intr.outgoing,
                r.beta,
                r.r_u,
                r.eta_scale,
                medium_interface});
            };
        material.dispatch(enqueue);

        LOG_INFO(std::format("Closest hit found intersection at t {}\n", tMax));
    }

    struct TransmittanceTraceResult
    {
        bool hit;
        Point3f p_hit;
        Material material;
    };

    template <typename T, typename S>
    inline LOQUAT_CPU_GPU void trace_transmittance(ShadowRayWorkItem sr,
        SOA<PixelSampleState>* pixel_sample_state,
        T trace, S spawn_to)
    {
        SampledWavelengths lambda = sr.lambda;

        SampledSpectrum Ld = sr.Ld;

        Ray ray = sr.ray;
        Float tMax = sr.tMax;
        Point3f pLight = ray(tMax);
        RNG rng(Hash(ray.origin), Hash(ray.direction));

        SampledSpectrum T_ray(1.f);
        SampledSpectrum r_u(1.f), r_l(1.f);

        while (ray.direction != Vector3f(0, 0, 0)) {
            LOG_INFO(
                std::format("Tracing shadow tr shadow ray pixel index {} o {} {} {} d {} {} {} tMax {}\n",
                sr.pixel_index, ray.origin.x, ray.origin.y, ray.origin.z,
                ray.direction.x, ray.direction.y, ray.direction.z, tMax));

            TransmittanceTraceResult result = trace(ray, tMax);

            if (result.hit && result.material) {
                LOG_INFO("Hit opaque. Bye\n");
                // Hit opaque surface
                T_ray = SampledSpectrum(0.f);
                break;
            }

            if (ray.medium)
            {
                LOG_INFO(std::format("Ray medium {}. Will sample tmaj...\n", ray.medium.ptr()));

                Float tEnd = !result.hit
                    ? tMax
                    : (Distance(ray.origin, Point3f(result.p_hit)) / Length(ray.direction));
                SampledSpectrum T_maj = SampleT_maj(
                    ray, tEnd, rng.Uniform<Float>(), rng, lambda,
                    [&](Point3f p, MediumProperties mp, SampledSpectrum sigma_maj,
                        SampledSpectrum T_maj) {
                            SampledSpectrum sigma_n =
                                ClampZero(sigma_maj - mp.sigma_a - mp.sigma_s);

                            // ratio-tracking: only evaluate null scattering
                            Float pr = T_maj[0] * sigma_maj[0];
                            T_ray *= T_maj * sigma_n / pr;
                            r_l *= T_maj * sigma_maj / pr;
                            r_u *= T_maj * sigma_n / pr;

                            // Possibly terminate transmittance computation using Russian roulette
                            SampledSpectrum Tr = T_ray / (r_l + r_u).average();
                            if (Tr.MaxComponentValue() < 0.05f) {
                                Float q = 0.75f;
                                if (rng.Uniform<Float>() < q)
                                    T_ray = SampledSpectrum(0.);
                                else
                                    T_ray /= 1 - q;
                            }

                            LOG_INFO(std::format(
                                "T_maj {} {} {} {} sigma_n {} {} {} {} sigma_maj {} {} {} {}\n",
                                T_maj[0], T_maj[1], T_maj[2], T_maj[3], sigma_n[0], sigma_n[1],
                                sigma_n[2], sigma_n[3], sigma_maj[0], sigma_maj[1], sigma_maj[2],
                                sigma_maj[3]));
                            LOG_INFO(std::format(
                                "T_ray {} {} {} {} r_l {} {} {} {} r_u {} {} {} {}\n",
                                T_ray[0], T_ray[1], T_ray[2], T_ray[3], r_l[0], r_l[1],
                                r_l[2], r_l[3], r_u[0], r_u[1], r_u[2], r_u[3]));

                            if (!T_ray)
                            {
                                return false;
                            }

                            return true;
                    });
                T_ray *= T_maj / T_maj[0];
                r_l *= T_maj / T_maj[0];
                r_u *= T_maj / T_maj[0];
            }

            if (!result.hit || !T_ray)
            {
                // done
                break;
            }

            ray = spawn_to(pLight);
        }

        LOG_INFO(std::format("Final T_ray {:.9g} {:.9g} {:.9g} {:.9g} sr.r_u {:.9g} {:.9g} {:.9g} {:.9g} "
            "r_u {:.9g} {:.9g} {:.9g} {:.9g}\n",
            T_ray[0], T_ray[1], T_ray[2], T_ray[3], sr.r_u[0], sr.r_u[1],
            sr.r_u[2], sr.r_u[3], r_u[0], r_u[1], r_u[2], r_u[3]));
        LOG_INFO(std::format("sr.r_l {:.9g} {:.9g} {:.9g} {:.9g} r_l {:.9g} {:.9g} {:.9g} {:.9g}\n",
            sr.r_l[0], sr.r_l[1], sr.r_l[2], sr.r_l[3], r_l[0],
            r_l[1], r_l[2], r_l[3]));
        LOG_INFO(std::format("scaled throughput {:.9g} {:.9g} {:.9g} {:.9g}\n",
            T_ray[0] / (sr.r_u * r_u + sr.r_l * r_l).average(),
            T_ray[1] / (sr.r_u * r_u + sr.r_l * r_l).average(),
            T_ray[2] / (sr.r_u * r_u + sr.r_l * r_l).average(),
            T_ray[3] / (sr.r_u * r_u + sr.r_l * r_l).average()));

        if (T_ray)
        {
            // FIXME/reconcile: this takes r_l as input while
            // e.g. VolPathIntegrator::SampleLd() does not...
            Ld *= T_ray / (sr.r_u * r_u + sr.r_l * r_l).average();

            LOG_INFO(std::format("Setting final Ld for shadow ray pixel index {} = as {} {} {} {}\n",
                sr.pixel_index, Ld[0], Ld[1], Ld[2], Ld[3]));

            SampledSpectrum Lpixel = pixel_sample_state->L[sr.pixel_index];
            pixel_sample_state->L[sr.pixel_index] = Lpixel + Ld;
        }
    }
}