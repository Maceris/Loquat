#include "device/device.h"

#include <map>
#include <vector>

#include "debug/logger.h"
#include "main/vulkan_instance.h"
#include "main/global_state.h"

Device::Device()
{
	select_physical_device();
	select_logical_device();

	const uint32_t queue_index = 0;
	vkGetDeviceQueue(logical_device, indices.graphics_family.value(), 
		queue_index, &graphics_queue);
}

Device::~Device()
{
	//NOTE(ches) physical device gets destroyed implicitly with the instance
	if (logical_device != VK_NULL_HANDLE)
	{
		vkDestroyDevice(logical_device, nullptr);
	}
}

[[nodiscard]] QueueFamilyIndices
Device::find_queue_families(const VkPhysicalDevice device) const noexcept
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

int Device::rate_device(const VkPhysicalDevice device) const noexcept
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

void Device::select_physical_device() noexcept
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

	physical_device = candidates.rbegin()->second;
}

void Device::select_logical_device() noexcept
{
	indices = find_queue_families(physical_device);

	VkDeviceQueueCreateInfo queue_create_info{};
	queue_create_info.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
	queue_create_info.queueFamilyIndex = indices.graphics_family.value();
	queue_create_info.queueCount = 1;

	float queue_priority = 1.0f;
	queue_create_info.pQueuePriorities = &queue_priority;

	VkPhysicalDeviceFeatures device_features{};
	//TODO(ches) Select device features

	VkDeviceCreateInfo create_info{};
	create_info.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
	create_info.pQueueCreateInfos = &queue_create_info;
	create_info.queueCreateInfoCount = 1;
	create_info.pEnabledFeatures = &device_features;

	create_info.enabledExtensionCount = 0;

	if (ENABLE_VALIDATION_LAYERS) {
		create_info.enabledLayerCount =
			static_cast<uint32_t>(VALIDATION_LAYERS.size());
		create_info.ppEnabledLayerNames = VALIDATION_LAYERS.data();
	}
	else {
		create_info.enabledLayerCount = 0;
	}

	if (vkCreateDevice(physical_device, &create_info, nullptr,
		&logical_device) != VK_SUCCESS) {
		LOG_FATAL("Could not create a logical device");
	}
}