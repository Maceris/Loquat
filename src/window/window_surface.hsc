package window

import vulkan from "vendor"

// TODO(ches) should these have cleaner types in our vendored library?

WindowSurface :: struct {
    vulkan_surface :: ptr[VkSurfaceKHR],
    surface_format :: ptr[VkSurfaceFormatKHR],
    presentMode :: VkPresentModeKHR,
}
