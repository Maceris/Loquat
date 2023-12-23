#pragma once

#define GLFW_INCLUDE_VULKAN
#include "GLFW/glfw3.h"

class Window
{
public:
	Window();
	Window(const Window&) = delete;
	Window& operator=(const Window&) = delete;
	Window(const Window&&) = delete;
	Window& operator=(const Window&&) = delete;
	~Window();
private:
	GLFWwindow* glfw_window;
};