#pragma once

#include <vector>

#include <vulkan/vulkan.h>

#include "debug/logger.h"
#include "main/global_state.h"

/// <summary>
/// The validation layers that we want to enable.
/// </summary>
const std::vector<const char*> VALIDATION_LAYERS = {
    "VK_LAYER_KHRONOS_validation"
};

#if _DEBUG
constexpr bool ENABLE_VALIDATION_LAYERS = true;
#elif
constexpr bool ENABLE_VALIDATION_LAYERS = false;
#endif

/// <summary>
/// Attemt to load and call an extension function.
/// </summary>
/// <typeparam name="...ParamTypes">The types of the parameters.</typeparam>
/// <typeparam name="ResultType">The return type.</typeparam>
/// <param name="function_name">The name of the function to call.</param>
/// <param name="result">An out paramter where the result will be stored.
/// If there is no result, this should be nullptr.</param>
/// <param name="...params">Parameters to the extension function.</param>
template <typename ResultType, typename... ParamTypes>
void call_extension_function(const char* function_name, ResultType&& result,
	ParamTypes... params)
{
    PFN_vkVoidFunction func = vkGetInstanceProcAddr(g_global_state->instance, 
        function_name);

    if (func == nullptr)
    {
        LOG_ERROR("Invalid extension function called: "
            + std::string(function_name));
        return;
    }

    using ActualReturn = std::conditional<
        std::is_same_v<ResultType, std::nullptr_t>, void, ResultType>::type;

    typedef ActualReturn (VKAPI_PTR* ActualSignature)(ParamTypes...);

    ActualSignature actual_func = (ActualSignature) func;

    if constexpr (std::is_same_v<ResultType, std::nullptr_t>)
    {
        actual_func(std::forward<ParamTypes>(params)...);
    }
    else
    {
        result = (ResultType)
            actual_func(std::forward<ParamTypes>(params)...);
    }
}

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

/// <summary>
/// Initialize and fill out the debug create info structure.
/// </summary>
/// <param name="create_info">The structure to fill out.</param>
void popualate_debug_info(VkDebugUtilsMessengerCreateInfoEXT& create_info)
noexcept;