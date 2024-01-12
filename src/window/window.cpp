#include "window/window.h"

#include "main/global_state.h"
#include "render/render.h"

namespace loquat
{
	constexpr int DEFAULT_WIDTH = 640;
	constexpr int DEFAULT_HEIGHT = 480;

	void callback_iconify(GLFWwindow* window, int iconified) noexcept
	{
		if (iconified == GLFW_TRUE)
		{
			render::stop_rendering();
		}
		else if (iconified == GLFW_FALSE)
		{
			render::resume_rendering();
		}
	}

	void Window::callback_resized(GLFWwindow* glfw_window, int width, 
		int height) noexcept
	{
		Window* window = g_global_state->window_state->window;
		window->width = width;
		window->height = height;
		window->resized = true;
	}

	Window::Window()
		: width{ DEFAULT_WIDTH }
		, height{ DEFAULT_HEIGHT }
	{
		glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
		glfwWindowHint(GLFW_RESIZABLE, GL_TRUE);
		glfw_window = glfwCreateWindow(DEFAULT_WIDTH, DEFAULT_HEIGHT,
			"Loquat", NULL, NULL);
		glfwSetWindowIconifyCallback(glfw_window, callback_iconify);
		glfwSetFramebufferSizeCallback(glfw_window, callback_resized);
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

	void Window::reset_resized() noexcept
	{
		resized = false;
	}

	[[nodiscard]] bool Window::should_close() const noexcept
	{
		return glfwWindowShouldClose(glfw_window);
	}

	[[nodiscard]] bool Window::was_resized() const noexcept
	{
		return resized;
	}
}