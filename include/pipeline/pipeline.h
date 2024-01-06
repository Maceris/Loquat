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

		void record_command_buffer(const VkCommandBuffer buffer,
			uint32_t image_index) const noexcept;
	private:
		std::unique_ptr<Shader> shader;
		VkPipeline graphics_pipeline = nullptr;
		VkPipelineLayout layout = nullptr;
		VkRenderPass render_pass = nullptr;
		std::vector<VkDynamicState> dynamic_states;
		std::vector<VkFramebuffer> frame_buffers;
	};
}