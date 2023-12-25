#include "main/global_state.h"

#include "main/vulkan_instance.h"

void unload_debug_messenger();

GlobalState::~GlobalState()
{
	if (debug_messenger)
	{
		unload_debug_messenger();
	}
	if (window_state)
	{
		delete window_state;
		window_state = nullptr;
	}
	if (instance)
	{
		vkDestroyInstance(instance, nullptr);
		instance = nullptr;
	}
}

bool load_debug_messenger(PFN_vkDebugUtilsMessengerCallbackEXT callback)
{
	VkDebugUtilsMessengerCreateInfoEXT createInfo{};
	createInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
	createInfo.messageSeverity = 
		VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT 
		| VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT
		| VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT 
		| VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
	createInfo.messageType = 
		VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT 
		| VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT 
		| VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
	createInfo.pfnUserCallback = callback;
	createInfo.pUserData = nullptr;

	VkResult result = VK_SUCCESS;
	
	VkAllocationCallbacks* allocator = nullptr;
	call_extension_function("vkCreateDebugUtilsMessengerEXT", &result, 
		g_global_state->instance, &createInfo, allocator, 
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