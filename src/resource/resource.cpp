#include "resource/resource.h"

#include <algorithm>

namespace loquat
{
	Resource::Resource(const std::string& resource_name)
		: name{ resource_name }
	{
		std::transform(name.begin(), name.end(), name.begin(), std::tolower);
	}
}