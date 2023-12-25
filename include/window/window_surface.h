#pragma once

#include <vulkan/vulkan.h>

typedef struct Window;

/// <summary>
/// A wrapper around the Vulkan surface.
/// </summary>
struct WindowSurface
{
public:
	WindowSurface(Window* window);
	WindowSurface(const WindowSurface&) = delete;
	WindowSurface& operator=(const WindowSurface&) = delete;
	WindowSurface(const WindowSurface&&) = delete;
	WindowSurface& operator=(const WindowSurface&&) = delete;
	~WindowSurface();
private:
	VkSurfaceKHR surface = nullptr;
};