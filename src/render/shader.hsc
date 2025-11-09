package render

import vulkan from "vendor"

Shader :: struct {
    modules : ptr[VkShaderModule][..],
    create_info_list : VkPipelineShaderStageCreateInfo[..],
}

ShaderStage :: struct {
    type : ShaderType,
    location : string,
}

ShaderType :: enum {
    FRAGMENT,
    VERTEX,
}

stage_type :: fn(stage: ShaderType) -> VkShaderStageFlagBits {
    //TODO(ches) fill this out
    return VK_SHADER_STAGE_ALL
}
