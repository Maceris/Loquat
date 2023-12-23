#pragma once

#include <vulkan/vulkan.h>

typedef struct Window;

/// <summary>
/// Global state information about the rendering ecosystem.
/// </summary>
struct GlobalState
{
public:
	VkInstance instance = nullptr;
	Window* window = nullptr;

	GlobalState() = default;
	~GlobalState()
	{
		if (window)
		{
			delete window;
			window = nullptr;
		}
		if (instance)
		{
			vkDestroyInstance(instance, nullptr);
			instance = nullptr;
		}
	}

	/// <summary>
	/// Check if we currently have an instance. We should not be able to create
	/// two instances.
	/// </summary>
	/// <returns>If there is currently an instance created.</returns>
	[[nodiscard]] bool constexpr has_instance() const noexcept
	{
		return instance != nullptr;
	}

	/// <summary>
	/// Check if we currently have a window. We should not be able to create
	/// two windows.
	/// </summary>
	/// <returns>If there is currently a window created.</returns>
	[[nodiscard]] bool constexpr has_window() const noexcept
	{
		return window != nullptr;
	}
};

extern GlobalState* g_global_state;
