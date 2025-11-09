package render

import vulkan from "vendor"

RenderState :: struct {
    image_available_semaphores : VkSemaphore[..],
    render_finished_semaphores : VkSemaphore[..],
    frame_in_flight_fences : VkFence[..],
    rendering_active : bool,
    current_frame : u64,
    command_buffers : VkCommandBuffer[..],
    command_pool : VkCommandPool,
}

current_command_buffer :: fn(render_state: ptr[RenderState]) -> VkCommandBuffer {
    //TODO(ches) fill this out
    frame : u64 : render_state.current_frame

    if (frame >= render_state.command_buffers.count) {
        return VK_NULL_HANDLE
    }

    return render_state.command_buffers[frame]
}

recreate_synchronization_objects :: fn(render_state: ptr[mut RenderState]) {
    destroy_synchronization_objects(render_state)
    create_synchronization_objects(render_state)
}

create_synchronization_objects :: fn(render_state: ptr[mut RenderState]) {
    //TODO(ches) fill this out
}

destroy_synchronization_objects :: fn(render_state: ptr[mut RenderState]) {
    //TODO(ches) fill this out
}

create_command_buffers :: fn(render_state: ptr[mut RenderState]) {
    //TODO(ches) fill this out
}

destroy_command_buffers :: fn(render_state: ptr[mut RenderState]) {
    //TODO(ches) fill this out
}

create_render_state :: fn() {
    //TODO(ches) fill this out
}

destroy_render_state :: fn() {
    //TODO(ches) fill this out
}
