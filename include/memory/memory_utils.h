#pragma once

#include <cstddef>
#include <memory_resource>

namespace loquat
{
	/// <summary>
	/// The type of allocator we are using for memory management.
	/// </summary>
	using Allocator = std::pmr::polymorphic_allocator<std::byte>;

	/// <summary>
	/// The allocator for certain data structures, and potentially general use.
	/// </summary>
	extern Allocator* g_allocator;

	/// <summary>
	/// Whether we are using new/delete instead of just using a custom 
	/// allocator for everything. We still use a custom allocator for some
	/// data structures even if this is true.
	/// </summary>
	constexpr bool USE_DEFAULT_ALLOCATOR = false;

	/// <summary>
	/// Safely delete a pointer to an object, if it's not null.
	/// </summary>
	/// <typeparam name="T">The type of the poinZter to delete.</typeparam>
	/// <param name="ptr">The pointer we are deleting.</param>
	template<typename T>
	constexpr void safe_delete(T* ptr) noexcept
	{
		if constexpr (USE_DEFAULT_ALLOCATOR)
		{
			if (ptr)
			{
				delete ptr;
			}
		}
		else
		{
			if (ptr)
			{
				g_allocator->delete_object(ptr);
			}
		}
		ptr = nullptr;
	}

	/// <summary>
	/// Safely delete an array, if it's not null.
	/// </summary>
	/// <typeparam name="T">The type of the pointer to delete.</typeparam>
	/// <param name="ptr">The array we are deleting.</param>
	template<typename T>
	constexpr void safe_delete_array(T* arr) noexcept
	{
		if constexpr (USE_DEFAULT_ALLOCATOR)
		{
			if (arr)
			{
				delete[] arr;
			}
		}
		else
		{
			if (arr)
			{
				g_allocator->deallocate_object(arr, sizeof(arr) / sizeof(T));
			}
		}
		arr = nullptr;
	}

	/// <summary>
	/// Allocate and construct an object.
	/// </summary>
	/// <typeparam name="T">The type of object to create.</typeparam>
	/// <typeparam name="...Params">The type of the constructor parameters.
	/// </typeparam>
	/// <param name="...params">The constructor parameters to pass along.</param>
	/// <returns>A pointer to the resulting object.</returns>
	template<typename T, typename... Params>
	constexpr T* alloc(Params&&... params)
	{
		if constexpr (USE_DEFAULT_ALLOCATOR)
		{
			return new T(std::forward<Params>(params)...);
		}
		else
		{
			return g_allocator->new_object<T>(std::forward<Params>(params)...);
		}
	}

	/// <summary>
	/// Allocate an array to store objects in. Just allocates the memory,
	/// does not actually construct it.
	/// </summary>
	/// <typeparam name="T">The type of the array.</typeparam>
	/// <param name="count">The number of elements in the array.</param>
	/// <returns>A pointer to the newly allocated memory.</returns>
	template<typename T>
	constexpr T* alloc_array(size_t count)
	{
		if constexpr (USE_DEFAULT_ALLOCATOR)
		{
			return new T[count];
		}
		else
		{
			return g_allocator->allocate_object<T>(count);
		}
	}
}