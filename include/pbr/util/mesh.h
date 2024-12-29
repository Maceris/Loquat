// pbrt is Copyright(c) 1998-2020 Matt Pharr, Wenzel Jakob, and Greg Humphreys.
// The pbrt source code is licensed under the Apache License, Version 2.0.
// SPDX: Apache-2.0

// This file has been modified from the original, original notice is above.

#pragma once

#include <array>
#include <string>
#include <vector>

#include "main/loquat.h"

#include "pbr/math/hash.h"
#include "pbr/math/vector_math.h"
#include "pbr/struct/containers.h"
#include "pbr/util/parallel.h"
#include "pbr/util/pstd.h"

namespace loquat
{
    class TriangleMesh
    {
    public:
        TriangleMesh(const Transform& render_from_object,
            bool reverse_orientation, std::vector<int> vertex_indices,
            std::vector<Point3f> vertex_positions, std::vector<Vec3f> tangents,
            std::vector<Normal3f> vertex_normals, std::vector<Point2f> uvs,
            std::vector<int> face_indices, Allocator allocator);

        [[nodiscard]]
        std::string to_string() const;

        bool write_PLY(std::string filename) const;

        static void init(Allocator allocator);

        // TriangleMesh Public Members
        int triangle_count;
        int vertex_count;
        const int* vertex_indices = nullptr;
        const Point3f* vertex_positions = nullptr;
        const Normal3f* vertex_normals = nullptr;
        const Vec3f* tangents = nullptr;
        const Point2f* uvs = nullptr;
        const int* face_indices = nullptr;
        bool reverse_orientation;
        bool transform_swaps_handedness;
    };

    class BilinearPatchMesh
    {
    public:
        BilinearPatchMesh(const Transform& render_from_object,
            bool reverse_orientation, std::vector<int> vertex_indices,
            std::vector<Point3f> vertex_positions,
            std::vector<Normal3f> vertex_normals,
            std::vector<Point2f> uvs,
            std::vector<int> face_indices, PiecewiseConstant2D* image_distance,
            Allocator allocator);

        [[nodiscard]]
        std::string to_string() const;

        static void init(Allocator allocator);

        // BilinearPatchMesh Public Members
        bool reverse_orientation;
        bool transform_swaps_handedness;
        int patch_count;
        int vertex_count;
        const int* vertex_indices = nullptr;
        const Point3f* vertex_positions = nullptr;
        const Normal3f* vertex_normals = nullptr;
        const Point2f* uvs = nullptr;
        const int* face_indices = nullptr;
        PiecewiseConstant2D* image_distanceribution;
    };

    struct HashIntPair
    {
        LOQUAT_CPU_GPU
        size_t operator()(std::pair<int, int> p) const
        {
            return mix_bits(static_cast<uint64_t>(p.first) << 32 | p.second);
        };
    };

    struct TriQuadMesh
    {
        static TriQuadMesh read_PLY(const std::string& filename);

        void convert_to_only_triangles();
        void compute_normals();

        std::string to_string() const;

        template <typename Dist, typename Disp>
        TriQuadMesh displace(Dist&& dist, Float max_distance,
            Disp&& displace) const
        {
            if (uv.empty())
            {
                LOG_FATAL("Vertex uvs are currently required by displace()");
            }

            // Prepare the output mesh
            TriQuadMesh output_mesh = *this;
            output_mesh.convert_to_only_triangles();
            if (output_mesh.normals.empty())
            {
                output_mesh.compute_normals();
            }
            std::vector<int> old_tri_indices(output_mesh.tri_indices);
            output_mesh.tri_indices.clear();

            // Refine
            HashMap<std::pair<int, int>, int, HashIntPair> edge_split({});
            for (int i = 0; i < old_tri_indices.size() / 3; ++i)
            {
                output_mesh.refine(dist, max_distance, old_tri_indices[3 * i],
                    old_tri_indices[3 * i + 1], old_tri_indices[3 * i + 2],
                    edge_split);
            }

            // Displace
            displace(output_mesh.positions.data(), output_mesh.normals.data(),
                output_mesh.uvs.data(), output_mesh.positions.size());

            output_mesh.compute_normals();

            return output_mesh;
        }

        std::vector<Point3f> positions;
        std::vector<Normal3f> normals;
        std::vector<Point2f> uvs;
        std::vector<int> face_indices;
        std::vector<int> tri_indices;
        std::vector<int> quad_indices;

    private:
        template <typename Dist>
        void refine(Dist&& distance, Float max_distance, int v0, int v1, int v2,
            HashMap<std::pair<int, int>, int, HashIntPair>& edge_split)
        {
            Point3f p0 = p[v0];
            Point3f p1 = p[v1];
            Point3f p2 = p[v2];
            Float d01 = distance(p0, p1);
            Float d12 = distance(p1, p2);
            Float d20 = distance(p2, p0);

            if (d01 < max_distance && d12 < max_distance && d20 < max_distance)
            {
                tri_indices.push_back(v0);
                tri_indices.push_back(v1);
                tri_indices.push_back(v2);
                return;
            }

            // Order so that the first two vertices have the longest edge
            std::array<int, 3> v;
            if (d01 > d12)
            {
                if (d01 > d20)
                {
                    v = { v0, v1, v2 };
                }
                else
                {
                    v = { v2, v0, v1 };
                }
            }
            else
            {
                if (d12 > d20)
                {
                    v = { v1, v2, v0 };
                }
                else
                {
                    v = { v2, v0, v1 };
                }
            }

            // has the edge been spilt before?
            std::pair<int, int> edge(v[0], v[1]);
            if (v[0] > v[1])
            {
                std::swap(edge.first, edge.second);
            }

            int vmid;
            if (edge_split.has_key(edge))
            {
                vmid = edge_split[edge];
            }
            else
            {
                vmid = p.size();
                edge_split.insert(edge, vmid);
                p.push_back((p[v[0]] + p[v[1]]) / 2);
                if (!n.empty())
                {
                    Normal3f nn = n[v[0]] + n[v[1]];
                    if (length_squared(nn) > 0)
                    {
                        nn = normalize(nn);
                    }
                    n.push_back(nn);
                }
                if (!uv.empty())
                {
                    uv.push_back((uv[v[0]] + uv[v[1]]) / 2);
                }
            }

            refine(distance, max_distance, v[0], vmid, v[2], edge_split);
            refine(distance, max_distance, vmid, v[1], v[2], edge_split);
        }
    };

    bool write_PLY(std::string filename, pstd::span<const int> tri_indices,
        pstd::span<const int> quad_indices,
        pstd::span<const Point3f> positions,
        pstd::span<const Normal3f> normals, pstd::span<const Point2f> uvs,
        pstd::span<const int> face_indices);

}
