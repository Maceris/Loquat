// loquat is Copyright(c) 1998-2020 Matt Pharr, Wenzel Jakob, and Greg Humphreys.
// The loquat source code is licensed under the Apache License, Version 2.0.
// SPDX: Apache-2.0

// This file has been modified from the original, original notice is above.

#pragma once

#include <map>
#include <string>

#include "main/loquat.h"

#include "pbr/cpu/primitive.h"
#include "pbr/lights.h"
#include "pbr/materials.h"
#include "pbr/scene.h"
#include "pbr/struct/containers.h"
#include "pbr/struct/soa.h"
#include "pbr/util/pstd.h"
#include "pbr/wavefront/integrator.h"
#include "pbr/wavefront/work_items.h"

namespace loquat
{
    class CPUAggregate : public WavefrontAggregate
    {
    public:
        CPUAggregate(BasicScene& scene, NamedTextures& textures,
            const std::map<int, pstd::vector<Light>*>& shape_index_to_area_lights,
            const std::map<std::string, Medium>& media,
            const std::map<std::string, loquat::Material>& named_materials,
            const std::vector<loquat::Material>& materials);

        AABB3f get_bounds() const
        {
            return aggregate ? aggregate.bounds() : AABB3f();
        }

        void intersect_closest(int max_rays, const RayQueue* ray_queue,
            EscapedRayQueue* escaped_ray_queue,
            HitAreaLightQueue* hit_area_light_queue,
            MaterialEvalQueue* basic_eval_material_queue,
            MaterialEvalQueue* universal_eval_material_queue,
            MediumSampleQueue* medium_sample_queue,
            RayQueue* next_ray_queue) const;

        void intersect_shadow(int max_rays, ShadowRayQueue* shadow_ray_queue,
            SOA<PixelSampleState>* pixel_sample_state) const;

        void intersect_shadow_tr(int max_rays, ShadowRayQueue* shadow_ray_queue,
            SOA<PixelSampleState>* pixel_sample_state) const;

        void intersect_one_random(int max_rays,
            SubsurfaceScatterQueue* subsurface_scattering_queue) const;

    private:
        Primitive aggregate;
    };
}