#include "main/vulkan_instance.h"

#include <cstdint>
#include <cstring>
#include <vector>

#include "revision.h"
#include "debug/logger.h"
#include "main/global_state.h"

#include "GLFW/glfw3.h"

const std::vector<const char*> VALIDATION_LAYERS = {
    "VK_LAYER_KHRONOS_validation"
};

#if _DEBUG
constexpr bool ENABLE_VALIDATION_LAYERS = true;
#elif
constexpr bool ENABLE_VALIDATION_LAYERS = false;
#endif

/// <summary>
/// Check that we have all the validation layers that we expect to have.
/// </summary>
/// <returns>Whether we have all the validation layers we want.</returns>
[[nodiscard]] bool check_validation_layer_support() noexcept
{
    uint32_t layer_count;
    vkEnumerateInstanceLayerProperties(&layer_count, nullptr);

    std::vector<VkLayerProperties> available_layers(layer_count);
    vkEnumerateInstanceLayerProperties(&layer_count, available_layers.data());
    
    for (const auto& layer_name : VALIDATION_LAYERS)
    {
        bool layer_found = false;

        for (const auto& layer_properties : available_layers)
        {
            if (strcmp(layer_name, layer_properties.layerName) == 0)
            {
                layer_found = true;
                break;
            }
        }
        if (!layer_found)
        {
            LOG_INFO("We are missing (at least) the validation layer "
                + std::string(layer_name));
            return false;
        }
    }
    return true;
}

void create_vulkan_instance() noexcept
{
    if (g_global_state->has_instance())
    {
        LOG_FATAL("We are trying to create a second Vulkan instance");
    }

    if (ENABLE_VALIDATION_LAYERS && !check_validation_layer_support())
    {
        LOG_FATAL("We expect validation layers but don't have them");
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

[[nodiscard]]
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