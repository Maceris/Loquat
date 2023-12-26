#pragma once

#include <optional>

#include <vulkan/vulkan.h>

/// <summary>
/// A set of all the queue families we care about.
/// </summary>
struct QueueFamilyIndices
{
	std::optional<uint32_t> graphics_family;

	[[nodiscard]] bool has_all_values() const noexcept
	{
		return graphics_family.has_value();
	}
};

struct Device
{
	VkPhysicalDevice physical_device = VK_NULL_HANDLE;
	VkDevice logical_device = VK_NULL_HANDLE;
	VkQueue graphics_queue = nullptr;

	Device();
	Device(const Device&) = delete;
	Device& operator=(const Device&) = delete;
	Device(const Device&&) = delete;
	Device& operator=(const Device&&) = delete;
	~Device();

private:
	QueueFamilyIndices indices;

	[[nodiscard]]
	QueueFamilyIndices 
		find_queue_families(const VkPhysicalDevice device) const noexcept;

	[[nodiscard]]
	int rate_device(const VkPhysicalDevice device) const noexcept;

	/// <summary>
	/// Select a logical device to use among the system devices, and store it
	/// in the game state. Might have a fatal problem if we can't pick a device.
	/// </summary>
	void select_logical_device() noexcept;

	/// <summary>
	/// Select a physical device to use among the system devices, and store it
	/// in the game state. Might have a fatal problem if we can't find a GPU.
	/// </summary>
	void select_physical_device() noexcept;
};

