#pragma once

#include <vulkan/vulkan.h>

typedef struct Window;
typedef struct WindowSurface;

/// <summary>
/// Stores the state information for a window.
/// </summary>
struct WindowState
{
	Window* window = nullptr;
	WindowSurface* surface = nullptr;
	VkSwapchainKHR swap_chain = nullptr;

	WindowState() = default;
	WindowState(const WindowState&) = delete;
	WindowState& operator=(const WindowState&) = delete;
	WindowState(const WindowState&&) = delete;
	WindowState& operator=(const WindowState&&) = delete;
	~WindowState();

	/// <summary>
	/// Check if we currently have a window. We should not be able to create
	/// two windows.
	/// </summary>
	/// <returns>If there is currently a window created.</returns>
	[[nodiscard]] bool constexpr has_window() const noexcept
	{
		return window != nullptr;
	}
};

void create_vulkan_window();
