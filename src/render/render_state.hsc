package render

import vulkan from "vendor"

RenderState :: struct {
    image_available_semaphores : VkSemaphore[..],
    render_finished_semaphores : VkSemaphore[..],
    frame_in_flight_fences : VkFence[..],
    rendering_active : bool,
    current_frame : uint,
    command_buffers : VkCommandBuffer[..],
    command_pool : VkCommandPool,
}
