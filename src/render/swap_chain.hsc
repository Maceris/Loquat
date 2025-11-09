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

initialize_swap_chain :: fn(swap_chain: ptr[mut SwapChainSupport]) {
    //TODO(ches) fill this out
}

initialize_image_views :: fn(swap_chain: ptr[mut SwapChainSupport]) {
    //TODO(ches) fill this out
}

select_extent :: fn(capabilities: ptr[VkSurfaceCapabilitiesKHR]) -> VkExtent2D {
    //TODO(ches) fill this out
    result : VkExtent2D
    return result
}

create_swap_chain :: fn() {
    //TODO(ches) fill this out
}

recreate_swap_chain :: fn() {
    //TODO(ches) fill this out
}
