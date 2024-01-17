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

	/// <summary>
	/// The main method, called from any entrypoint.
	/// </summary>
	/// <returns></returns>
	int main();

	/// <summary>
	/// Setup the program and rendering information.
	/// </summary>
	void initialize() noexcept;

	/// <summary>
	/// Load a scene from file.
	/// </summary>
	void load_scene() noexcept;

	/// <summary>
	/// Parse the scene from basic scene specifications and prepare to render.
	/// </summary>
	void setup_scene() noexcept;

	/// <summary>
	/// Actually render the scene.
	/// </summary>
	void render_scene() noexcept;

	/// <summary>
	/// Clean up the scene and rendering pipeline, prepare to end the program.
	/// </summary>
	void cleanup() noexcept;
}

int main(int argc, char* argv[])
{
	return loquat::main();
}

namespace loquat
{
	int main()
	{
		initialize();

		//TODO(ches) we probably want to have a dropdown or something to pick
		load_scene();
		setup_scene();

		while (!g_global_state->window_state->window->should_close())
		{
			glfwPollEvents();
			//TODO(ches) we probably want to have a button to render
			render_scene();
		}
		
		cleanup();
		return 0;
	}

	void initialize() noexcept
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
		create_swap_chain();
		create_pipeline();

		create_render_state();

		render::init_UI();
	}

	void load_scene() noexcept
	{
		//TODO(ches) complete
	}

	void setup_scene() noexcept
	{
		//TODO(ches) complete
	}

	void render_scene() noexcept
	{
		//TODO(ches) complete
		render::draw_frame();
	}

	void cleanup() noexcept
	{
		vkDeviceWaitIdle(g_global_state->device->logical_device);
		render::teardown_UI();

		safe_delete(g_global_state);
		glfwTerminate();
		Logger::destroy();
	}

}