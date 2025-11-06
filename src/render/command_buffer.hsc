package render

import vulkan from "vendor"

CommandBuffer :: struct {
    buffer : VkCommandBuffer,
    command_pool : VkCommandPool,
}
