#pragma once

#include <vulkan/vulkan.h>

namespace loquat
{
	/// <summary>
	/// Handles commands and buffers.
	/// </summary>
	struct CommandBuffer
	{
		CommandBuffer();
		~CommandBuffer();
		CommandBuffer(const CommandBuffer&) = delete;
		CommandBuffer& operator=(const CommandBuffer&) = delete;
		CommandBuffer(CommandBuffer&&) = delete;
		CommandBuffer& operator=(CommandBuffer&&) = delete;
		
		VkCommandBuffer buffer;

	private:
		VkCommandPool command_pool;
		
	};
}