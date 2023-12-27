#pragma once

#include <optional>

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
	/// Set up the device queues.
	/// </summary>
	void create_queues() noexcept;

	[[nodiscard]]
	QueueFamilyIndices 
		find_queue_families(const VkPhysicalDevice device) const noexcept;

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

