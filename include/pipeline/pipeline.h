#pragma once

#include <memory>
#include <vector>

#include "shader/shader.h"

namespace loquat
{
	/// <summary>
	/// Used to configure the state of the pipeline.
	/// </summary>
	struct Pipeline
	{
		Pipeline(std::unique_ptr<Shader> shader);
		~Pipeline();
		Pipeline(const Pipeline&) = delete;
		Pipeline& operator=(const Pipeline&) = delete;

	private:
		std::unique_ptr<Shader> shader;
		VkPipelineLayout layout = nullptr;
		VkPipeline graphics_pipeline = nullptr;
		VkRenderPass render_pass = nullptr;
		std::vector<VkDynamicState> dynamic_states;
	};
}