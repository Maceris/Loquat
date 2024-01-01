#pragma once

#include <vulkan/vulkan.h>

struct SwapChain;
struct Window;
struct WindowSurface;

/// <summary>
/// Stores the state information for a window.
/// </summary>
struct WindowState
{
	Window* window = nullptr;
	WindowSurface* surface = nullptr;
	SwapChain* swap_chain = nullptr;

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

/// <summary>
/// Create a vulkan window, including associated things like the window 
/// surface.
/// </summary>
void create_vulkan_window();
