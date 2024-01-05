#include "main/global_state.h"

#include "main/vulkan_instance.h"
#include "memory/memory_utils.h"

namespace loquat
{
	void unload_debug_messenger();

	GlobalState::~GlobalState()
	{
		SAFE_DELETE(pipeline);
		SAFE_DELETE(window_state);
		SAFE_DELETE(device);
		if (debug_messenger)
		{
			unload_debug_messenger();
		}

		if (instance)
		{
			vkDestroyInstance(instance, nullptr);
			instance = nullptr;
		}
	}

	bool load_debug_messenger()
	{
		VkDebugUtilsMessengerCreateInfoEXT create_info;
		popualate_debug_info(create_info);

		VkResult result = VK_SUCCESS;

		VkAllocationCallbacks* allocator = nullptr;
		call_extension_function("vkCreateDebugUtilsMessengerEXT", &result,
			g_global_state->instance, &create_info, allocator,
			&g_global_state->debug_messenger);

		return result == VK_SUCCESS;
	}

	void unload_debug_messenger()
	{
		if (g_global_state->debug_messenger == nullptr)
		{
			LOG_WARNING("Trying to unload a debug messenger that is not loaded");
			return;
		}

		VkAllocationCallbacks* allocator = nullptr;
		call_extension_function("vkDestroyDebugUtilsMessengerEXT", nullptr,
			g_global_state->instance, g_global_state->debug_messenger, allocator);
	}
}