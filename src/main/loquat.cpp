#define GLFW_INCLUDE_VULKAN
#include "GLFW/glfw3.h"

#include "debug/logger.h"
#include "main/global_state.h"
#include "main/loquat.h"
#include "main/vulkan_instance.h"
#include "window/swap_chain.h"
#include "window/window.h"

GlobalState* g_global_state = new GlobalState();

namespace loquat
{
	int main();
}

int main(int argc, char* argv[])
{
	return loquat::main();
}

namespace loquat
{
	int main()
	{
		Logger::init();
		Logger::set_display_flags("Debug", FLAG_WRITE_TO_DEBUGGER);

		glfwInit();
		if (!glfwVulkanSupported())
		{
			LOG_FATAL("Vulkan is not supported on this system!");
		}

		create_vulkan_instance();
		create_vulkan_window();
		g_global_state->device = new Device();
		g_global_state->window_state->swap_chain = new SwapChain();

		while (!g_global_state->window_state->window->should_close())
		{
			glfwPollEvents();
		}

		delete g_global_state;
		glfwTerminate();
		Logger::destroy();
		return 0;
	}
}