#include "main/loquat.h"

#define GLFW_INCLUDE_VULKAN
#include "GLFW/glfw3.h"

#include "debug/logger.h"
#include "main/global_state.h"
#include "window/window.h"

GlobalState* g_global_state = new GlobalState();

int main(int argc, char* argv[])
{
	Logger::init();
	glfwInit();
	if (!glfwVulkanSupported())
	{
		LOG_FATAL("Vulkan is not supported on this system!");
	}

	g_global_state->window = new Window();

	Logger::destroy();
	return 0;
}