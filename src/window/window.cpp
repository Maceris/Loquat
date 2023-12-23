#include "window/window.h"

#include "debug/logger.h"
#include "main/global_state.h"

Window::Window()
{
	if (g_global_state->has_window())
	{
		LOG_FATAL("We are trying to create a second window");
	}

	glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
	glfw_window = glfwCreateWindow(640, 480, "Loquat", NULL, NULL);
}

Window::~Window()
{
	glfwDestroyWindow(glfw_window);
}