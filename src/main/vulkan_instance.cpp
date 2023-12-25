#include "main/vulkan_instance.h"

#include <cstdint>

#include "revision.h"
#include "debug/logger.h"
#include "main/global_state.h"

#include "GLFW/glfw3.h"

void create_vulkan_instance() noexcept
{
    if (g_global_state->has_instance())
    {
        LOG_FATAL("We are trying to create a second Vulkan instance");
    }

    VkApplicationInfo app_info{};
    app_info.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    app_info.pApplicationName = "Loquat Renderer";
    app_info.applicationVersion = 
        VK_MAKE_API_VERSION(1, VERSION_MAJOR, VERSION_MINOR, VERSION_PATCH);
    app_info.pEngineName = "Loquat";
    app_info.engineVersion = 
        VK_MAKE_API_VERSION(1, VERSION_MAJOR, VERSION_MINOR, VERSION_PATCH);
    app_info.apiVersion = VK_API_VERSION_1_0;

    VkInstanceCreateInfo create_info{};
    create_info.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    create_info.pApplicationInfo = &app_info;
    create_info.enabledLayerCount = 0;

    uint32_t glfw_extension_count = 0;
    const char** glfw_extensions;
    glfw_extensions = glfwGetRequiredInstanceExtensions(&glfw_extension_count);
    create_info.enabledExtensionCount = glfw_extension_count;
    create_info.ppEnabledExtensionNames = glfw_extensions;

    if (vkCreateInstance(&create_info, nullptr, &g_global_state->instance)
        != VK_SUCCESS)
    {
        LOG_FATAL("Failed to create Vulkan instance");
    }
}

std::vector<VkExtensionProperties> get_available_extensions() noexcept
{
    uint32_t extension_count = 0;
    const char* layer_name = nullptr;
    vkEnumerateInstanceExtensionProperties(layer_name, &extension_count, 
        nullptr);
    std::vector<VkExtensionProperties> results(extension_count);
    vkEnumerateInstanceExtensionProperties(layer_name, &extension_count,
        results.data());
    return results;
}