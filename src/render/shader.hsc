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
