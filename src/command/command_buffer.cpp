#include "command/command_buffer.h"

#include "main/loquat.h"

namespace loquat
{

	CommandBuffer::CommandBuffer()
	{
		VkCommandPoolCreateInfo pool_info{};
		pool_info.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
		pool_info.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
		pool_info.queueFamilyIndex = 
			g_global_state->device->indices.graphics_family.value();

		const auto& device = g_global_state->device->logical_device;

		if (vkCreateCommandPool(device, &pool_info, nullptr, &command_pool)
			!= VK_SUCCESS)
		{
			LOG_FATAL("Failed to create command pool");
		}

		VkCommandBufferAllocateInfo allocate_info{};
		allocate_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
		allocate_info.commandPool = command_pool;
		allocate_info.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
		allocate_info.commandBufferCount = 1;

		if (vkAllocateCommandBuffers(device, &allocate_info, &buffer) 
			!= VK_SUCCESS)
		{
			LOG_FATAL("Failed to create command buffer");
		}

	}

	CommandBuffer::~CommandBuffer()
	{
		//NOTE(ches) Buffers get cleaned up with the pool.
		vkDestroyCommandPool(g_global_state->device->logical_device, 
			command_pool, nullptr);
	}
}