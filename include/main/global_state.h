#pragma once

#include <vulkan/vulkan.h>

#include "window/window_state.h"

/// <summary>
/// Global state information about the rendering ecosystem.
/// </summary>
struct GlobalState
{
	VkInstance instance = nullptr;
	WindowState* window_state = nullptr;

	GlobalState() = default;
	~GlobalState()
	{
		if (window_state)
		{
			delete window_state;
			window_state = nullptr;
		}
		if (instance)
		{
			vkDestroyInstance(instance, nullptr);
			instance = nullptr;
		}
	}

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
};

extern GlobalState* g_global_state;
