#include "window/window_surface.h"

#include "GLFW/glfw3.h"

#include "debug/logger.h"
#include "main/global_state.h"
#include "window/window.h"

WindowSurface::WindowSurface(Window* window)
{
	LOG_ASSERT(g_global_state != nullptr && g_global_state->has_instance()
		&& "We require a Vulkan instance before setting up a surface");
	LOG_ASSERT(window != nullptr && window->glfw_window
		&& "Trying to create a surface for a null window");

	if (glfwCreateWindowSurface(g_global_state->instance, 
		window->glfw_window, 
		NULL, &surface) != VK_SUCCESS)
	{
		LOG_FATAL("Failed to create a window surface");
	}
}

WindowSurface::~WindowSurface()
{
	vkDestroySurfaceKHR(g_global_state->instance, surface, nullptr);
}