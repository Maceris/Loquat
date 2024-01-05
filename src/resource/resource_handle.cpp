#include "resource/resource_handle.h"

#include "memory/memory_utils.h"
#include "resource/resource_cache.h"

namespace loquat
{
	[[nodiscard]]
	const std::string ResourceHandle::get_name() const noexcept
	{
		return resource.name;
	}

	[[nodiscard]]
	size_t ResourceHandle::get_size() const noexcept
	{
		return size;
	}

	[[nodiscard]]
	const char* ResourceHandle::get_buffer() const noexcept
	{
		return buffer;
	}

	[[nodiscard]]
	char* ResourceHandle::get_writeable_buffer() const noexcept
	{
		return buffer;
	}

	[[nodiscard]]
	std::shared_ptr<ResourceExtraData> ResourceHandle::get_extra() const noexcept
	{
		return extra;
	}

	void ResourceHandle::set_extra(std::shared_ptr<ResourceExtraData> extra) noexcept
	{
		this->extra = extra;
	}

	ResourceHandle::ResourceHandle(Resource& resource, char* buffer,
		size_t size, ResourceCache* resource_cache) noexcept
		: resource{ resource }
		, buffer{ buffer }
		, size{ size }
		, extra{ std::shared_ptr<ResourceExtraData>() }
		, resource_cache{ resource_cache }
	{}

	ResourceHandle::~ResourceHandle()
	{
		SAFE_DELETE_ARRAY(buffer);
		resource_cache->memory_has_been_freed(size);
	}
}