#include "main/loquat.h"

#define GLFW_INCLUDE_VULKAN
#include "GLFW/glfw3.h"

#include "debug/logger.h"
#include "main/global_state.h"
#include "main/vulkan_instance.h"
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

	create_vulkan_instance();
	g_global_state->window = new Window();


	delete g_global_state;
	glfwTerminate();
	Logger::destroy();
	return 0;
}