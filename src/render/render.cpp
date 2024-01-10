#include "render/render.h"

#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_vulkan.h"

#include "main/loquat.h"
#include "window/window.h"
#include "window/window_state.h"

namespace loquat::render
{
	void draw_UI() noexcept;

	void imgui_result_callback(VkResult err)
	{
		LOG_ASSERT("Issue with ImGui");
	}

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

		draw_UI();

		vkQueuePresentKHR(g_global_state->device->graphics_queue, 
			&present_info);
	}

	void init_UI() noexcept
	{
		IMGUI_CHECKVERSION();
		ImGui::CreateContext();

		ImGuiIO& io = ImGui::GetIO(); (void)io;
		io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
		io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;

		ImGui::StyleColorsDark();
		
		ImGui_ImplGlfw_InitForVulkan(
			g_global_state->window_state->window->glfw_window, true);
		ImGui_ImplVulkan_InitInfo init_info = {};
		init_info.Instance = g_global_state->instance;
		init_info.PhysicalDevice = g_global_state->device->physical_device;
		init_info.Device = g_global_state->device->logical_device;
		init_info.QueueFamily = 
			g_global_state->device->indices.graphics_family.value();
		init_info.Queue = g_global_state->device->graphics_queue;
		init_info.PipelineCache = VK_NULL_HANDLE;
		init_info.DescriptorPool = g_global_state->device->descriptor_pool;
		init_info.Subpass = 0;
		init_info.MinImageCount = 2;
		init_info.ImageCount = MAX_FRAMES_IN_FLIGHT;
		init_info.MSAASamples = VK_SAMPLE_COUNT_1_BIT;
		init_info.Allocator = nullptr;
		init_info.CheckVkResultFn = imgui_result_callback;
		ImGui_ImplVulkan_Init(&init_info, 
			g_global_state->pipeline->render_pass);
	}

	void stop_rendering() noexcept
	{
		g_global_state->render_state->rendering_active = false;
	}

	void resume_rendering() noexcept
	{
		g_global_state->render_state->rendering_active = true;
	}

	void teardown_UI() noexcept
	{
		ImGui_ImplVulkan_Shutdown();
		ImGui_ImplGlfw_Shutdown();
		ImGui::DestroyContext();
	}

	void draw_UI() noexcept
	{
		
	}

}