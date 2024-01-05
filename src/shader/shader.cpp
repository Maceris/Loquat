#include "shader/shader.h"

#include <format>

#include "main/global_state.h"

namespace loquat
{
	ShaderModule::ShaderModule(const std::string& name)
	{
		Resource shader_resource{ name };
		auto handle = g_resource_cache->get_handle(&shader_resource);

		VkShaderModuleCreateInfo createInfo{};
		createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
		createInfo.codeSize = handle->get_size();
		createInfo.pCode = reinterpret_cast<const uint32_t*>(
			handle->get_buffer());

		if (vkCreateShaderModule(g_global_state->device->logical_device, 
			&createInfo, nullptr, &shader_module) != VK_SUCCESS)
		{
			LOG_FATAL(std::format("Could not load shader module {}!", name));
		}
	}

	ShaderModule::~ShaderModule()
	{
		if (shader_module)
		{
			vkDestroyShaderModule(g_global_state->device->logical_device,
				shader_module, nullptr);
		}
	}

	[[nodiscard]]
	constexpr VkShaderStageFlagBits stage_type(ShaderType stage) noexcept
	{
		switch (stage)
		{
		case ShaderType::fragment:
			return VK_SHADER_STAGE_FRAGMENT_BIT;
		case ShaderType::vertex:
			return VK_SHADER_STAGE_VERTEX_BIT;
		default:
			LOG_WARNING("Invalid shader stage provided");
			return VK_SHADER_STAGE_ALL;
		}
	}

	Shader::Shader(std::initializer_list<ShaderStage> stages) noexcept
	{
		LOG_ASSERT(stages.size() > 0
			&& "There were no shader stages provided");

		for (auto& stage : stages)
		{
			auto module = std::make_shared<ShaderModule>(stage.location);
			modules.push_back(module);

			VkPipelineShaderStageCreateInfo create_info{};
			create_info.sType =
				VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
			create_info.stage = stage_type(stage.type);
			create_info.module = module->shader_module;
			create_info.pName = "main";
			create_info.pSpecializationInfo = nullptr;

			create_info_list.push_back(create_info);
		}
	}

	Shader::~Shader() = default;
}