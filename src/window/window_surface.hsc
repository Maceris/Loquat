package window

import vulkan from "vendor"

WindowSurface :: struct {
    vulkan_surface :: ptr[VkSurfaceKHR],
    surface_format :: ptr[VkSurfaceFormatKHR],
    presentMode :: VkPresentModeKHR,
}
