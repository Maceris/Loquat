#include "pipeline/pipeline.h"

#include "main/global_state.h"

namespace loquat
{
	Pipeline::Pipeline(std::unique_ptr<Shader> shader_parameter)
		: dynamic_states{ VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR }
		, shader{ std::move(shader_parameter) }
	{
		const auto& device = g_global_state->device->logical_device;
		const auto& extent = g_global_state->window_state->swap_chain->extent;

		VkPipelineDynamicStateCreateInfo dynamic_state_info{};
		dynamic_state_info.sType =
			VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
		dynamic_state_info.dynamicStateCount =
			static_cast<uint32_t>(dynamic_states.size());
		dynamic_state_info.pDynamicStates = dynamic_states.data();

		VkPipelineVertexInputStateCreateInfo vertex_input{};
		vertex_input.sType =
			VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
		vertex_input.vertexBindingDescriptionCount = 0;
		vertex_input.pVertexBindingDescriptions = nullptr;
		vertex_input.vertexAttributeDescriptionCount = 0;
		vertex_input.pVertexAttributeDescriptions = nullptr;

		VkPipelineInputAssemblyStateCreateInfo input_assembly{};
		input_assembly.sType =
			VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
		input_assembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
		input_assembly.primitiveRestartEnable = VK_FALSE;

		VkViewport viewport{};
		viewport.x = 0.0f;
		viewport.y = 0.0f;
		viewport.width = static_cast<float>(extent.width);
		viewport.height = static_cast<float>(extent.height);
		viewport.minDepth = 0.0f;
		viewport.maxDepth = 1.0f;

		VkRect2D scissor{};
		scissor.offset = { 0, 0 };
		scissor.extent = extent;

		VkPipelineViewportStateCreateInfo viewport_state{};
		viewport_state.sType =
			VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
		//NOTE(ches) using multiple requires enabling a GPU feature
		viewport_state.viewportCount = 1;
		viewport_state.pViewports = &viewport;
		viewport_state.scissorCount = 1;
		viewport_state.pScissors = &scissor;

		VkPipelineRasterizationStateCreateInfo rasterizer{};
		rasterizer.sType =
			VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
		//NOTE(ches) depth clamp requires enabling a GPU feature
		rasterizer.depthClampEnable = VK_FALSE;
		rasterizer.rasterizerDiscardEnable = VK_FALSE;
		//NOTE(ches) modes besides fill requires enabling a GPU feature
		rasterizer.polygonMode = VK_POLYGON_MODE_FILL;
		//NOTE(ches) width above 1 requires enabling a GPU feature
		rasterizer.lineWidth = 1.0f;
		rasterizer.cullMode = VK_CULL_MODE_BACK_BIT;
		rasterizer.frontFace = VK_FRONT_FACE_CLOCKWISE;
		rasterizer.depthBiasEnable = VK_FALSE;
		rasterizer.depthBiasConstantFactor = 0.0f;
		rasterizer.depthBiasClamp = 0.0f;
		rasterizer.depthBiasSlopeFactor = 0.0f;

		VkPipelineMultisampleStateCreateInfo multisampling{};
		multisampling.sType =
			VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
		//NOTE(ches) enabling requires enabling a GPU feature
		multisampling.sampleShadingEnable = VK_FALSE;
		multisampling.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
		multisampling.minSampleShading = 1.0f;
		multisampling.pSampleMask = nullptr;
		multisampling.alphaToCoverageEnable = VK_FALSE;
		multisampling.alphaToOneEnable = VK_FALSE;

		VkPipelineColorBlendAttachmentState color_blend_attachment{};
		color_blend_attachment.colorWriteMask =
			VK_COLOR_COMPONENT_R_BIT
			| VK_COLOR_COMPONENT_G_BIT
			| VK_COLOR_COMPONENT_B_BIT
			| VK_COLOR_COMPONENT_A_BIT;
		color_blend_attachment.blendEnable = VK_FALSE;
		color_blend_attachment.srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
		color_blend_attachment.dstColorBlendFactor =
			VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
		color_blend_attachment.colorBlendOp = VK_BLEND_OP_ADD;
		color_blend_attachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
		color_blend_attachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
		color_blend_attachment.alphaBlendOp = VK_BLEND_OP_ADD;

		VkPipelineColorBlendStateCreateInfo color_blend{};
		color_blend.sType =
			VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
		color_blend.logicOpEnable = VK_FALSE;
		color_blend.logicOp = VK_LOGIC_OP_COPY;
		color_blend.attachmentCount = 1;
		color_blend.pAttachments = &color_blend_attachment;
		color_blend.blendConstants[0] = 0.0f;
		color_blend.blendConstants[1] = 0.0f;
		color_blend.blendConstants[2] = 0.0f;
		color_blend.blendConstants[3] = 0.0f;

		VkPipelineLayoutCreateInfo pipeline_layout_info{};
		pipeline_layout_info.sType =
			VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
		pipeline_layout_info.setLayoutCount = 0;
		pipeline_layout_info.pSetLayouts = nullptr;
		pipeline_layout_info.pushConstantRangeCount = 0;
		pipeline_layout_info.pPushConstantRanges = nullptr;

		if (vkCreatePipelineLayout(device, &pipeline_layout_info, nullptr, 
			&layout) != VK_SUCCESS)
		{
			LOG_FATAL("Failed to create pipeline layout");
		}

		VkAttachmentDescription color_attachment{};
		color_attachment.format = 
			g_global_state->window_state->swap_chain->image_format;
		color_attachment.samples = VK_SAMPLE_COUNT_1_BIT;
		color_attachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
		color_attachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
		color_attachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
		color_attachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
		color_attachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		color_attachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

		//NOTE(ches) color attachment at layout location 0
		VkAttachmentReference color_attachment_ref{};
		color_attachment_ref.attachment = 0;
		color_attachment_ref.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

		VkSubpassDescription subpass{};
		subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
		subpass.colorAttachmentCount = 1;
		subpass.pColorAttachments = &color_attachment_ref;

		VkSubpassDependency dependency{};
		dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
		dependency.dstSubpass = 0;
		dependency.srcStageMask = 
			VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
		dependency.srcAccessMask = 0;
		dependency.dstStageMask = 
			VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
		dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

		VkRenderPassCreateInfo render_pass_info{};
		render_pass_info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
		render_pass_info.attachmentCount = 1;
		render_pass_info.pAttachments = &color_attachment;
		render_pass_info.subpassCount = 1;
		render_pass_info.pSubpasses = &subpass;
		render_pass_info.dependencyCount = 1;
		render_pass_info.pDependencies = &dependency;

		if (vkCreateRenderPass(device, &render_pass_info, nullptr, 
			&render_pass) != VK_SUCCESS)
		{
			LOG_FATAL("Failed to create render pass");
		}

		VkGraphicsPipelineCreateInfo pipeline_info{};
		pipeline_info.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
		pipeline_info.stageCount = 
			static_cast<uint32_t>(this->shader->create_info_list.size());
		pipeline_info.pStages = shader->create_info_list.data();
		pipeline_info.pVertexInputState = &vertex_input;
		pipeline_info.pInputAssemblyState = &input_assembly;
		pipeline_info.pViewportState = &viewport_state;
		pipeline_info.pRasterizationState = &rasterizer;
		pipeline_info.pMultisampleState = &multisampling;
		pipeline_info.pDepthStencilState = nullptr;
		pipeline_info.pColorBlendState = &color_blend;
		pipeline_info.pDynamicState = &dynamic_state_info;
		pipeline_info.layout = layout;
		pipeline_info.renderPass = render_pass;
		pipeline_info.subpass = 0;
		pipeline_info.basePipelineHandle = VK_NULL_HANDLE;
		pipeline_info.basePipelineIndex = -1;

		VkPipelineCache pipeline_cache = VK_NULL_HANDLE;
		uint32_t create_info_count = 1;

		if (vkCreateGraphicsPipelines(device, pipeline_cache,
			create_info_count, &pipeline_info, nullptr, &graphics_pipeline) 
			!= VK_SUCCESS)
		{
			LOG_FATAL("Failed to create graphics pipeline");
		}

		const auto& image_views =
			g_global_state->window_state->swap_chain->image_views;
	

		frame_buffers.resize(image_views.size());
		for (size_t i = 0; i < image_views.size(); ++i)
		{
			VkImageView attachments[] = {
				image_views[i]
			};

			VkFramebufferCreateInfo frame_buffer_info{};
			frame_buffer_info.sType = 
				VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
			frame_buffer_info.renderPass = render_pass;
			frame_buffer_info.attachmentCount = 1;
			frame_buffer_info.pAttachments = attachments;
			frame_buffer_info.width = extent.width;
			frame_buffer_info.height = extent.height;
			frame_buffer_info.layers = 1;

			if (vkCreateFramebuffer(device, &frame_buffer_info, nullptr, 
				&frame_buffers[i]) != VK_SUCCESS)
			{
				LOG_FATAL("Failed to create frame buffer");
			}
		}
	}

	Pipeline::~Pipeline()
	{
		const auto& device = g_global_state->device->logical_device;

		for (auto framebuffer : frame_buffers)
		{
			vkDestroyFramebuffer(device, framebuffer, nullptr);
		}

		vkDestroyPipeline(device, graphics_pipeline, nullptr);
		vkDestroyPipelineLayout(device, layout, nullptr);
		vkDestroyRenderPass(device, render_pass, nullptr);
	}

	void Pipeline::record_command_buffer(const VkCommandBuffer buffer,
		uint32_t image_index) const noexcept
	{
		VkCommandBufferBeginInfo begin_info{};
		begin_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
		begin_info.flags = 0;
		begin_info.pInheritanceInfo = nullptr;

		if (vkBeginCommandBuffer(buffer, &begin_info) != VK_SUCCESS)
		{
			LOG_FATAL("Failed to begin recording a command buffer");
		}

		const auto& extent = g_global_state->window_state->swap_chain->extent;

		VkRenderPassBeginInfo renderPassInfo{};
		renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
		renderPassInfo.renderPass = render_pass;
		renderPassInfo.framebuffer = frame_buffers[image_index];
		renderPassInfo.renderArea.offset = { 0, 0 };
		renderPassInfo.renderArea.extent = extent;

		VkClearValue clearColor = { 0.0f, 0.0f, 0.0f, 1.0f };
		renderPassInfo.clearValueCount = 1;
		renderPassInfo.pClearValues = &clearColor;

		vkCmdBeginRenderPass(buffer, &renderPassInfo, 
			VK_SUBPASS_CONTENTS_INLINE);

		vkCmdBindPipeline(buffer, VK_PIPELINE_BIND_POINT_GRAPHICS,
			graphics_pipeline);

		VkViewport viewport{};
		viewport.x = 0.0f;
		viewport.y = 0.0f;
		viewport.width = static_cast<float>(extent.width);
		viewport.height = static_cast<float>(extent.height);
		viewport.minDepth = 0.0f;
		viewport.maxDepth = 1.0f;
		vkCmdSetViewport(buffer, 0, 1, &viewport);

		VkRect2D scissor{};
		scissor.offset = { 0, 0 };
		scissor.extent = extent;
		vkCmdSetScissor(buffer, 0, 1, &scissor);

		vkCmdDraw(buffer, 3, 1, 0, 0);

		vkCmdEndRenderPass(g_global_state->command_buffer->buffer);

		if (vkEndCommandBuffer(buffer) != VK_SUCCESS)
		{
			LOG_FATAL("Failed recording command buffer");
		}
	}
}