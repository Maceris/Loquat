#include "window/window_state.h"

#include "debug/logger.h"
#include "main/global_state.h"
#include "window/window.h"
#include "window/window_surface.h"

namespace loquat
{
	WindowState::~WindowState()
	{
		safe_delete(swap_chain);
		safe_delete(surface);
		safe_delete(window);
	}

	void create_vulkan_window()
	{
		if (g_global_state->has_window())
		{
			LOG_FATAL("We are trying to create a second window");
		}

		WindowState* state = alloc<WindowState>();
		g_global_state->window_state = state;
		state->window = alloc<Window>();
		state->surface = alloc<WindowSurface>(state->window);
	}
}