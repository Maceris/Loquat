#pragma once

#include <optional>
#include <vector>

#include <vulkan/vulkan.h>

/// <summary>
/// A set of all the queue families we care about.
/// </summary>
struct QueueFamilyIndices
{
	std::optional<uint32_t> graphics_family;
	std::optional<uint32_t> present_family;

	[[nodiscard]] bool has_all_values() const noexcept
	{
		return graphics_family.has_value()
			&& present_family.has_value();
	}
};

/// <summary>
/// Information about swap chain support.
/// </summary>
struct SwapChainDetails
{
	VkSurfaceCapabilitiesKHR capabilities;
	std::vector<VkSurfaceFormatKHR> formats;
	std::vector<VkPresentModeKHR> present_modes;
};

/// <summary>
/// Stores information about a device and what we need to interact with it.
/// </summary>
struct Device
{
	VkPhysicalDevice physical_device = VK_NULL_HANDLE;
	VkDevice logical_device = VK_NULL_HANDLE;
	VkQueue graphics_queue = nullptr;
	VkQueue present_queue = nullptr;

	Device();
	Device(const Device&) = delete;
	Device& operator=(const Device&) = delete;
	Device(const Device&&) = delete;
	Device& operator=(const Device&&) = delete;
	~Device();

private:
	QueueFamilyIndices indices;

	/// <summary>
	/// Check what kind of swap chain support the device has.
	/// </summary>
	/// <param name="device">The device we are checking.</param>
	/// <returns></returns>
	[[nodiscard]] SwapChainDetails
		check_swap_chain_support(const VkPhysicalDevice device) const noexcept;

	/// <summary>
	/// Set up the device queues.
	/// </summary>
	void create_queues() noexcept;

	/// <summary>
	/// Find all the queue family indices we care about for a device.
	/// </summary>
	/// <param name="device">The device we are checking.</param>
	/// <returns>What queue family indices we could find.</returns>
	[[nodiscard]] QueueFamilyIndices 
		find_queue_families(const VkPhysicalDevice device) const noexcept;

	/// <summary>
	/// Calculate a score to represent how much desireable a device is. Will
	/// be zero if it's not usable for us.
	/// </summary>
	/// <param name="device">The device we are rating.</param>
	/// <returns>A score for the device.</returns>
	[[nodiscard]]
	int rate_device(const VkPhysicalDevice device) const noexcept;

	/// <summary>
	/// Select a logical device to use among the system devices, and store it
	/// in the game state. Might have a fatal problem if we can't pick a
	/// device.
	/// </summary>
	void select_logical_device() noexcept;

	/// <summary>
	/// Select a physical device to use among the system devices, and store it
	/// in the game state. Might have a fatal problem if we can't find a GPU.
	/// </summary>
	void select_physical_device() noexcept;

	/// <summary>
	/// Checks if a device supports all the extensions we need.
	/// </summary>
	/// <param name="device">The device to check.</param>
	/// <returns>If it supports the required extensions.</returns>
	[[nodiscard]]
	bool supports_required_extensions(const VkPhysicalDevice device) 
		const noexcept;
};

