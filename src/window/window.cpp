#include "window/window.h"

#include "main/global_state.h"

constexpr int DEFAULT_WIDTH = 640;
constexpr int DEFAULT_HEIGHT = 480;

Window::Window()
	: width{ DEFAULT_WIDTH }
	, height{ DEFAULT_HEIGHT }
{
	glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
	glfw_window = glfwCreateWindow(DEFAULT_WIDTH, DEFAULT_HEIGHT, 
		"Loquat", NULL, NULL);
}

Window::~Window()
{
	glfwDestroyWindow(glfw_window);
}

[[nodiscard]] int Window::get_height() const noexcept
{
	return height;
}

[[nodiscard]] int Window::get_width() const noexcept
{
	return width;
}

[[nodiscard]] bool Window::should_close() const noexcept
{
	return glfwWindowShouldClose(glfw_window);
}