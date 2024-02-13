// pbrt is Copyright(c) 1998-2020 Matt Pharr, Wenzel Jakob, and Greg Humphreys.
// The pbrt source code is licensed under the Apache License, Version 2.0.
// SPDX: Apache-2.0

// This file has been modified from the original, original notice is above.

#pragma once

#include "main/loquat.h"
#include "pbr/base/primitive.h"
#include "pbr/util/parallel.h"

#include <atomic>
#include <memory>
#include <span>
#include <string_view>

namespace loquat
{
	Primitive create_accelerator(std::string_view,
		std::vector<Primitive> primitives,
		const ParameterDictionary& parameters) noexcept;

	struct BVHBuildNode;
	struct BVHPrimitive;
	struct LinearBVHNode;
	struct MortonPrimitive;

	class BVHAggregate
	{
	public:

		enum class SplitMethod
		{
			SurfaceAreaHeuristic,
			HiearchicalLinearBoundingVolumeHierarchy,
			Middle,
			EqualCounts
		};

		BVHAggregate(std::vector<Primitive> primitives,
			int max_primitives_in_node = 1,
			SplitMethod split_method = SplitMethod::SurfaceAreaHeuristic)
			noexcept;

		static BVHAggregate* create(std::vector<Primitive> primitives,
			const ParameterDictionary& parameters) noexcept;

		AABB3f bounds() const noexcept;

		std::optional<ShapeIntersection> intersect(const Ray& ray,
			Float t_max) const noexcept;

		bool has_intersection(const Ray& ray, Float t_max) const noexcept;

	private:
		BVHBuildNode* build_recursive(
			ThreadLocal<Allocator>& thread_allocators,
			std::span<BVHPrimitive> bvh_primitives,
			std::atomic_ref<int> total_nodes,
			std::atomic_ref<int> ordered_primitive_offset,
			std::vector<Primitive>& ordered_primitives) noexcept;

		/// <summary>
		/// Build Hiearchical Linear Bounding Volume Hierarchy.
		/// </summary>
		BVHBuildNode* build_HLBVH(Allocator allocator,
			const std::vector<BVHPrimitive>& primitive_info,
			std::atomic_ref<int> total_nodes,
			std::vector<Primitive>& ordered_primitives) noexcept;

		BVHBuildNode* emit_LBVH(BVHBuildNode*& build_nodes,
			const std::vector<BVHPrimitive>& primitive_info,
			MortonPrimitive* morton_primitives, int primitive_count,
			int* total_nodes, std::vector<Primitive>& ordered_primitives,
			std::atomic_ref<int> ordered_primitives_offset, int bit_index)
			noexcept;

		BVHBuildNode* build_upper_SAH(Allocator allocator,
			std::vector<BVHBuildNode*>& treelet_roots, int start,
			int end, std::atomic_ref<int> total_nodes) const noexcept;

		int flatten_BVH(BVHBuildNode* node, int* offset) noexcept;

		int max_primitives_in_node;
		std::vector<Primitive> primitives;
		SplitMethod split_method;
		LinearBVHNode* nodes = nullptr;
	};

	struct KdTreeNode;
	struct BoundEdge;

	class KdTreeAggregate
	{

	};
}