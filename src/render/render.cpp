#include "render/render.h"

#include "main/loquat.h"

namespace loquat::render
{
	void draw_frame() noexcept
	{
		const RenderState* render_state = g_global_state->render_state;
		
		if (!render_state->rendering_active)
		{
			return;
		}

		const auto& device = g_global_state->device->logical_device;
		const SwapChain* swap_chain = g_global_state->window_state->swap_chain;

		vkWaitForFences(device, 1, &render_state->frame_in_flight_fence,
			VK_TRUE, UINT64_MAX);
		vkResetFences(device, 1, &render_state->frame_in_flight_fence);

		uint32_t image_index;
		vkAcquireNextImageKHR(device, swap_chain->vulkan_swap_chain, 
			UINT64_MAX, render_state->image_available_semaphore, 
			VK_NULL_HANDLE, &image_index);

		const VkCommandBuffer buffer = g_global_state->command_buffer->buffer;
		vkResetCommandBuffer(buffer, 0);
		g_global_state->pipeline->record_command_buffer(buffer, image_index);

		VkSubmitInfo submit_info{};
		submit_info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

		VkSemaphore wait_semaphores[] = { 
			render_state->image_available_semaphore
		};
		VkPipelineStageFlags wait_stages[] = { 
			VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT
		};
		submit_info.waitSemaphoreCount = 1;
		submit_info.pWaitSemaphores = wait_semaphores;
		submit_info.pWaitDstStageMask = wait_stages;
		submit_info.commandBufferCount = 1;
		submit_info.pCommandBuffers = &buffer;

		VkSemaphore signal_semaphores[] = { 
			render_state->render_finished_semaphore
		};
		submit_info.signalSemaphoreCount = 1;
		submit_info.pSignalSemaphores = signal_semaphores;
		if (vkQueueSubmit(g_global_state->device->graphics_queue, 1, 
			&submit_info, render_state->frame_in_flight_fence) != VK_SUCCESS)
		{
			LOG_FATAL("Failed to submit draw command buffer");
		}

		VkPresentInfoKHR present_info{};
		present_info.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
		present_info.waitSemaphoreCount = 1;
		present_info.pWaitSemaphores = signal_semaphores;

		VkSwapchainKHR swapChains[] = { 
			g_global_state->window_state->swap_chain->vulkan_swap_chain 
		};
		present_info.swapchainCount = 1;
		present_info.pSwapchains = swapChains;
		present_info.pImageIndices = &image_index;
		present_info.pResults = nullptr;

		vkQueuePresentKHR(g_global_state->device->graphics_queue, 
			&present_info);
	}

	void stop_rendering() noexcept
	{
		g_global_state->render_state->rendering_active = false;
	}

	void resume_rendering() noexcept
	{
		g_global_state->render_state->rendering_active = true;
	}

}