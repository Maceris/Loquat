#include "device/physical_device.h"

#include <optional>
#include <map>
#include <vector>

#include "debug/logger.h"
#include "main/global_state.h"

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

QueueFamilyIndices find_queue_families(const VkPhysicalDevice device)
{
	QueueFamilyIndices indices{};

	uint32_t queue_family_count = 0;
	vkGetPhysicalDeviceQueueFamilyProperties(device, &queue_family_count, 
		nullptr);

	std::vector<VkQueueFamilyProperties> queue_families(queue_family_count);
	vkGetPhysicalDeviceQueueFamilyProperties(device, &queue_family_count,
		queue_families.data());

	int i = 0;
	for (const auto& queue_family : queue_families) {
		if (queue_family.queueFlags & VK_QUEUE_GRAPHICS_BIT) {
			indices.graphics_family = i;
		}
		++i;
		if (indices.has_all_values())
		{
			break;
		}
	}

	return indices;
}

int rate_device(const VkPhysicalDevice device) noexcept
{
	int score = 0;

	VkPhysicalDeviceProperties device_properties;
	vkGetPhysicalDeviceProperties(device, &device_properties);
	VkPhysicalDeviceFeatures device_features;
	vkGetPhysicalDeviceFeatures(device, &device_features);

	if (!device_features.geometryShader)
	{
		return 0;
	}

	QueueFamilyIndices queue_families = find_queue_families(device);

	if (!queue_families.has_all_values())
	{
		return 0;
	}

	//NOTE(ches) Discrete GPU is much better than on-chip
	if (device_properties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU)
	{
		score += 1000;
	}

	//NOTE(ches) Largest possible size of a texture
	score += device_properties.limits.maxImageDimension2D;

	return score;
}

void select_physical_device() noexcept
{
	uint32_t device_count = 0;
	vkEnumeratePhysicalDevices(g_global_state->instance, &device_count,
		nullptr);

	if (device_count == 0)
	{
		LOG_FATAL("No GPUs support Vulkan");
	}

	std::vector<VkPhysicalDevice> devices(device_count);
	vkEnumeratePhysicalDevices(g_global_state->instance, &device_count,
		devices.data());

	std::multimap<int, VkPhysicalDevice> candidates;
	for (const auto& device : devices)
	{
		int score = rate_device(device);
		if (score > 0)
		{
			candidates.insert(std::make_pair(score, device));
		}
	}

	if (candidates.empty()) {
		LOG_FATAL("No GPUs are suitable for this program");
	}

	g_global_state->physical_device = candidates.rbegin()->second;
}