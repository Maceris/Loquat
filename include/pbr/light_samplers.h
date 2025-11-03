// pbrt is Copyright(c) 1998-2020 Matt Pharr, Wenzel Jakob, and Greg Humphreys.
// The pbrt source code is licensed under the Apache License, Version 2.0.
// SPDX: Apache-2.0

// This file has been modified from the original, original notice is above.

#pragma once

#include <algorithm>
#include <cstdint>
#include <string>

#include "main/loquat.h"
#include "pbr/base/light.h"
#include "pbr/base/light_sampler.h"
#include "pbr/lights.h"  // LightBounds. Should that live elsewhere?
#include "pbr/math/hash.h"
#include "pbr/math/vector_math.h"
#include "pbr/struct/containers.h"
#include "pbr/util/check.h"
#include "pbr/util/pstd.h"
#include "pbr/util/sampling.h"

namespace loquat
{
    class UniformLightSampler
    {
    public:
        UniformLightSampler(pstd::span<const Light> lights, Allocator alloc)
            : lights(lights.begin(), lights.end(), alloc)
        {}

        LOQUAT_CPU_GPU
        pstd::optional<SampledLight> sample(Float u) const
        {
            if (lights.empty())
            {
                return {};
            }
            int light_index = std::min<int>(u * lights.size(), lights.size() - 1);
            return SampledLight{ lights[light_index], 1.f / lights.size() };
        }

        LOQUAT_CPU_GPU
        Float PMF(Light light) const
        {
            if (lights.empty())
            {
                return 0;
            }
            return 1.f / lights.size();
        }

        LOQUAT_CPU_GPU
        pstd::optional<SampledLight> sample(const LightSampleContext& ctx,
            Float u) const
        {
            return sample(u);
        }

        LOQUAT_CPU_GPU
        Float PMF(const LightSampleContext& ctx, Light light) const
        {
            return PMF(light);
        }

        std::string to_string() const { return "UniformLightSampler"; }

    private:
        pstd::vector<Light> lights;
    };

    class PowerLightSampler
    {
    public:
        PowerLightSampler(pstd::span<const Light> lights, Allocator alloc);

        LOQUAT_CPU_GPU
        pstd::optional<SampledLight> sample(Float u) const
        {
            if (!alias_table.size())
            {
                return {};
            }
            Float pmf;
            int light_index = alias_table.sample(u, &pmf);
            return SampledLight{ lights[light_index], pmf };
        }

        LOQUAT_CPU_GPU
        Float PMF(Light light) const
        {
            if (!alias_table.size())
            {
                return 0;
            }
            return alias_table.PMF(light_to_index[light]);
        }

        LOQUAT_CPU_GPU
        pstd::optional<SampledLight> sample(const LightSampleContext& ctx,
            Float u) const
        {
            return sample(u);
        }

        LOQUAT_CPU_GPU
        Float PMF(const LightSampleContext& ctx, Light light) const
        {
            return PMF(light);
        }

        std::string to_string() const;

    private:
        pstd::vector<Light> lights;
        HashMap<Light, size_t> light_to_index;
        AliasTable alias_table;
    };

    class CompactLightBounds
    {
    public:
        CompactLightBounds() = default;

        LOQUAT_CPU_GPU
            CompactLightBounds(const LightBounds& lb, const AABB3f& allb)
            : direction(normalize(lb.direction))
            , phi(lb.phi)
            , q_cos_theta_o(quantize_cos(lb.cos_theta_o))
            , q_cos_theta_e(quantize_cos(lb.cos_theta_e))
            , two_sided(lb.two_sided)
        {
            // Quantize bounding box into _qb_
            for (int c = 0; c < 3; ++c)
            {
                qb[0][c] =
                    pstd::floor(quantize_bounds(lb.bounds[0][c], allb.min[c], allb.max[c]));
                qb[1][c] =
                    pstd::ceil(quantize_bounds(lb.bounds[1][c], allb.min[c], allb.max[c]));
            }
        }

        std::string to_string() const;
        std::string to_string(const AABB3f& all_bounds) const;

        LOQUAT_CPU_GPU
        bool is_two_sided() const
        {
            return two_sided;
        }

        LOQUAT_CPU_GPU
        Float get_cos_theta_o() const
        {
            return 2 * (q_cos_theta_o / 32767.f) - 1;
        }

        LOQUAT_CPU_GPU
        Float get_cos_theta_e() const
        {
            return 2 * (q_cos_theta_e / 32767.f) - 1;
        }

        LOQUAT_CPU_GPU
        AABB3f get_bounds(const AABB3f& allb) const
        {
            return { Point3f(lerp(qb[0][0] / 65535.f, allb.min.x, allb.max.x),
                            lerp(qb[0][1] / 65535.f, allb.min.y, allb.max.y),
                            lerp(qb[0][2] / 65535.f, allb.min.z, allb.max.z)),
                    Point3f(lerp(qb[1][0] / 65535.f, allb.min.x, allb.max.x),
                            lerp(qb[1][1] / 65535.f, allb.min.y, allb.max.y),
                            lerp(qb[1][2] / 65535.f, allb.min.z, allb.max.z)) };
        }

        LOQUAT_CPU_GPU
        Float importance(Point3f p, Normal3f n, const AABB3f& allb) const
        {
            AABB3f bounds = get_bounds(allb);
            Float cos_theta_o = get_cos_theta_o();
            Float cos_theta_e = get_cos_theta_e();
            // Return importance for light bounds at reference point
            // Compute clamped squared distance to reference point
            Point3f pc = (bounds.min + bounds.max) / 2.0f;
            Float d2 = distance_squared(p, pc);
            d2 = std::max(d2, loquat::length(bounds.diagonal()) / 2.0f);

            // Define cosine and sine clamped subtraction lambdas
            auto cosSubClamped = [](Float sinTheta_a, Float cosTheta_a,
                Float sinTheta_b, Float cosTheta_b) -> Float {
                    if (cosTheta_a > cosTheta_b)
                    {
                        return 1;
                    }
                    return cosTheta_a * cosTheta_b + sinTheta_a * sinTheta_b;
                };

            auto sinSubClamped = [](Float sinTheta_a, Float cosTheta_a,
                Float sinTheta_b, Float cosTheta_b) -> Float {
                    if (cosTheta_a > cosTheta_b)
                    {
                        return 0;
                    }
                    return sinTheta_a * cosTheta_b - cosTheta_a * sinTheta_b;
                };

            // Compute sine and cosine of angle to vector _w_, $\theta_\roman{direction}$
            Vec3f wi = normalize(p - pc);
            Float cosTheta_w = dot(Vec3f(direction), wi);
            if (two_sided)
                cosTheta_w = std::abs(cosTheta_w);
            Float sinTheta_w = safe_square_root(1 - square(cosTheta_w));

            // Compute $\cos\,\theta_\roman{\+b}$ for reference point
            Float cosTheta_b = bound_subtended_directions(bounds, p).cos_theta;
            Float sinTheta_b = safe_square_root(1 - square(cosTheta_b));

            // Compute $\cos\,\theta'$ and test against $\cos\,\theta_\roman{e}$
            Float sinTheta_o = safe_square_root(1 - square(cos_theta_o));
            Float cosTheta_x = cosSubClamped(sinTheta_w, cosTheta_w, sinTheta_o, cos_theta_o);
            Float sinTheta_x = sinSubClamped(sinTheta_w, cosTheta_w, sinTheta_o, cos_theta_o);
            Float cosThetap = cosSubClamped(sinTheta_x, cosTheta_x, sinTheta_b, cosTheta_b);
            if (cosThetap <= cos_theta_e)
            {
                return 0;
            }

            // Return final importance at reference point
            Float importance = phi * cosThetap / d2;
            DCHECK_GE(importance, -1e-3);
            // Account for $\cos\theta_\roman{i}$ in importance at surfaces
            if (n != Normal3f(0, 0, 0))
            {
                Float cosTheta_i = absolute_dot(wi, n);
                Float sinTheta_i = safe_square_root(1 - square(cosTheta_i));
                Float cosThetap_i =
                    cosSubClamped(sinTheta_i, cosTheta_i, sinTheta_b, cosTheta_b);
                importance *= cosThetap_i;
            }

            importance = std::max<Float>(importance, 0);
            return importance;
        }

    private:
        LOQUAT_CPU_GPU
        static unsigned int quantize_cos(Float c)
        {
            CHECK(c >= -1 && c <= 1);
            return pstd::floor(32767.f * ((c + 1) / 2));
        }

        LOQUAT_CPU_GPU
        static Float quantize_bounds(Float c, Float min, Float max)
        {
            CHECK(c >= min && c <= max);
            if (min == max)
            {
                return 0;
            }
            return 65535.f * clamp((c - min) / (max - min), 0, 1);
        }

        OctahedralVector direction;
        Float phi = 0;
        struct {
            unsigned int q_cos_theta_o : 15;
            unsigned int q_cos_theta_e : 15;
            unsigned int two_sided : 1;
        };
        uint16_t qb[2][3];
    };

    // LightBVHNode Definition
    struct alignas(32) LightBVHNode {
        // LightBVHNode Public Methods
        LightBVHNode() = default;

        LOQUAT_CPU_GPU
            static LightBVHNode MakeLeaf(unsigned int light_index, const CompactLightBounds& cb) {
            return LightBVHNode{ cb, {light_index, 1} };
        }

        LOQUAT_CPU_GPU
            static LightBVHNode MakeInterior(unsigned int child1Index,
                const CompactLightBounds& cb) {
            return LightBVHNode{ cb, {child1Index, 0} };
        }

        LOQUAT_CPU_GPU
            pstd::optional<SampledLight> sample(const LightSampleContext& ctx, Float u) const;

        std::string to_string() const;

        // LightBVHNode Public Members
        CompactLightBounds lightBounds;
        struct {
            unsigned int childOrLightIndex : 31;
            unsigned int isLeaf : 1;
        };
    };

    // BVHLightSampler Definition
    class BVHLightSampler {
    public:
        // BVHLightSampler Public Methods
        BVHLightSampler(pstd::span<const Light> lights, Allocator alloc);

        LOQUAT_CPU_GPU
            pstd::optional<SampledLight> sample(const LightSampleContext& ctx, Float u) const {
            // Compute infinite light sampling probability _pInfinite_
            Float pInfinite = Float(infiniteLights.size()) /
                Float(infiniteLights.size() + (nodes.empty() ? 0 : 1));

            if (u < pInfinite) {
                // sample infinite lights with uniform probability
                u /= pInfinite;
                int index =
                    std::min<int>(u * infiniteLights.size(), infiniteLights.size() - 1);
                Float pmf = pInfinite / infiniteLights.size();
                return SampledLight{ infiniteLights[index], pmf };

            }
            else {
                // Traverse light BVH to sample light
                if (nodes.empty())
                    return {};
                // Declare common variables for light BVH traversal
                Point3f p = ctx.p();
                Normal3f n = ctx.ns;
                u = std::min<Float>((u - pInfinite) / (1 - pInfinite), OneMinusEpsilon);
                int nodeIndex = 0;
                Float pmf = 1 - pInfinite;

                while (true) {
                    // Process light BVH node for light sampling
                    LightBVHNode node = nodes[nodeIndex];
                    if (!node.isLeaf) {
                        // Compute light BVH child node importances
                        const LightBVHNode* children[2] = { &nodes[nodeIndex + 1],
                                                           &nodes[node.childOrLightIndex] };
                        Float ci[2] = {
                            children[0]->lightBounds.importance(p, n, allLightBounds),
                            children[1]->lightBounds.importance(p, n, allLightBounds) };
                        if (ci[0] == 0 && ci[1] == 0)
                            return {};

                        // Randomly sample light BVH child node
                        Float nodePMF;
                        int child = SampleDiscrete(ci, u, &nodePMF, &u);
                        pmf *= nodePMF;
                        nodeIndex = (child == 0) ? (nodeIndex + 1) : node.childOrLightIndex;

                    }
                    else {
                        // Confirm light has nonzero importance before returning light sample
                        if (nodeIndex > 0)
                            DCHECK_GT(node.lightBounds.importance(p, n, allLightBounds), 0);
                        if (nodeIndex > 0 ||
                            node.lightBounds.importance(p, n, allLightBounds) > 0)
                            return SampledLight{ lights[node.childOrLightIndex], pmf };
                        return {};
                    }
                }
            }
        }

        LOQUAT_CPU_GPU
            Float PMF(const LightSampleContext& ctx, Light light) const {
            // Handle infinite _light_ PMF computation
            if (!lightToBitTrail.HasKey(light))
                return 1.f / (infiniteLights.size() + (nodes.empty() ? 0 : 1));

            // Initialize local variables for BVH traversal for PMF computation
            uint32_t bitTrail = lightToBitTrail[light];
            Point3f p = ctx.p();
            Normal3f n = ctx.ns;
            // Compute infinite light sampling probability _pInfinite_
            Float pInfinite = Float(infiniteLights.size()) /
                Float(infiniteLights.size() + (nodes.empty() ? 0 : 1));

            Float pmf = 1 - pInfinite;
            int nodeIndex = 0;

            // Compute light's PMF by walking down tree nodes to the light
            while (true) {
                const LightBVHNode* node = &nodes[nodeIndex];
                if (node->isLeaf) {
                    DCHECK_EQ(light, lights[node->childOrLightIndex]);
                    return pmf;
                }
                // Compute child importances and update PMF for current node
                const LightBVHNode* child0 = &nodes[nodeIndex + 1];
                const LightBVHNode* child1 = &nodes[node->childOrLightIndex];
                Float ci[2] = { child0->lightBounds.importance(p, n, allLightBounds),
                               child1->lightBounds.importance(p, n, allLightBounds) };
                DCHECK_GT(ci[bitTrail & 1], 0);
                pmf *= ci[bitTrail & 1] / (ci[0] + ci[1]);

                // Use _bitTrail_ to find next node index and update its value
                nodeIndex = (bitTrail & 1) ? node->childOrLightIndex : (nodeIndex + 1);
                bitTrail >>= 1;
            }
        }

        LOQUAT_CPU_GPU
            pstd::optional<SampledLight> sample(Float u) const {
            if (lights.empty())
                return {};
            int light_index = std::min<int>(u * lights.size(), lights.size() - 1);
            return SampledLight{ lights[light_index], 1.f / lights.size() };
        }

        LOQUAT_CPU_GPU
            Float PMF(Light light) const {
            if (lights.empty())
                return 0;
            return 1.f / lights.size();
        }

        std::string to_string() const;

    private:
        // BVHLightSampler Private Methods
        std::pair<int, LightBounds> buildBVH(
            std::vector<std::pair<int, LightBounds>>& bvhLights, int start, int end,
            uint32_t bitTrail, int depth);

        Float EvaluateCost(const LightBounds& b, const AABB3f& bounds, int dim) const {
            // Evaluate direction bounds measure for _LightBounds_
            Float theta_o = std::acos(b.cos_theta_o), theta_e = std::acos(b.cos_theta_e);
            Float theta_w = std::min(theta_o + theta_e, Pi);
            Float sinTheta_o = safe_square_root(1 - square(b.cos_theta_o));
            Float M_omega = 2 * Pi * (1 - b.cos_theta_o) +
                Pi / 2 *
                (2 * theta_w * sinTheta_o - std::cos(theta_o - 2 * theta_w) -
                    2 * theta_o * sinTheta_o + b.cos_theta_o);

            // Return complete cost estimate for _LightBounds_
            Float Kr = MaxComponentValue(bounds.diagonal()) / bounds.diagonal()[dim];
            return b.phi * M_omega * Kr * b.bounds.SurfaceArea();
        }

        pstd::vector<Light> lights;
        pstd::vector<Light> infiniteLights;
        AABB3f allLightBounds;
        pstd::vector<LightBVHNode> nodes;
        HashMap<Light, uint32_t> lightToBitTrail;
    };

    class ExhaustiveLightSampler {
    public:
        ExhaustiveLightSampler(pstd::span<const Light> lights, Allocator alloc);

        LOQUAT_CPU_GPU
            pstd::optional<SampledLight> sample(const LightSampleContext& ctx, Float u) const;

        LOQUAT_CPU_GPU
            Float PMF(const LightSampleContext& ctx, Light light) const;

        LOQUAT_CPU_GPU
            pstd::optional<SampledLight> sample(Float u) const {
            if (lights.empty())
                return {};

            int light_index = std::min<int>(u * lights.size(), lights.size() - 1);
            return SampledLight{ lights[light_index], 1.f / lights.size() };
        }

        LOQUAT_CPU_GPU
            Float PMF(Light light) const {
            if (lights.empty())
                return 0;
            return 1.f / lights.size();
        }

        std::string to_string() const;

    private:
        pstd::vector<Light> lights, boundedLights, infiniteLights;
        pstd::vector<LightBounds> lightBounds;
        HashMap<Light, size_t> lightToBoundedIndex;
    };

    LOQUAT_CPU_GPU inline pstd::optional<SampledLight> LightSampler::sample(const LightSampleContext& ctx,
        Float u) const {
        auto s = [&](auto ptr) { return ptr->sample(ctx, u); };
        return dispatch(s);
    }

    LOQUAT_CPU_GPU inline Float LightSampler::PMF(const LightSampleContext& ctx, Light light) const {
        auto pdf = [&](auto ptr) { return ptr->PMF(ctx, light); };
        return dispatch(pdf);
    }

    LOQUAT_CPU_GPU inline pstd::optional<SampledLight> LightSampler::sample(Float u) const {
        auto sample = [&](auto ptr) { return ptr->sample(u); };
        return dispatch(sample);
    }

    LOQUAT_CPU_GPU inline Float LightSampler::PMF(Light light) const {
        auto pdf = [&](auto ptr) { return ptr->PMF(light); };
        return dispatch(pdf);
    }
}
//TODO(ches) complete this