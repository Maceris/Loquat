#include "resource/resource.h"

#include <algorithm>
#include <cctype>

namespace loquat
{
	Resource::Resource(std::string_view resource_name)
		: name{ resource_name }
	{
		std::transform(name.begin(), name.end(), name.begin(),
                       [](unsigned char c){ return std::tolower(c); });
	}
}
