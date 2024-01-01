#pragma once

#include <vector>

#include <vulkan/vulkan.h>

struct Window;

/// <summary>
/// A wrapper around the Vulkan surface.
/// </summary>
struct WindowSurface
{
	friend struct Device;
	friend struct SwapChain;
public:
	WindowSurface(Window* window);
	WindowSurface(const WindowSurface&) = delete;
	WindowSurface& operator=(const WindowSurface&) = delete;
	WindowSurface(const WindowSurface&&) = delete;
	WindowSurface& operator=(const WindowSurface&&) = delete;
	~WindowSurface();

private:
	VkSurfaceKHR vulkan_surface = nullptr;
	VkSurfaceFormatKHR* surface_format = nullptr;
	VkPresentModeKHR present_mode = VK_PRESENT_MODE_FIFO_KHR;
	
	/// <summary>
	/// Select the best present mode from a list, and store it in this struct.
	/// </summary>
	/// <param name="available_present_modes">The present modes to choose from,
	/// which is determined by device support.</param>
	void select_present_mode(
		const std::vector<VkPresentModeKHR>& available_present_modes) noexcept;

	/// <summary>
	/// Select the best format from a list, and store it in this struct.
	/// </summary>
	/// <param name="available_formats">The formats to choose from,
	/// which is determined by device support.</param>
	void select_surface_format(
		const std::vector<VkSurfaceFormatKHR>& available_formats) noexcept;
};