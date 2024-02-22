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


	template <typename T, int N, class Allocator = AllocatorBase<T>>
	class InlinedVector
	{
	public:
		using value_type = T;
		using allocator_type = Allocator;
		using size_type = std::size_t;
		using difference_type = std::ptrdiff_t;
		using reference_type = value_type&;
		using const_reference_type = const value_type&;
		using pointer_type = T*;
		using const_pointer_type = const T*;
		using iterator = T*;
		using const_iterator = const T*;
		using reverse_iterator = std::reverse_iterator<iterator>;
		using const_reverse_iterator = std::reverse_iterator<const_iterator>;

		InlinedVector(const Allocator& allocator = {}) noexcept
			: allocator{ allocator }
		{}
		InlinedVector(size_t count, const value_type& value,
			const Allocator& allocator = {}) noexcept
			: allocator{ allocator }
		{
			reserve(count);
			for (size_t i = 0; i < count; ++i)
			{
				this->allocator.template construct<T>(begin() + i, value);
			}
			stored_count = count;
		}
		InlinedVector(size_t count, const Allocator& allocator = {}) noexcept
			: InlinedVector{ count, T{}, allocator }
		{}
		InlinedVector(const InlinedVector& other,
			const Allocator& allocator = {}) noexcept
			: allocator{ allocator }
		{
			reserve(other.size());
			for (size_t i = 0; i < other.size(); ++i)
			{
				this->allocator.template construct<T>(begin() + i, other[i]);
			}
			stored_count = other.size();
		}
		template <class InputIterator>
		InlinedVector(InputIterator first, InputIterator last,
			const Allocator& allocator = {}) noexcept
			: allocator{ allocator }
		{
			reserve(last - first);
			for (InputIterator iterator = first; iterator != last;
				++iterator, ++stored_count)
			{
				this->allocator.template construct<T>(begin() + stored_count,
					*iterator);
			}
		}
		InlinedVector(InlinedVector&& other) noexcept
			: allocator{ other.allocator }
		{
			stored_count = other.stored_count;
			allocated_count = other.allocated_count;
			pointer = other.pointer;
			if (other.stored_count <= N)
			{
				for (int i = 0; i < other.stored_count; ++i)
				{
					this->allocator.template construct<T>(fixed + i,
						std::move(other.fixed[i]);
				}
			}
			else
			{
				other.stored_count = 0;
			}
			other.allocated_count = 0;
			other.pointer = nullptr;
		}

		InlinedVector(InlinedVector&& other, const Allocator& allocator)
			noexcept
		{
			if (allocator == other.allocator)
			{
				pointer = other.pointer;
				allocated_count = other.allocated_count;
				stored_count = other.stored_count;
				if (other.stored_count <= N)
				{
					for (int i = 0; i < other.stored_count; ++i)
					{
						fixed[i] = std::move(other.fixed[i]);
					}
				}
				other.pointer = nullptr;
				other.allocated_count = 0;
				other.stored_count = 0;
			}
			else
			{
				reserve(other.size());
				for (size_t i = 0; i < other.size(); ++i)
				{
					allocator.template construct<T>(begin() + i,
						std::move(other[i]));
				}
				stored_count = other.size();
			}
		}

		InlinedVector& operator=(const InlinedVector& other) noexcept
		{
			if (this == &other)
			{
				return *this;
			}

			clear();
			reserve(other.size());
			for (size_t i = 0; i < other.size(); ++i)
			{
				allocator.template construct<T>(begin() + i, other[i]);
			}
			stored_count = other.size();

			return *this;
		}

		InlinedVector& operator=(InlinedVector&& other) noexcept
		{
			if (this == &other)
			{
				return *this;
			}

			clear();
			if (allocator == other.allocator)
			{
				std::swap(pointer, other.pointer);
				std::swap(allocated_count, other.allocated_count);
				std::swap(stored_count, other.stored_count);
				if (stored_count > 0 && !pointer)
				{
					for (int i = 0; i < other.stored_count; ++i)
					{
						allocator.template construct<T>(begin() + i,
							std::move(other.fixed[i]));
					}
					other.stored_count = stored_count;
				}
			}
			else
			{
				reserve(other.size());
				for (size_t i = 0; i < other.size(); ++i)
				{
					allocator.template construct<T>(begin() + i,
						std::move(other[i]));
				}
				stored_count = other.size();
			}

			return *this;
		}

		InlinedVector& operator=(std::initializer_list<T>& init) noexcept
		{
			clear();
			reserve(init.size());
			for (const auto& value : init)
			{
				allocator.template construct<T>(begin() + stored_count, value);
				++stored_count;
			}
			return *this;
		}

		void assign(size_t count, const T& value) noexcept
		{
			clear();
			reserve(count);
			for (size_t i = 0; i < count; ++i)
			{
				allocator.template construct<T>(begin() + i, value);
				++stored_count;
			}
		}

		template <class InputIterator>
			requires requires(InputIterator a, InputIterator b) { b - a; }
		void assign(InputIterator first, InputIterator last) noexcept
		{
			difference_type count = last - first;
			if (count <= 0)
			{
				LOG_WARNING("Assigning using reversed iterators");
				return;
			}
			clear();
			reserve(static_cast<size_type>(count));

			for (InputIterator iterator = first; iterator != last;
				++iterator, ++stored_count)
			{
				this->allocator.template construct<T>(begin() + stored_count,
					*iterator);
			}
		}

		void assign(std::initializer_list<T>& init) noexcept
		{
			assign(init.begin(), init.end());
		}

		~InlinedVector() noexcept
		{
			clear();
			allocator.deallocate_object(pointer, allocated_count);
		}

		iterator begin() noexcept
		{
			return pointer ? pointer : fixed;
		}
		iterator end() noexcept
		{
			return begin() + stored_count;
		}
		const_iterator begin() const noexcept
		{
			return pointer ? pointer : fixed;
		}
		const_iterator end() const noexcept
		{
			return begin() + stored_count;
		}
		const_iterator cbegin() const noexcept
		{
			return pointer ? pointer : fixed;
		}
		const_iterator cend() const noexcept
		{
			return begin() + stored_count;
		}

		reverse_iterator rbegin() noexcept
		{
			return reverse_iterator{ end() };
		}
		reverse_iterator rend() noexcept
		{
			return reverse_iterator{ begin() };
		}
		const_reverse_iterator rbegin() const noexcept
		{
			return const_reverse_iterator{ end() };
		}
		const_reverse_iterator rend() const noexcept
		{
			return const_reverse_iterator{ begin() };
		}

		Allocator get_allocator() const noexcept
		{
			return allocator;
		}
		size_t size() const noexcept
		{
			return stored_count;
		}
		bool empty() const noexcept
		{
			return size() == 0;
		}
		size_t max_size() const noexcept
		{
			return (size_t)-1;
		}
		size_t capacity() const noexcept
		{
			return pointer ? allocated_count : N;
		}

		void reserve(size_t n) noexcept
		{
			if (capacity() >= N)
			{
				return;
			}

			T* reserved = allocator.template allocate_object<T>(n);
			for (int i = 0; i < stored_count; ++i)
			{
				allocator.template construct<T>(reserved + i,
					std::move(begin()[i]));
				allocator.destroy(begin() + i);
			}

			allocator.deallocate_object(pointer, allocated_count);
			allocated_count = n;
			pointer = reserved;
		}

		void shrink_to_fit() noexcept
		{
			if (capacity() >= n)
				return;

			if (allocated_count == stored_count)
			{
				return;
			}
			
			T* reserved = allocator.template allocate_object<T>(stored_count);
			for (int i = 0; i < stored_count; ++i)
			{
				allocator.template construct<T>(reserved + i,
					std::move(begin()[i]));
				allocator.destroy(begin() + i);
			}

			allocator.deallocate_object(pointer, allocated_count);
			allocated_count = stored_count;
			pointer = reserved;
		}

		T& operator[](size_t index) noexcept
		{
			LOG_ASSERT(index <= size() && "Index out of bounds");
			return begin()[index];
		}

		const T& operator[](size_t index) const noexcept
		{
			LOG_ASSERT(index <= size() && "Index out of bounds");
			return begin()[index];
		}

		T& front() noexcept
		{
			return *begin();
		}
		const T& front() const noexcept
		{
			return *begin();
		}
		T& back() noexcept
		{
			return *(begin() + stored_count - 1);
		}
		const T& back() const noexcept
		{
			return *(begin() + stored_count - 1);
		}
		T* data() noexcept
		{
			return pointer ? pointer : fixed;
		}
		const T* data() const noexcept
		{
			return pointer ? pointer : fixed;
		}

		void clear() noexcept
		{
			for (int i = 0; i < stored_count; ++i)
			{
				allocator.destroy(begin() + i);
			}
			stored_count = 0;
		}

		iterator insert(const_iterator position, const T& value) noexcept
		{
			LOG_FATAL("Not yet implemented");
			// TODO(ches) implement this
		}
		iterator insert(const_iterator position, T&& value) noexcept
		{
			LOG_FATAL("Not yet implemented");
			// TODO(ches) implement this
		}
		iterator insert(const_iterator position, size_t count, const T& value)
			noexcept
		{
			LOG_FATAL("Not yet implemented");
			// TODO(ches) implement this
		}
		template <class InputIterator>
		iterator insert(const_iterator position) noexcept
		{
			if (position == end())
			{
				reserve(size() + (last - first));
				iterator pos = end();
				for (auto iter = first; iter != last; ++iter, ++pos)
				{
					allocator.template construct<T>(pos, *iter);
				}
				stored_count += last - first;
				return pos;
			}
			else
			{
				LOG_FATAL("Not yet implemented");
				// TODO(ches) implement this
			}
		}
		iterator insert(const_iterator position, std::initializer_list<T> init)
			noexcept
		{
			LOG_FATAL("Not yet implemented");
			// TODO(ches) implement this
		}
		template <class... Args>
		iterator emplace(const_iterator pos, Args &&... args) noexcept
		{
			LOG_FATAL("Not yet implemented");
			// TODO(ches) implement this
		}
		template <class... Args>
		iterator emplace_back(Args &&... args) noexcept
		{
			LOG_FATAL("Not yet implemented");
			// TODO(ches) implement this
		}

		iterator erase(const_iterator cpos) noexcept
		{
			iterator pos = const_cast<iterator>(cpos);
			while (pos != end() - 1)
			{
				*pos = std::move(*(pos + 1));
				++pos;
			}
			allocator.destroy(pos);
			--stored_count;
			return const_cast<iterator>(cpos);
		}
		iterator erase(const_iterator first, const_iterator last) noexcept
		{
			LOG_FATAL("Not yet implemented");
			// TODO(ches) implement this
		}

		void push_back(const T& value) noexcept
		{
			if (size() == capacity())
			{
				reserve(2 * capacity());
			}
			allocator.construct(begin() stored_count, value);
			++stored_count;
		}
		void push_back(T&& value) noexcept
		{
			if (size() == capacity())
			{
				reserve(2 * capacity());
			}
			allocator.construct(begin() stored_count, std::move(value));
			++stored_count;
		}
		void pop_back() noexcept
		{
			LOG_ASSERT(!empty());
			allocator.destroy(begin() + stored_count - 1);
			--stored_count;
		}

		void resize(size_t n) noexcept
		{
			if (n < size())
			{
				for (size_t i = n; n < size(); ++i)
				{
					allocator.destroy(begin() + i);
				}
			}
			else if (n > size())
			{
				reserve(n);
				for (size_t i = stored_count; i < n; ++i)
				{
					allocator.construct(begin() + i);
				}
			}
			stored_count = n;
		}
		void resize(size_t count, const T& value) noexcept
		{
			LOG_FATAL("Not yet implemented");
			// TODO(ches) implement this
		}
		void swap(InlinedVector& other) noexcept
		{
			LOG_FATAL("Not yet implemented");
			// TODO(ches) implement this
		}

	private:
		Allocator allocator;
		/// <summary>
		/// A pointer to the contents, or nullptr if we are using fixed[].
		/// </summary>
		T* pointer = nullptr;

		union
		{
			T fixed[N];
		};
		size_type allocated_count = 0;
		size_type stored_count = 0;
	};

	template <typename Key, typename Value, typename Hash = std::hash<Key>,
		typename Allocator = AllocatorBase<std::optional<std::pair<Key, Value>>>
	>
	class HashMap
	{
	public:
		using TableEntry = std::optional<std::pair<Key, Value>>;

		class Iterator
		{
		public:
			Iterator& operator++() noexcept
			{
				while (++pointer < end && !pointer->has_value())
				{
					// Nothing required here
				}
				return *this;
			}

			Iterator operator++(int) noexcept
			{
				Iterator old = *this;
				operator++();
				return old;
			}

			bool operator==(const Iterator& other) const noexcept
			{
				return pointer == other.pointer;
			}

			bool operator!=(const Iterator& other) const noexcept
			{
				return pointer != other.pointer;
			}

			std::pair<Key, Value>& operator*() noexcept
			{
				return pointer->value();
			}

			const std::pair<Key, Value>& operator*() const noexcept
			{
				return pointer->value();
			}

			std::pair<Key, Value>* operator->() noexcept
			{
				return &pointer->value();
			}

			const std::pair<Key, Value>* operator->() const noexcept
			{
				return &pointer->value();
			}

		private:
			friend class HashMap;
			Iterator(TableEntry* pointer, TableEntry* end) noexcept
				: pointer{ pointer }
				, end{ end }
			{}
			TableEntry* pointer;
			TableEntry* end;
		};

		using iterator = Iterator;
		using const_iterator = const iterator;

		size_t size() const noexcept
		{
			return stored_count;
		}

		size_t capacity() const noexcept
		{
			return table.size();
		}

		void clear() noexcept
		{
			table.clear();
			stored_count = 0;
		}

		HashMap(Allocator allocator)
			: table{ 8, allocator }
		{}

		HashMap(const HashMap&) = delete;
		HashMap& operator=(const HashMap&) = delete;

		void insert(const Key& key, const Value& value) noexcept
		{
			size_t offset = find_offset(key);
			if (!table[offset].has_value())
			{
				if (3 * ++stored_count > capacity())
				{
					grow();
					offset = find_offset(key);
				}
			}
			table[offset] = std::make_pair(key, value);
		}

		bool has_key(const Key& key) const noexcept
		{
			return table[find_offset(key)].has_value();
		}

		const Value& operator[](const Key& key) const noexcept
		{
			size_t offset = find_offset(key);
			LOG_ASSERT(table[offset].has_value() && "Key not found");
			return table[offset]->second;
		}

		iterator begin() noexcept
		{
			Iterator iterator{ table.data(), table.data() + capacity() };
			while (iterator.pointer < iterator.end
				&& !iterator.pointer->has_value())
			{
				++iterator.pointer;
			}
			return iterator;
		}

	private:

		size_t find_offset(const Key& key) const noexcept
		{
			size_t base_offset = hash()(key) & (capacity() - 1);
			for (int probe_count = 0;/* nothing */; ++probe_count)
			{
				size_t offset = (base_offset + probe_count / 2
					+ probe_count * probe_count / 2) & (capacity() - 1);
				if (!table[offset].has_value() || key == table[offset]->first)
				{
					return offset;
				}
			}
			LOG_FATAL("Offset not found");
		}

		void grow() noexcept
		{
			size_t current_capacity = capacity();
			std::vector<TableEntry> new_table{
				std::max<size_t>(64, 2 * current_capacity),
				table.get_allocator() };
			size_t new_capacity = new_table.size();
			for (size_t i = 0; i < current_capacity; ++i)
			{
				if (!table[i].has_value())
				{
					continue;
				}
				size_t base_offset = hash()(table[i]->first) 
					& (new_capacity - 1);
				for (int probe_count = 0; /* nothing */; ++probe_count)
				{
					size_t offset = (base_offset + probe_count / 2
						+ probe_count * probe_count / 2) & (capacity() - 1);
					if (!new_table[offset])
					{
						new_table[offset] = std::move(*table[i]);
						break;
					}
				}
			}
			table = std::move(new_table);
		}

		std::vector<TableEntry> table;
		size_t stored_count = 0;
	};


	//TODO(ches) complete this
}