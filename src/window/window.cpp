#include "window/window.h"

#include "main/global_state.h"

Window::Window()
{
	glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
	glfw_window = glfwCreateWindow(640, 480, "Loquat", NULL, NULL);
}

Window::~Window()
{
	glfwDestroyWindow(glfw_window);
}