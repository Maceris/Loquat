#include "main/loquat.h"

#define GLFW_INCLUDE_VULKAN
#include "GLFW/glfw3.h"

#include "debug/logger.h"

int main(int argc, char* argv[])
{
	Logger::init();
	if (!glfwVulkanSupported())
	{
		LOG_FATAL("Vulkan is not supported on this system!");
	}
	LOG_INFO("Main method");
	Logger::destroy();
	return 0;
}