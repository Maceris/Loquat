#include "window/window_state.h"

#include "debug/logger.h"
#include "main/global_state.h"
#include "memory/memory_utils.h"
#include "window/window.h"
#include "window/window_surface.h"

namespace loquat
{
	WindowState::~WindowState()
	{
		SAFE_DELETE(swap_chain);
		SAFE_DELETE(surface);
		SAFE_DELETE(window);
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
}