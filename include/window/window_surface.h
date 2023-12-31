#pragma once

#include <vulkan/vulkan.h>

struct Window;

/// <summary>
/// A wrapper around the Vulkan surface.
/// </summary>
struct WindowSurface
{
	friend struct Device;
public:
	WindowSurface(Window* window);
	WindowSurface(const WindowSurface&) = delete;
	WindowSurface& operator=(const WindowSurface&) = delete;
	WindowSurface(const WindowSurface&&) = delete;
	WindowSurface& operator=(const WindowSurface&&) = delete;
	~WindowSurface();
private:
	VkSurfaceKHR vulkan_surface = nullptr;
};