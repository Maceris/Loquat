package render

import vulkan from "vendor"

SwapChain :: struct {
    vulkan_swap_chain : VkSwapchainKHR,
    images : VkImage[..],
    image_views : VkImageView[..],
    extent : VkExtent2D,
    image_format : VkFormat,
}

SwapChainSupport :: struct {
    capabilities : VkSurfaceCapabilitiesKHR,
    formats : VkSurfaceFormatKHR[..],
    present_modes : VkPresentModeKHR[..],
}
