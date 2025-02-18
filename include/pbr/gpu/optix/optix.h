// pbrt is Copyright(c) 1998-2020 Matt Pharr, Wenzel Jakob, and Greg Humphreys.
// The pbrt source code is licensed under the Apache License, Version 2.0.
// SPDX: Apache-2.0

// This file has been modified from the original, original notice is above.

#pragma once

#include <optix.h>

#include "main/loquat.h"

#include "pbr/base/light.h"
#include "pbr/base/material.h"
#include "pbr/base/medium.h"
#include "pbr/base/shape.h"
#include "pbr/base/texture.h"
#include "pbr/util/pstd.h"
#include "pbr/wavefront/work_items.h"
#include "pbr/wavefront/work_queue.h"

namespace loquat
{
    class TriangleMesh;
    class BilinearPatchMesh;

    struct TriangleMeshRecord
    {
        const TriangleMesh* mesh;
        Material material;
        FloatTexture alpha_texture;
        pstd::span<Light> area_lights;
        MediumInterface* medium_interface;
    };

    struct BilinearMeshRecord
    {
        const BilinearPatchMesh* mesh;
        Material material;
        FloatTexture alpha_texture;
        pstd::span<Light> area_lights;
        MediumInterface* medium_interface;
    };

    struct QuadricRecord
    {
        Shape shape;
        Material material;
        FloatTexture alpha_texture;
        Light area_light;
        MediumInterface* medium_interface;
    };

    struct RayIntersectParameters
    {
        OptixTraversableHandle traversable;

        const RayQueue* ray_queue;

        // closest hit
        RayQueue* next_ray_queue;
        EscapedRayQueue* escaped_ray_queue;
        HitAreaLightQueue* hit_area_light_queue;
        MaterialEvalQueue* basic_eval_material_queue;
        MaterialEvalQueue* universal_eval_material_ueue;
        MediumSampleQueue* medium_sample_queue;

        // shadow rays
        ShadowRayQueue* shadow_ray_queue;
        SOA<PixelSampleState> pixel_sample_state;

        // Subsurface scattering...
        SubsurfaceScatterQueue* subsurface_scatter_queue;
    };

}