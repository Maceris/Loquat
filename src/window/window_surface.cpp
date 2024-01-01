#include "window/window_surface.h"

#include "GLFW/glfw3.h"

#include "debug/logger.h"
#include "main/global_state.h"
#include "memory/memory_utils.h"
#include "window/window.h"

WindowSurface::WindowSurface(Window* window)
{
	LOG_ASSERT(g_global_state != nullptr && g_global_state->has_instance()
		&& "We require a Vulkan instance before setting up a surface");
	LOG_ASSERT(window != nullptr && window->glfw_window
		&& "Trying to create a surface for a null window");

	if (glfwCreateWindowSurface(g_global_state->instance, 
		window->glfw_window, 
		NULL, &vulkan_surface) != VK_SUCCESS)
	{
		LOG_FATAL("Failed to create a window surface");
	}
}

WindowSurface::~WindowSurface()
{
	SAFE_DELETE(surface_format);
	vkDestroySurfaceKHR(g_global_state->instance, vulkan_surface, nullptr);
}

void WindowSurface::select_present_mode(
	const std::vector<VkPresentModeKHR>& available_present_modes) noexcept
{
	if (available_present_modes.empty())
	{
		LOG_WARNING("We don't have any available present modes");
	}
	
	for (const auto& availablePresentMode : available_present_modes)
	{
		//NOTE(ches) reduces latency, but maybe at the expense of some power
		if (availablePresentMode == VK_PRESENT_MODE_MAILBOX_KHR)
		{
			present_mode = VK_PRESENT_MODE_MAILBOX_KHR;
			return;
		}
	}
	// NOTE(ches) VK_PRESENT_MODE_FIFO_KHR is guaranteed, and our default
}

void WindowSurface::select_surface_format(
	const std::vector<VkSurfaceFormatKHR>& available_formats) noexcept
{
	LOG_ASSERT(available_formats.size() > 0 
		&& "We require available surface formats");

	surface_format = new VkSurfaceFormatKHR();
	for (const auto& choice : available_formats)
	{
		if (choice.format == VK_FORMAT_B8G8R8A8_SRGB 
			&& choice.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR)
		{
			surface_format->colorSpace = choice.colorSpace;
			surface_format->format = choice.format;
			return;
		}
	}

	surface_format->colorSpace = available_formats[0].colorSpace;
	surface_format->format = available_formats[0].format;
}