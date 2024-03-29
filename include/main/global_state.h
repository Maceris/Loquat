#pragma once

#include <vulkan/vulkan.h>

#include "command/command_buffer.h"
#include "device/device.h"
#include "pipeline/pipeline.h"
#include "render/render_state.h"
#include "window/window_state.h"

namespace loquat
{
	/// <summary>
	/// Global state information about the rendering ecosystem.
	/// </summary>
	struct GlobalState
	{
		VkInstance instance = nullptr;
		WindowState* window_state = nullptr;
		Device* device = nullptr;
		Pipeline* pipeline = nullptr;
		RenderState* render_state = nullptr;

		/// <summary>
		/// Used to signal that we want to exit the application.
		/// </summary>
		std::atomic_bool close_requested = false;

		/// <summary>
		/// The debug messenger, which is generally only set in debug builds.
		/// </summary>
		VkDebugUtilsMessengerEXT debug_messenger = nullptr;

		GlobalState() = default;
		~GlobalState();

		/// <summary>
		/// Check if we currently have an instance. We should not be able to create
		/// two instances.
		/// </summary>
		/// <returns>If there is currently an instance created.</returns>
		[[nodiscard]] bool constexpr has_instance() const noexcept
		{
			return instance != nullptr;
		}

		/// <summary>
		/// Check if we currently have a window. We should not be able to create
		/// two windows.
		/// </summary>
		/// <returns>If there is currently a window created.</returns>
		[[nodiscard]] bool constexpr has_window() const noexcept
		{
			return window_state != nullptr;
		}

		/// <summary>
		/// Checks if the program or glfw indicates the program should close.
		/// </summary>
		/// <returns>If the application should exit.</returns>
		[[nodiscard]] bool should_close() const noexcept;
	};

	extern GlobalState* g_global_state;

	/// <summary>
	/// Set up the debug messenger to use a callback. This should only be called
	/// once, and only if we need the validation layer logging.
	/// </summary>
	/// <returns>Whether we were able to attach the debugger.</returns>
	bool load_debug_messenger();
}