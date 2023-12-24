#include "window/window_state.h"

#include "debug/logger.h"
#include "main/global_state.h"
#include "window/window.h"
#include "window/window_surface.h"

WindowState::~WindowState()
{
	if (surface)
	{
		delete surface;
		surface = nullptr;
	}
	if (window)
	{
		delete window;
		window = nullptr;
	}
}

void create_vulkan_window()
{
	if (g_global_state->has_window())
	{
		LOG_FATAL("We are trying to create a second window");
	}

	WindowState* state = new WindowState();
	g_global_state->window_state = state;
	state->window = new Window();
	state->surface = new WindowSurface(state->window);
}