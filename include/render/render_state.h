#pragma once

#include <atomic>
#include <vector>

#include <vulkan/vulkan.h>

namespace loquat
{
	/// <summary>
	/// Tracks the state of rendering.
	/// </summary>
	struct RenderState
	{
		RenderState();
		~RenderState();
		RenderState(const RenderState&) = delete;
		RenderState& operator=(const RenderState&) = delete;
		RenderState(RenderState&&) = delete;
		RenderState& operator=(RenderState&&) = delete;

		std::vector<VkSemaphore> image_available_semaphores;
		std::vector<VkSemaphore> render_finished_semaphores;
		std::vector<VkFence> frame_in_flight_fences;
		std::atomic_bool rendering_active = true;
		uint32_t current_frame = 0;

		std::vector<VkCommandBuffer> command_buffers;
		VkCommandPool command_pool;

		/// <summary>
		/// Fetch the current command buffer.
		/// </summary>
		VkCommandBuffer current_command_buffer() const noexcept;

		/// <summary>
		/// Destroy and recreate all the synchronization objects.
		/// </summary>
		void recreate_synchronization_objects() noexcept;

	private:
		void create_synchronization_objects() noexcept;
		void destroy_synchronization_objects() noexcept;

		void create_command_buffers() noexcept;
		void destroy_command_buffers() noexcept;
	};

	void create_render_state() noexcept;
	void destroy_render_state() noexcept;
}