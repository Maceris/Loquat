#include "render/render_state.h"

#include "main/loquat.h"

namespace loquat
{
	RenderState::RenderState()
	{
		create_synchronization_objects();
		create_command_buffers();
	}

	RenderState::~RenderState()
	{
		destroy_synchronization_objects();
		destroy_command_buffers();
	}

	VkCommandBuffer RenderState::current_command_buffer() const noexcept
	{
		return command_buffers[current_frame];
	}

	void RenderState::recreate_synchronization_objects() noexcept
	{
		destroy_synchronization_objects();
		create_synchronization_objects();
	}

	void RenderState::create_synchronization_objects() noexcept
	{
		VkSemaphoreCreateInfo semaphore_info{};
		semaphore_info.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

		VkFenceCreateInfo fence_info{};
		fence_info.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
		fence_info.flags = VK_FENCE_CREATE_SIGNALED_BIT;

		image_available_semaphores.resize(MAX_FRAMES_IN_FLIGHT);
		render_finished_semaphores.resize(MAX_FRAMES_IN_FLIGHT);
		frame_in_flight_fences.resize(MAX_FRAMES_IN_FLIGHT);

		auto& device = g_global_state->device->logical_device;

		for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; ++i)
		{
			if (vkCreateSemaphore(device, &semaphore_info, nullptr,
				&image_available_semaphores[i]) != VK_SUCCESS
				|| vkCreateSemaphore(device, &semaphore_info, nullptr,
					&render_finished_semaphores[i]) != VK_SUCCESS
				|| vkCreateFence(device, &fence_info, nullptr,
					&frame_in_flight_fences[i]) != VK_SUCCESS)
			{
				LOG_FATAL("Failed to create synchronization objects");
			}
		}
	}

	void RenderState::destroy_synchronization_objects() noexcept
	{
		auto& device = g_global_state->device->logical_device;

		for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; ++i)
		{
			vkDestroySemaphore(device, image_available_semaphores[i], nullptr);
			vkDestroySemaphore(device, render_finished_semaphores[i], nullptr);
			vkDestroyFence(device, frame_in_flight_fences[i], nullptr);
		}
	}

	void RenderState::create_command_buffers() noexcept
	{
		VkCommandPoolCreateInfo pool_info{};
		pool_info.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
		pool_info.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
		pool_info.queueFamilyIndex =
			g_global_state->device->indices.graphics_family.value();

		auto& device = g_global_state->device->logical_device;

		if (vkCreateCommandPool(device, &pool_info, nullptr, 
			&command_pool) != VK_SUCCESS)
		{
			LOG_FATAL("Failed to create command pool");
		}

		command_buffers.resize(MAX_FRAMES_IN_FLIGHT);
		
		VkCommandBufferAllocateInfo allocate_info{};
		allocate_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
		allocate_info.commandPool = command_pool;
		allocate_info.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
		allocate_info.commandBufferCount = MAX_FRAMES_IN_FLIGHT;

		if (vkAllocateCommandBuffers(device, &allocate_info, 
			command_buffers.data())
			!= VK_SUCCESS)
		{
			LOG_FATAL("Failed to create command buffer");
		}
	}

	void RenderState::destroy_command_buffers() noexcept
	{
		//NOTE(ches) Buffers get cleaned up with the pool.
		vkDestroyCommandPool(g_global_state->device->logical_device,
			command_pool, nullptr);
	}

	void create_render_state() noexcept
	{
		g_global_state->render_state = alloc<RenderState>();
	}

	void destroy_render_state() noexcept
	{
		safe_delete(g_global_state->render_state);
	}
}