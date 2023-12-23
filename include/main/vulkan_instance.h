#pragma once

#include <vulkan/vulkan.h>

/// <summary>
/// Create a Vulkan instance, storing it in the global state. 
/// Expected to only be called once.
/// </summary>
void create_vulkan_instance() noexcept;
