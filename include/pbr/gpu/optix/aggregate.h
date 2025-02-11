// loquat is Copyright(c) 1998-2020 Matt Pharr, Wenzel Jakob, and Greg Humphreys.
// The loquat source code is licensed under the Apache License, Version 2.0.
// SPDX: Apache-2.0

// This file has been modified from the original, original notice is above.

#pragma once

#include <map>
#include <string>
#include <vector>

#include <cuda.h>
#include <cuda_runtime.h>
#include <optix.h>

#include "main/loquat.h"

#include "pbr/gpu/memory.h"
#include "pbr/gpu/optix/optix.h"
#include "pbr/scene.h"
#include "pbr/struct/containers.h"
#include "pbr/struct/soa.h"
#include "pbr/math/vector_math.h"
#include "pbr/util/pstd.h"
#include "pbr/wavefront/integrator.h"
#include "pbr/wavefront/work_items.h"

namespace loquat
{
    class OptiXAggregate : public WavefrontAggregate {
    public:
        OptiXAggregate(const BasicScene& scene,
            CUDATrackedMemoryResource* memory_resource,
            NamedTextures& textures,
            const std::map<int, pstd::vector<Light>*>& shape_index_to_area_lights,
            const std::map<std::string, Medium>& media,
            const std::map<std::string, loquat::Material>& named_materials,
            const std::vector<loquat::Material>& materials);

        AABB3f get_bounds() const { return bounds; }

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
            SubsurfaceScatterQueue* subsurface_scatter_queue) const;

        // WAR: The enclosing parent function ("prepare_PLY_meshes") for an
        // extended __device__ lambda cannot have private or protected access
        // within its class, so it's public...
        static std::map<int, TriQuadMesh> prepare_PLY_meshes(
            const std::vector<ShapeSceneEntity>& shapes,
            const std::map<std::string, FloatTexture>& float_textures);

    private:
        struct HitgroupRecord;

        struct BVH
        {
            BVH() = default;
            BVH(size_t size);

            OptixTraversableHandle traversable_handle = {};
            std::vector<HitgroupRecord> intersect_HG_records;
            std::vector<HitgroupRecord> shadow_HG_records;
            std::vector<HitgroupRecord> random_hit_HG_records;
            AABB3f bounds;
        };

        static BVH build_BVH_for_triangles(
            const std::vector<ShapeSceneEntity>& shapes,
            const std::map<int, TriQuadMesh>& ply_meshes,
            OptixDeviceContext optix_context,
            const OptixProgramGroup& intersect_PG,
            const OptixProgramGroup& shadow_PG,
            const OptixProgramGroup& random_hit_PG,
            const std::map<std::string, FloatTexture>& float_textures,
            const std::map<std::string, Material>& named_materials,
            const std::vector<Material>& materials,
            const std::map<std::string, Medium>& media,
            const std::map<int, pstd::vector<Light>*>& shape_index_to_area_lights,
            ThreadLocal<Allocator>& thread_allocators,
            ThreadLocal<cudaStream_t>& thread_CUDA_streams);

        static BilinearPatchMesh* dice_curve_to_BLP(
            const ShapeSceneEntity& shape, int dice_u_count,
            int dice_v_count, Allocator alloc);

        static BVH build_BVH_for_BLPs(
            const std::vector<ShapeSceneEntity>& shapes,
            OptixDeviceContext optix_context,
            const OptixProgramGroup& intersect_PG,
            const OptixProgramGroup& shadow_PG,
            const OptixProgramGroup& random_hit_PG,
            const std::map<std::string, FloatTexture>& float_textures,
            const std::map<std::string, Material>& named_materials,
            const std::vector<Material>& materials,
            const std::map<std::string, Medium>& media,
            const std::map<int, pstd::vector<Light>*>& shape_index_to_area_lights,
            ThreadLocal<Allocator>& thread_allocators,
            ThreadLocal<cudaStream_t>& thread_CUDA_streams);

        static BVH build_BVH_for_quadrics(
            const std::vector<ShapeSceneEntity>& shapes,
            OptixDeviceContext optix_context,
            const OptixProgramGroup& intersect_PG,
            const OptixProgramGroup& shadow_PG,
            const OptixProgramGroup& random_hit_PG,
            const std::map<std::string, FloatTexture>& float_textures,
            const std::map<std::string, Material>& named_materials,
            const std::vector<Material>& materials,
            const std::map<std::string, Medium>& media,
            const std::map<int, pstd::vector<Light>*>& shape_index_to_area_lights,
            ThreadLocal<Allocator>& thread_allocators,
            ThreadLocal<cudaStream_t>& thread_CUDA_streams);

        int add_HG_records(const BVH& bvh);

        static OptixModule create_OptiX_module(OptixDeviceContext optix_context,
            const char* ptx);
        static OptixPipelineCompileOptions get_pipeline_compile_options();

        OptixProgramGroup create_raygen_PG(const char* entrypoint) const;
        OptixProgramGroup create_miss_PG(const char* entrypoint) const;
        OptixProgramGroup create_intersection_PG(const char* closest,
            const char* any, const char* intersect) const;

        static OptixTraversableHandle build_OptiX_BVH(
            OptixDeviceContext optix_context,
            const std::vector<OptixBuildInput>& build_inputs,
            ThreadLocal<cudaStream_t>& thread_CUDA_streams);

        CUDATrackedMemoryResource* memory_resource;
        std::mutex bounds_mutex;
        AABB3f bounds;
        CUstream cuda_stream;
        OptixDeviceContext optix_context;
        OptixModule optix_module;
        OptixPipeline optix_pipeline;

        struct ParamBufferState
        {
            bool used = false;
            cudaEvent_t finished_event;
            CUdeviceptr ptr = 0;
            void* host_ptr = nullptr;
        };
        mutable std::vector<ParamBufferState> params_pool;
        mutable size_t next_param_offset = 0;

        ParamBufferState& get_param_buffer(const RayIntersectParameters&) const;

        pstd::vector<HitgroupRecord> intersect_HG_records;
        pstd::vector<HitgroupRecord> shadow_HG_records;
        pstd::vector<HitgroupRecord> random_hit_HG_records;
        OptixShaderBindingTable intersectSBT = {};
        OptixShaderBindingTable shadow_SBT = {};
        OptixShaderBindingTable shadow_tr_SBT = {};
        OptixShaderBindingTable random_hit_SBT = {};
        OptixTraversableHandle root_traversable = {};
    };
}