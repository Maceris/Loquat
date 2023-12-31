#include <filesystem>

#define GLFW_INCLUDE_VULKAN
#include "GLFW/glfw3.h"

#include "debug/logger.h"
#include "main/loquat.h"
#include "main/vulkan_instance.h"
#include "render/render.h"
#include "resource/resource_file_folder.h"
#include "window/swap_chain.h"
#include "window/window.h"

namespace loquat
{
	Allocator* g_allocator = new Allocator();
	GlobalState* g_global_state = alloc<GlobalState>();
	ResourceCache* g_resource_cache;
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

		std::filesystem::path resource_path{ 
			std::filesystem::current_path().append("resources") };
		std::filesystem::path full_resource_path =
			std::filesystem::canonical(resource_path);
		ResourceFileFolder* resource_folder =
			alloc<ResourceFileFolder>(full_resource_path.string());
		g_resource_cache = alloc<ResourceCache>(50, resource_folder);

		if (!g_resource_cache->init())
		{
			LOG_FATAL("Failed to initialize the resource cache. Is there enough memory?");
		}

		create_vulkan_instance();
		create_vulkan_window();
		g_global_state->device = alloc<Device>();
		g_global_state->window_state->swap_chain = alloc<SwapChain>();

		auto shader_stages = std::initializer_list<ShaderStage>{
			ShaderStage{ ShaderType::vertex, "shaders/simple.vert.spv" },
				ShaderStage{ ShaderType::fragment, "shaders/simple.frag.spv" }
		};
		g_global_state->pipeline = alloc<Pipeline>(
			std::make_unique<Shader>(shader_stages));

		g_global_state->command_buffer = alloc<CommandBuffer>();
		g_global_state->render_state = alloc<RenderState>();

		while (!g_global_state->window_state->window->should_close())
		{
			glfwPollEvents();
			render::draw_frame();
		}
		
		vkDeviceWaitIdle(g_global_state->device->logical_device);

		safe_delete(g_global_state);
		glfwTerminate();
		Logger::destroy();
		return 0;
	}
}