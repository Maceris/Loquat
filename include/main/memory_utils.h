#pragma once

#include <atomic>
#include <memory_resource>
#include <new>
#include <list>

namespace loquat
{

	/// <summary>
	/// The base type of allocator we are using. Used as a shorthand when we
	/// need to use the base allocator for different types.
	/// </summary>
	/// <typeparam name="T">The type of memory being allocated.</typeparam>
	template <typename T>
	using AllocatorBase = std::pmr::polymorphic_allocator<T>;

	/// <summary>
	/// The type of allocator we are using for memory management.
	/// </summary>
	using Allocator = AllocatorBase<std::byte>;

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

	class TrackedMemoryResource : public std::pmr::memory_resource
	{
	public:
		TrackedMemoryResource(std::pmr::memory_resource* source
			= std::pmr::get_default_resource()) noexcept
			: source{ source }
		{}

		void* do_allocate(size_t size, size_t alignment) noexcept
		{
			void* ptr = source->allocate(size, alignment);
			uint64_t current_bytes = allocated_bytes.fetch_add(size) + size;
			uint64_t previous_max =
				max_allocated_bytes.load(std::memory_order_relaxed);

			while (previous_max < current_bytes && !max_allocated_bytes
				.compare_exchange_weak(previous_max, current_bytes))
			{
			}

			return ptr;
		}

		void do_deallocate(void* p, size_t bytes, size_t alignment) noexcept
		{
			source->deallocate(p, bytes, alignment);
			allocated_bytes -= bytes;
		}

		bool do_is_equal(const memory_resource& other) const noexcept
		{
			return this == &other;
		}

		size_t CurrentAllocatedBytes() const noexcept
		{
			return allocated_bytes.load();
		}

		size_t MaxAllocatedBytes() const noexcept
		{
			return max_allocated_bytes.load();
		}

	private:
		std::pmr::memory_resource* source;
		std::atomic<uint64_t> allocated_bytes{ 0 };
		std::atomic<uint64_t> max_allocated_bytes{ 0 };
	};

	template <typename T>
	struct AllocationTraits
	{
		using SingleObject = T*;
	};

	template <typename T>
	struct AllocationTraits<T[]>
	{
		using Array = T*;
	};

	template <typename T, size_t n>
	struct AllocationTraits<T[n]>
	{
		struct Invalid {};
	};

	class alignas(std::hardware_destructive_interference_size) ScratchBuffer
	{
	public:
		ScratchBuffer(size_t size = 256) noexcept
			: allocation_size{ size }
		{
			pointer = (char*)g_allocator->allocate_bytes(size, align);
		}

		ScratchBuffer(const ScratchBuffer&) = delete;

		ScratchBuffer(ScratchBuffer&& other) noexcept
			: pointer{ other.pointer }
			, allocation_size{ other.allocation_size }
			, offset{ other.offset }
			, small_buffers{ std::move(other.small_buffers) }
		{
			other.pointer = nullptr;
			other.allocation_size = 0;
			other.offset = 0;
		}

		ScratchBuffer& operator=(const ScratchBuffer&) = delete;
		
		ScratchBuffer& operator=(ScratchBuffer&& other) noexcept
		{
			using std::swap;
			swap(other.pointer, pointer);
			swap(other.allocation_size, allocation_size);
			swap(other.offset, offset);
			swap(other.small_buffers, small_buffers);

			return *this;
		}

		~ScratchBuffer() noexcept
		{
			reset();
			g_allocator->deallocate_bytes(pointer, allocation_size, align);
		}

		void* allocate(size_t size, size_t align) noexcept
		{
			if (offset % align != 0)
			{
				offset += align - (offset % align);
			}
			if (offset + size > allocation_size)
			{
				reallocate(size);
			}
			void* ptr = pointer + offset;
			offset += size;
			return ptr;
		}

		template <typename T, typename... Args>
		typename AllocationTraits<T>::SingleObject allocate(Args &&...args)
			noexcept
		{
			T* ptr = (T*) allocate(sizeof(T), alignof(T));
			return g_allocator->construct(ptr, std::forward<Args>(args)...);
		}

		template <typename T>
		typename AllocationTraits<T>::Array allocate(size_t n = 1) noexcept
		{
			using ElementType = typename std::remove_extent_t<T>;
			ElementType* ret = (ElementType*)
				allocate(n * sizeof(ElementType), alignof(ElementType));

			for (size_t i = 0; i < n; ++i)
			{
				g_allocator->construct(&ret[i]);
			}
				
			return ret;
		}

		void reset() noexcept
		{
			for (const auto& buffer : small_buffers)
			{
				g_allocator->deallocate_bytes(buffer.first, buffer.second,
					align);
			}
			small_buffers.clear();
			offset = 0;
		}

	private:

		void reallocate(size_t min_size) noexcept
		{
			small_buffers.push_back(std::make_pair(pointer, allocation_size));
			allocation_size = 
				std::max(2 * min_size, allocation_size + min_size);
			pointer = (char*) 
				g_allocator->allocate_bytes(allocation_size, align);
			offset = 0;
		}

		static constexpr size_t align 
			= std::hardware_destructive_interference_size;
		char* pointer = nullptr;
		size_t allocation_size = 0;
		size_t offset = 0;
		std::list<std::pair<char*, size_t>> small_buffers;
	};
}