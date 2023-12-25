#pragma once

#include <vector>

#include <vulkan/vulkan.h>

/// <summary>
/// Create a Vulkan instance, storing it in the global state. 
/// Expected to only be called once.
/// </summary>
void create_vulkan_instance() noexcept;

/// <summary>
/// Fetch a list of available extensions.
/// </summary>
/// <returns>All the available Vulkan extensions.</returns>
[[nodiscard]]
std::vector<VkExtensionProperties> get_available_extensions() noexcept;
