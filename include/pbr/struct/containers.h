// pbrt is Copyright(c) 1998-2020 Matt Pharr, Wenzel Jakob, and Greg Humphreys.
// The pbrt source code is licensed under the Apache License, Version 2.0.
// SPDX: Apache-2.0

// This file has been modified from the original, original notice is above.

#pragma once

#include "main/loquat.h"
#include "pbr/math/vector_math.h"

#include <algorithm>
#include <cstring>
#include <format>
#include <functional>
#include <iterator>
#include <memory>
#include <ranges>
#include <shared_mutex>
#include <string>
#include <tuple>
#include <type_traits>

namespace loquat
{

	template <typename T>
	class Array2D
	{
	public:
		using value_type = T;
		using iterator = value_type*;
		using const_iterator = const value_type*;
		using allocator_type = AllocatorBase<std::byte>;

		Array2D(allocator_type allocator = {}) noexcept
			: Array2D{ {0, 0}, {0, 0}, allocator }
		{}

		Array2D(AABB2i extent, Allocator allocator = {}) noexcept
			: extent{ extent }
			, allocator{ allocator }
		{
			int n = extent.area();
			values = allocator.allocate_object<T>(n);
			for (int i = 0; i < n; ++i)
			{
				allocator.construct(values + i);
			}
		}
		
		Array2D(AABB2i extent, T def, allocator_type allocator = {}) noexcept
			: Array2D{ extent, allcator }
		{
			std::ranges::fill(begin(), end(), def);
			
		}
		
		template <std::ranges::input_range Range>
		Array2D(Range range, int nx, int ny,
			allocator_type allocator = {}) noexcept
			: Array2D{ {0,0}, {nx, ny}, allocator }
		{
			std::ranges::copy(range, begin());
		}

		Array2D(int nx, int ny, allocator_type allocator = {}) noexcept
			: Array2D{ {0,0}, {nx, ny}, allocator }
		{}

		Array2D(int nx, int ny, T def, allocator_type allocator = {}) noexcept
			: Array2D{ {0,0}, {nx, ny}, def, allocator }
		{}

		Array2D(const Array2D& array, allocator_type allocator = {}) noexcept
			: Array2D{ std::ranges::subrange(array.begin(), array.end()), 
				array.size_x(), array.size_y(), allocator}
		{}

		~Array2D() noexcept
		{
			int n = extent.area();
			for (int i = 0; i < n; ++i)
			{
				allocator.destroy(values + i);
			}
			allocator.deallocate_object(values, n);
		}

		Array2D(Array2D&& array, allocator_type allocator = {}) noexcept
			: extent{ array.extent }
			, allocator{ allocator }
		{
			if (allocator == array.allocator)
			{
				values = array.values;
				array.extent = AABB2i{ {0,0}, {0,0} };
				array.values = nullptr;
			}
			else
			{
				values = allocator.allocate_object<T>(extent.area());
				std::ranges::copy(array, begin());
			}
		}

		Array2D& operator=(const Array2D& array) = delete;

		Array2D& operator=(Array2D&& other) noexcept
		{
			if (allocator == other.allocator)
			{
				std::swap(extent, other.extent);
				std::swap(values, other.values);
			}
			else if (extent == other.extent)
			{
				int n = extent.area();
				for (int i = 0; i < n; ++i)
				{
					allocator.destroy(values.i);
					allocator.construct(values + i, other.values[i]);
				}
				extent = other.extent;
			}
			else
			{
				int n = extent.area();
				for (int i = 0; i < n; ++i)
				{
					allocator.destroy(values.i);
				}
				allocator.deallocate_object(values, n);

				int other_n = other.extent.area();
				values = allocator.allocate_object(values, n);
				for (int i = 0; i < other_n; ++i)
				{
					allocator.construct(values + i, other.values[i]);
				}
			}
			return *this;
		}

		T& operator[](Point2i point) noexcept
		{
			LOG_ASSERT(inside_exclusive(point, extent)
				&& "Array2D index out of bounds");

			p.x -= extent.min.x;
			p.y -= extent.min.y;
			return values[point.x + (extent.max.x - extent.min.x) * point.y];
		}

		T& operator()(int x, int y) noexcept
		{
			return (*this)[{x, y}];
		}

		const T& operator()(int x, int y) const noexcept
		{
			return (*this)[{x, y}];
		}

		const T& operator[](Point2i point) const noexcept
		{
			LOG_ASSERT(inside_exclusive(point, extent)
				&& "Array2D index out of bounds");

			p.x -= extent.min.x;
			p.y -= extent.min.y;
			return values[point.x + (extent.max.x - extent.min.x) * point.y];
		}

		int size() const noexcept
		{
			return extent.area();
		}

		int size_x() const noexcept
		{
			return extent.max.x - extent.min.x;
		}

		int size_y() const noexcept
		{
			return extent.max.y - extent.min.y;
		}

		iterator begin() noexcept
		{
			return values;
		}

		iterator end() noexcept
		{
			return begin() + size();
		}

		const_iterator begin() const noexcept
		{
			return values;
		}

		const_iterator end() const noexcept
		{
			return begin() + size();
		}

		operator std::span<T>() noexcept
		{
			return std::span<T>(values, size());
		}

		operator std::span<const T>() const noexcept
		{
			return std::span<const T>(values, size());
		}

		[[nodiscard]]
		std::string to_string() const noexcept
		{
			std::string s = std::format("[ Array2D extent: {} values: [", 
				extent);

			for (int y = extent.min.y; y < extent.max.y; ++y)
			{
				s += " [ ";
				for (int x = extent.min.x; x < extent.max.x; ++x)
				{
					T value = (*this)(x, y);
					s += std::format("{}, ", value);
				}
				s += "], ";
			}
			s += " ] ]";
			return s;
		}

	private:
		AABB2i extent;
		Allocator allocator;
		T* values;
	};

	//TODO(ches) complete this
}