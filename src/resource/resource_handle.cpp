#include "resource/resource_handle.h"

#include "memory/memory_utils.h"
#include "resource/resource_cache.h"

namespace loquat
{
	const std::string ResourceHandle::get_name()
	{
		return resource.name;
	}

	size_t ResourceHandle::get_size()
	{
		return size;
	}

	const char* ResourceHandle::get_buffer()
	{
		return buffer;
	}

	char* ResourceHandle::get_writeable_buffer()
	{
		return buffer;
	}

	std::shared_ptr<ResourceExtraData> ResourceHandle::get_extra()
	{
		return extra;
	}

	void ResourceHandle::set_extra(std::shared_ptr<ResourceExtraData> extra)
	{
		this->extra = extra;
	}

	ResourceHandle::ResourceHandle(Resource& resource, char* buffer,
		size_t size, ResourceCache* resource_cache)
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