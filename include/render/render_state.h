#pragma once

#include <atomic>

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

		VkSemaphore image_available_semaphore = nullptr;
		VkSemaphore render_finished_semaphore = nullptr;
		VkFence frame_in_flight_fence = nullptr;
		std::atomic_bool rendering_active = true;
	};
}