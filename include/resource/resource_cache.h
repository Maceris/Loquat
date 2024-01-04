#pragma once

#include "main/loquat.h"

#include <list>
#include <map>
#include <string>
#include <vector>

#include "resource/resource_file.h"
#include "resource/resource_handle.h"
#include "resource/resource_loader.h"

namespace loquat
{
	using ResourceHandleList = std::list<std::shared_ptr<ResourceHandle>>;
	using ResourceHandleMap = std::map<std::string, std::shared_ptr<ResourceHandle>>;
	using ResourceLoaders = std::list<std::shared_ptr<ResourceLoader>>;

	using ProgressCallback = void (*)(int, bool&);

	class ResourceCache
	{
		friend class ResourceHandle;

		/// <summary>
		/// The list of resource handles that are currently loaded. These are 
		/// stored in least-recently-used order, so every time a resource is used
		/// it gets moved to the front of the list.
		/// </summary>
		ResourceHandleList lru_list;

		/// <summary>
		/// Used to look up resource handles by name.
		/// </summary>
		ResourceHandleMap resources;

		/// <summary>
		/// A list of resource loaders. The most specific loaders should come
		/// first on the list, with more generic loaders last on the list.
		/// 
		/// Registering loaders pushes them to the front of the list.
		/// </summary>
		ResourceLoaders resource_loaders;

		/// <summary>
		/// The resource file that we load from.
		/// </summary>
		ResourceFile* file;

		/// <summary>
		/// The amount of cache memory in use.
		/// </summary>
		size_t cache_size;

		/// <summary>
		/// The total amount of memory allocated for the cache.
		/// </summary>
		size_t allocated;

	protected:

		/// <summary>
		/// Attempt to make room in the cache by freeing memory. If there is no
		/// possible way to free memory, we return false.
		/// </summary>
		/// <param name="size">The number of bytes we want to make room for.
		/// </param>
		/// <returns>Whether we have the specified amount of room.</returns>
		bool make_room(size_t size);

		/// <summary>
		/// Allocate raw memory of the specified size. If we did not have room
		/// to do so, null will be returned.
		/// </summary>
		/// <param name="size">The number of bytes to allocate.</param>
		/// <returns>The allocated memory, or null in the worst case.</returns>
		char* allocate(size_t size);

		/// <summary>
		/// Looks up a resource by handle and remove it from the cache.
		/// 
		/// The cache will only count the memory as freed once the resource
		/// pointed to by the handle is destroyed.
		/// </summary>
		/// <param name="resource">The resource to free.</param>
		void free(std::shared_ptr<ResourceHandle> resource);

		/// <summary>
		/// Loads a resource and returns the handle to it. If the resource cannot
		/// be found, or can't be laoded, an empty pointer is returned.
		/// </summary>
		/// <param name="resource">The resource we want to load.</param>
		/// <returns>The handle for the loaded resource, empty on failure.
		/// </returns>
		std::shared_ptr<ResourceHandle> load(Resource* resource);

		/// <summary>
		/// Looks up the resource handle for a resource. If no resource handle is
		/// found, an empty pointer is returned.
		/// </summary>
		/// <param name="resource">The resource to search for.</param>
		/// <returns>A pointer to the appropriate handle.</returns>
		std::shared_ptr<ResourceHandle> find(Resource* resource);

		/// <summary>
		/// Moves a handle to the front of the LRU list, to maintain the desired
		/// sorting.
		/// </summary>
		/// <param name="handle">The handle to update in the LRU tracking.</param>
		void update(std::shared_ptr<ResourceHandle> handle);

		/// <summary>
		/// Free the least recently used resource.
		/// 
		/// The cache will only count the memory as freed once the resource
		/// pointed to by the handle is destroyed.
		/// </summary>
		void free_one_resource();

		/// <summary>
		/// Called whenever memory associated with a resource has actually been
		/// freed, so that the cache can track allocation appropriately.
		/// </summary>
		/// <param name="size"></param>
		void memory_has_been_freed(size_t size);

	public:
		/// <summary>
		/// Create a resource cache for a file.
		/// </summary>
		/// <param name="size_in_MB">The amount of memory allocated to the 
		/// cache, in megabytes.</param>
		/// <param name="file">The file we are loading resources from.</param>
		ResourceCache(const size_t size_in_MB, ResourceFile* file);

		/// <summary>
		/// Clean up.
		/// </summary>
		virtual ~ResourceCache();

		/// <summary>
		/// Initializes the resource cache, preparing to load resources. If we
		/// had an issue, like the file already being in use by another cache
		/// instance, we will return false.
		/// </summary>
		/// <returns>Whether we initialized successfully.</returns>
		bool init();

		/// <summary>
		/// Register a resource loader, placing it at the front of the list of 
		/// loaders.
		/// 
		/// This way a loader for a specific file could be used, while
		/// still having more generic loaders for other files of the same
		/// extension.
		/// </summary>
		/// <param name="loader">The loader to register.</param>
		void register_loader(std::shared_ptr<ResourceLoader> loader);

		/// <summary>
		/// If a resource is already in the cache, return it. Otherwise, we load
		/// the resource from file first.
		/// 
		/// If we had an error, like running out of memory, an empty pointer is
		/// returned.
		/// </summary>
		/// <param name="resource">The resource to fetch.</param>
		/// <returns>A handle to the resource.</returns>
		std::shared_ptr<ResourceHandle> get_handle(Resource* resource);

		/// <summary>
		/// Pre-load a set of resources matching the specified wildcard pattern.
		/// For example, a pattern of "*.jpg" loads all the jpeg files in the 
		/// resource file.
		/// 
		/// This takes in a callback which will be notified with the percentage
		/// progress, and whether we have canceled loading.
		/// </summary>
		/// <param name="pattern">The wildcard pattern specifying which 
		/// resources to load.</param>
		/// <param name="callback">A callback to be updated regarding progress.
		/// May be null if progress does not need to be tracked.</param>
		/// <returns>The number of resources that were loaded.</returns>
		int preload(const std::string pattern, ProgressCallback callback);

		/// <summary>
		/// Searches through the cache for assets that match the specified 
		/// wildcard pattern.
		/// </summary>
		/// <param name="pattern">The wildcard pattern to look for.</param>
		/// <returns>A list of resource names that match the given pattern.
		/// </returns>
		std::vector<std::string> match(const std::string pattern);

		/// <summary>
		/// Free every handle in the cache. Useful for loading a new level or 
		/// debugging.
		/// </summary>
		void flush();
	};

	[[nodiscard]] extern bool 
		wildcard_match(const char* pattern, const char* string) noexcept;
}