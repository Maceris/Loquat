#pragma once

#include <memory>
#include <vector>

#include "shader/shader.h"

namespace loquat
{

	constexpr uint32_t MAX_FRAMES_IN_FLIGHT = 2;

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
		
		VkRenderPass render_pass = nullptr;

	private:
		std::unique_ptr<Shader> shader;
		VkPipeline graphics_pipeline = nullptr;
		VkPipelineLayout layout = nullptr;
		std::vector<VkDynamicState> dynamic_states;
		std::vector<VkFramebuffer> frame_buffers;
	};
}