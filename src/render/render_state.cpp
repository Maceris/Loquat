#include "render/render_state.h"

#include "main/loquat.h"

namespace loquat
{
	RenderState::RenderState()
	{
		VkSemaphoreCreateInfo semaphore_info{};
		semaphore_info.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

		VkFenceCreateInfo fence_info{};
		fence_info.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
		fence_info.flags = VK_FENCE_CREATE_SIGNALED_BIT;

		const auto& device = g_global_state->device->logical_device;

		if (vkCreateSemaphore(device, &semaphore_info, nullptr, 
			&image_available_semaphore) != VK_SUCCESS 
			|| vkCreateSemaphore(device, &semaphore_info, nullptr, 
				&render_finished_semaphore) != VK_SUCCESS
			|| vkCreateFence(device, &fence_info, nullptr, 
				&frame_in_flight_fence) != VK_SUCCESS)
		{
			LOG_FATAL("Failed to create synchronization objects");
		}
	}

	RenderState::~RenderState()
	{
		const auto& device = g_global_state->device->logical_device;

		vkDestroySemaphore(device, image_available_semaphore, nullptr);
		vkDestroySemaphore(device, render_finished_semaphore, nullptr);
		vkDestroyFence(device, frame_in_flight_fence, nullptr);
	}
}