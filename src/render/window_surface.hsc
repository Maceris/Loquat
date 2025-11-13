package render

import vulkan from "vendor"

WindowSurface :: struct {
    vulkan_surface :: VkSurfaceKHR,
    surface_format :: VkSurfaceFormatKHR,
    presentMode :: VkPresentModeKHR,
}

select_present_mode :: fn(surface: ptr[mut WindowSurface], available_present_modes: VkPresentModeKHR[]) -> VkPresentModeKHR {
    //TODO(ches) fill this out
    return .VK_PRESENT_MODE_FIFO_KHR
}

select_surface_format :: fn(surface: ptr[mut WindowSurface], available_formats: VkSurfaceFormatKHR[]) -> VkSurfaceFormatKHR {
    //TODO(ches) fill this out
    result : VkSurfaceFormatKHR
    return result
}

initialize_window_surface :: fn(surface: ptr[mut WindowSurface]) {
    //TODO(ches) fill this out
}

