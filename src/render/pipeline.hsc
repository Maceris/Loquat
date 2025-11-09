package render

import vulkan from "vendor"

Pipeline :: struct {
    frame_buffers: VkFrameBuffer[..],
    graphics_pipeline: VkPipeline,
    render_pass: VkRenderPass,
    shader : ptr[Shader],
    layout : VkPipelineLayout,
    dynamic_states : VkDynamicState[..],
}

create_pipeline :: fn() {
    //TODO(ches) fill this out
}

create_frame_buffers :: fn() {
    //TODO(ches) fill this out
}

destroy_frame_buffers :: fn() {
    //TODO(ches) fill this out
}
