package pipeline

import shader from "../"
import vulkan from "vendor"

Pipeline :: struct {
    shader : ptr[Shader],
    layout : VkPipelineLayout,
    dynamic_states : VkDynamicState[..],
}
