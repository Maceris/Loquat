#pragma once

#define GLFW_INCLUDE_VULKAN
#include "GLFW/glfw3.h"

/// <summary>
/// A wrapper around the GLFW window that handles lifecycle.
/// </summary>
class Window
{
	friend class WindowSurface;
public:
	Window();
	Window(const Window&) = delete;
	Window& operator=(const Window&) = delete;
	Window(const Window&&) = delete;
	Window& operator=(const Window&&) = delete;
	~Window();
private:
	GLFWwindow* glfw_window = nullptr;
};