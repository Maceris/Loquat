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

		[[nodiscard]]
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
		[[nodiscard]]
		BVHBuildNode* build_HLBVH(Allocator allocator,
			const std::vector<BVHPrimitive>& primitive_info,
			std::atomic_ref<int> total_nodes,
			std::vector<Primitive>& ordered_primitives) noexcept;

		[[nodiscard]]
		BVHBuildNode* emit_LBVH(BVHBuildNode*& build_nodes,
			const std::vector<BVHPrimitive>& primitive_info,
			MortonPrimitive* morton_primitives, int primitive_count,
			int* total_nodes, std::vector<Primitive>& ordered_primitives,
			std::atomic_ref<int> ordered_primitives_offset, int bit_index)
			noexcept;

		[[nodiscard]]
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
	public:
		KdTreeAggregate(std::vector<Primitive> primitives,
			int intersection_cost = 5, int traversal_cost = 1,
			Float empty_bonus = 0.5, int max_primitives = 1,
			int max_depth = -1) noexcept;
		
		[[nodiscard]]
		static KdTreeAggregate* create(std::vector<Primitive> primitives,
			const ParameterDictionary& parameters) noexcept;

		[[nodiscard]]
		std::optional<ShapeIntersection> intersect(const Ray& ray,
			float t_max) const noexcept;

		AABB3f bounds() const noexcept
		{
			return cached_bounds;
		}

		bool has_intersection(const Ray& ray, float t_max) const noexcept;

	private:
		void build_tree(int node_number, const AABB3f& bounds,
			const std::vector<AABB3f>& primitive_bounds,
			std::span<const int> primitive_numbers, int depth,
			std::vector<BoundEdge> edges[3], std::span<int> primitives0,
			std::span<int> primitives1, int bad_refines);

		int intersection_cost;
		int traversal_cost;
		int max_primitives;
		Float empty_bonus;
		std::vector<Primitive> primitives;
		std::vector<int> primitive_indices;
		KdTreeNode* nodes;
		int allocated_node_count;
		int next_free_node;
		AABB3f cached_bounds;

	};
}