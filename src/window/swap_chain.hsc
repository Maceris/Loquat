package window

import vulkan from "vendor"

// TODO(ches) should these have cleaner types in our vendored library?

SwapChain :: struct {
    vulkan_swap_chain : VkSwapchainKHR,
    images : VkImage[..],
    image_views : VkImageView[..],
    extent : VkExtent2D,
    image_format : VkFormat,
}
