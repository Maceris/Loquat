// loquat is Copyright(c) 1998-2020 Matt Pharr, Wenzel Jakob, and Greg Humphreys.
// The loquat source code is licensed under the Apache License, Version 2.0.
// SPDX: Apache-2.0

// This file has been modified from the original, original notice is above.

#pragma once

#include "main/loquat.h"

#include "pbr/base/bxdf.h"
#include "pbr/base/camera.h"
#include "pbr/base/film.h"
#include "pbr/base/filter.h"
#include "pbr/base/light.h"
#include "pbr/base/light_sampler.h"
#include "pbr/base/sampler.h"
#ifdef LOQUAT_BUILD_GPU_RENDERER
#include "pbr/gpu/util.h"
#endif
#include "pbr/options.h"
#include "pbr/util/parallel.h"
#include "pbr/util/pstd.h"
#include "pbr/wavefront/work_items.h"
#include "pbr/wavefront/work_queue.h"

namespace loquat
{
	class BasicScene;
	class GUI;

    class WavefrontAggregate
    {
    public:
        virtual ~WavefrontAggregate() = default;

        virtual AABB3f get_bounds() const = 0;

        virtual void intersect_closest(int max_rays, const RayQueue* ray_q,
            EscapedRayQueue* escaped_ray_q,
            HitAreaLightQueue* hit_area_light_q,
            MaterialEvalQueue* basic_mtl_q,
            MaterialEvalQueue* universal_mtl_q,
            MediumSampleQueue* medium_sample_q,
            RayQueue* next_ray_q) const = 0;

        virtual void intersect_shadow(int max_rays,
            ShadowRayQueue* shadow_ray_queue,
            SOA<PixelSampleState>* pixel_sample_state) const = 0;
        virtual void intersect_shadow_tr(int max_rays,
            ShadowRayQueue* shadow_ray_queue,
            SOA<PixelSampleState>* pixel_sample_state) const = 0;

        virtual void intersect_one_random(int max_rays,
            SubsurfaceScatterQueue* subsurface_scatter_queue) const = 0;
    };

    class WavefrontPathIntegrator
    {
    public:
        Float render();

        void generate_camera_rays(int y0, Transform moving_from_camera, int sample_index);
        template <typename Sampler>
        void generate_camera_rays(int y0, Transform moving_from_camera, int sample_index);

        void generate_ray_samples(int wavefront_depth, int sample_index);
        template <typename Sampler>
        void generate_ray_samples(int wavefront_depth, int sample_index);

        void trace_shadow_rays(int wavefront_depth);
        void sample_medium_interaction(int wavefront_depth);
        template <typename PhaseFunction>
        void sample_medium_scattering(int wavefront_depth);
        void sample_subsurface(int wavefront_depth);

        void handle_escaped_rays();
        void handle_emissive_intersection();

        void evaluate_materials_and_BSDFs(int wavefront_depth, Transform moving_from_camera);
        template <typename ConcreteMaterial>
        void evaluate_material_and_BSDF(int wavefront_depth, Transform moving_from_camera);
        template <typename ConcreteMaterial, typename TextureEvaluator>
        void evaluate_material_and_BSDF(MaterialEvalQueue* evalQueue, Transform moving_from_camera,
            int wavefront_depth);

        void update_film();

        WavefrontPathIntegrator(pstd::pmr::memory_resource* memory_resource,
            BasicScene& scene);

        template <typename F>
        void parallel_for(const char* description, int item_count, F&& func)
        {
            if (options->use_GPU)
#ifdef LOQUAT_BUILD_GPU_RENDERER
                GPU_parallel_for(description, item_count, func);
#else
                LOG_FATAL("options->use_GPU was set without LOQUAT_BUILD_GPU_RENDERER enabled");
#endif
            else
                loquat::parallel_for(0, item_count, func);
        }

        template <typename F>
        void Do(const char* description, F&& func)
        {
            if (options->use_GPU)
#ifdef LOQUAT_BUILD_GPU_RENDERER
                GPU_parallel_for(description, 1, [=] LOQUAT_GPU(int) mutable { func(); });
#else
                LOG_FATAL("options->use_GPU was set without LOQUAT_BUILD_GPU_RENDERER enabled");
#endif
            else
                func();
        }

        RayQueue* current_ray_queue(int wavefront_depth)
        {
            return ray_queues[wavefront_depth & 1];
        }

        RayQueue* next_ray_queue(int wavefront_depth)
        {
            return ray_queues[(wavefront_depth + 1) & 1];
        }

#ifdef LOQUAT_BUILD_GPU_RENDERER
        void prefetch_GPU_allocations();
#endif

        // --display-server methods
        void start_display_thread();
        void update_display_RGB_from_film(AABB2i pixel_bounds);
        void stop_display_thread();

        // --interactive support
        void update_framebuffer_from_film(AABB2i pixel_bounds, Float exposure,
            RGB* rgb);

        // WavefrontPathIntegrator Member Variables
        bool initialize_visible_surface;
        bool have_subsurface;
        bool have_media;
        pstd::array<bool, Material::num_tags()> have_basic_eval_material;
        pstd::array<bool, Material::num_tags()> have_universal_eval_material;

        struct Stats
        {
            Stats(int maxDepth, Allocator alloc);

            std::string print() const;

            // Note: not atomics: tid 0 always updates them for everyone...
            uint64_t camera_rays = 0;
            pstd::vector<uint64_t> indirect_rays;
            pstd::vector<uint64_t> shadow_rays;
        };
        Stats* stats;

        pstd::pmr::memory_resource* memory_resource;

        Filter filter;
        Film film;
        Sampler sampler;
        Camera camera;
        pstd::vector<Light>* infinite_lights;
        LightSampler light_sampler;

        int max_depth;
        int samples_per_pixel;
        bool regularize;

        int scanlines_per_pass;
        int max_queue_size;

        SOA<PixelSampleState> pixel_sample_state;

        RayQueue* ray_queues[2];

        WavefrontAggregate* aggregate = nullptr;

        MediumSampleQueue* medium_sample_queue = nullptr;
        MediumScatterQueue* medium_scatter_queue = nullptr;

        EscapedRayQueue* escaped_ray_queue = nullptr;

        HitAreaLightQueue* hit_area_light_queue = nullptr;

        MaterialEvalQueue* basic_eval_material_queue = nullptr;
        MaterialEvalQueue* universal_eval_material_queue = nullptr;

        ShadowRayQueue* shadow_ray_queue = nullptr;

        GetBSSRDFAndProbeRayQueue* bssrdf_eval_queue = nullptr;
        SubsurfaceScatterQueue* subsurface_scatter_queue = nullptr;

        RGB* display_RGB = nullptr;
        RGB* display_RGB_host = nullptr;
        std::atomic<bool>* exit_copy_thread;
        std::thread* copy_thread;
    };

}