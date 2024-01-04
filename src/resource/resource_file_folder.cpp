#include "resource/resource_file_folder.h"

#include <climits>
#include <filesystem>
#include <fstream>
#include <iostream>

#include "debug/logger.h"
#include "resource/resource.h"

namespace loquat
{
	namespace fs = std::filesystem;

	ResourceFileFolder::ResourceFileFolder(
		const std::string resource_folder_name)
		: resource_folder_name{ resource_folder_name + '/' }
	{
		LOG_INFO("Opening the resource folder " + resource_folder_name);
	}

	ResourceFileFolder::~ResourceFileFolder() = default;

	bool ResourceFileFolder::open()
	{
		if (!fs::is_directory(resource_folder_name))
		{
			LOG_ERROR("Resource folder does not exist!");
			return false;
		}
		return true;
	}

	size_t ResourceFileFolder::get_raw_resource_size(const Resource& resource)
	{
		const std::string full_path = resource_folder_name + resource.name;
		if (!fs::exists(full_path))
		{
			LOG_ERROR("Can not find the file " + full_path
				+ " in order to determine size");
			return 0;
		}
		if (!fs::is_regular_file(full_path))
		{
			LOG_ERROR(full_path + " is not a regular file, cannot determine size");
			return 0;
		}

		return fs::file_size(full_path);
	}

	size_t ResourceFileFolder::load_resource(const Resource& resource,
		char* buffer)
	{
		if (buffer == nullptr)
		{
			LOG_WARNING("Destination buffer is null");
			return 0;
		}

		const std::string full_path = resource_folder_name + resource.name;
		const auto size = fs::file_size(full_path);

		std::ifstream file_bytes(full_path, std::ios_base::binary);
		file_bytes.get(buffer, size);

		return size;
	}

	const size_t ResourceFileFolder::get_resource_count()
	{
		using predicate = bool (*)(const fs::path&);
		return std::count_if(fs::directory_iterator{ resource_folder_name },
			fs::directory_iterator{}, (predicate)fs::is_regular_file);
	}

	std::string ResourceFileFolder::get_resource_name(size_t index)
	{
		std::scoped_lock cache_lock{ cache_mutex };

		if (!has_file_name_cache)
		{
			for (auto const& file :
				fs::recursive_directory_iterator{ resource_folder_name })
			{
				if (!fs::is_regular_file(file))
				{
					continue;
				}
				cached_file_names.push_back(file.path().string());
			}
		}

		LOG_ASSERT(index >= 0 && index < cached_file_names.size()
			&& "Invalid resource index");

		return cached_file_names[index];
	}
}