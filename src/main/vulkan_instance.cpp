#include "main/vulkan_instance.h"

#include <cstdint>
#include <cstring>
#include <vector>

#include "revision.h"

#include "GLFW/glfw3.h"

namespace loquat
{

    static VKAPI_ATTR VkBool32 VKAPI_CALL debug_callback(
        VkDebugUtilsMessageSeverityFlagBitsEXT message_severity,
        VkDebugUtilsMessageTypeFlagsEXT message_type,
        const VkDebugUtilsMessengerCallbackDataEXT* callback_data,
        void* user_data)
    {
        std::string tag;
        switch (message_type)
        {
        case VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT:
            // Not relate dto specification or performance
            tag = "[General]";
            break;
        case VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT:
            // Violates specification or a possible mistake
            tag = "[Validation]";
            break;
        case VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT:
            // Potential non-optimal use of Vulkan
            tag = "[Performance]";
            break;
        default:
            tag = "[Other]";
            break;
        }

        const std::string message = tag + " Validation layer message: "
            + std::string(callback_data->pMessage);

        switch (message_severity)
        {
        case VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT:
            // Diagnostic message
            LOG_TAGGED("Debug", message);
            break;
        case VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT:
        default:
            // Basic info like creation of a resource
            LOG_INFO(message);
            break;
        case VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT:
            // Very likely a bug
            LOG_WARNING(message);
            break;
        case VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT:
            // Invalid behavior that may cause crashes
            LOG_ERROR(message);
            break;
        }

        return VK_FALSE;
    }

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

    [[nodiscard]]
    std::vector<const char*> get_required_extensions() noexcept
    {
        uint32_t glfw_extension_count = 0;
        const char** glfw_extensions;
        glfw_extensions = glfwGetRequiredInstanceExtensions(&glfw_extension_count);

        std::vector<const char*> extensions(glfw_extensions,
            glfw_extensions + glfw_extension_count);
        if (ENABLE_VALIDATION_LAYERS)
        {
            extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
        }

        return extensions;
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

        if constexpr (ENABLE_VALIDATION_LAYERS)
        {
            create_info.enabledLayerCount =
                static_cast<uint32_t>(VALIDATION_LAYERS.size());
            create_info.ppEnabledLayerNames = VALIDATION_LAYERS.data();

            VkDebugUtilsMessengerCreateInfoEXT debug_create_info{};
            popualate_debug_info(debug_create_info);
            create_info.pNext = &debug_create_info;
        }
        else
        {
            create_info.enabledLayerCount = 0;
            create_info.pNext = nullptr;
        }

        std::vector<const char*> extensions = get_required_extensions();
        create_info.enabledExtensionCount =
            static_cast<uint32_t>(extensions.size());
        create_info.ppEnabledExtensionNames = extensions.data();

        if (vkCreateInstance(&create_info, nullptr, &g_global_state->instance)
            != VK_SUCCESS)
        {
            LOG_FATAL("Failed to create Vulkan instance");
        }

        if (!load_debug_messenger())
        {
            LOG_ERROR("Failed attaching a debug callback for validation logs");
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

    void popualate_debug_info(VkDebugUtilsMessengerCreateInfoEXT& create_info)
        noexcept
    {
        create_info = {};
        create_info.sType =
            VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
        create_info.messageSeverity =
            VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT
            | VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT
            | VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT
            | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
        create_info.messageType =
            VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT
            | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT
            | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
        create_info.pfnUserCallback = debug_callback;
        create_info.pUserData = nullptr;
    }
}