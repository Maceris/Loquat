#include "device/device.h"

#include <map>
#include <set>
#include <vector>

#include "debug/logger.h"
#include "main/vulkan_instance.h"
#include "main/global_state.h"
#include "window/window_surface.h"

/// <summary>
/// The extensions that we require on a device.
/// </summary>
const std::vector<const char*> REQUIRED_EXTENSIONS = {
	VK_KHR_SWAPCHAIN_EXTENSION_NAME
};

Device::Device()
{
	select_physical_device();
	select_logical_device();
	create_queues();
	configure_surface();
}

Device::~Device()
{
	//NOTE(ches) queues are implicity destroyed when the logical device is
	//NOTE(ches) physical device gets destroyed implicitly with the instance
	if (logical_device != VK_NULL_HANDLE)
	{
		vkDestroyDevice(logical_device, nullptr);
	}
}

[[nodiscard]] SwapChainSupport
	Device::check_swap_chain_support(const VkPhysicalDevice device)
	const noexcept
{
	const VkSurfaceKHR surface =
		g_global_state->window_state->surface->vulkan_surface;

	SwapChainSupport details;
	vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device, surface,
		&details.capabilities);

	uint32_t format_count;
	vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, 
		&format_count, nullptr);

	if (format_count != 0)
	{
		details.formats.resize(format_count);
		vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface,
			&format_count, details.formats.data());
	}

	uint32_t present_mode_count;
	vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, 
		&present_mode_count, nullptr);

	if (present_mode_count != 0)
	{
		details.present_modes.resize(present_mode_count);
		vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface,
			&present_mode_count, details.present_modes.data());
	}

	return details;
}

void Device::configure_surface() const noexcept
{
	SwapChainSupport swap_chain_support = 
		check_swap_chain_support(physical_device);
	WindowSurface* surface = g_global_state->window_state->surface;

	surface->select_present_mode(swap_chain_support.present_modes);
	surface->select_surface_format(swap_chain_support.formats);
}

void Device::create_queues() noexcept
{
	const uint32_t queue_index = 0;
	vkGetDeviceQueue(logical_device, indices.present_family.value(),
		queue_index, &present_queue);
	vkGetDeviceQueue(logical_device, indices.graphics_family.value(),
		queue_index, &graphics_queue);
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

	const VkSurfaceKHR surface = 
		g_global_state->window_state->surface->vulkan_surface;

	int i = 0;
	for (const auto& queue_family : queue_families)
	{
		if (queue_family.queueFlags & VK_QUEUE_GRAPHICS_BIT)
		{
			indices.graphics_family = i;
		}
		VkBool32 present_support = false;
		vkGetPhysicalDeviceSurfaceSupportKHR(device, i, surface,
			&present_support);

		if (present_support)
		{
			indices.present_family = i;
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

	if (!supports_required_extensions(device))
	{
		return 0;
	}

	SwapChainSupport swap_chain_support = check_swap_chain_support(device);
	if (swap_chain_support.formats.empty() 
		|| swap_chain_support.present_modes.empty())
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

	std::vector<VkDeviceQueueCreateInfo> queueCreateInfos;
	std::set<uint32_t> uniqueQueueFamilies = { 
		indices.graphics_family.value(),
		indices.present_family.value()
	};

	float queuePriority = 1.0f;
	for (uint32_t queueFamily : uniqueQueueFamilies)
	{
		VkDeviceQueueCreateInfo queueCreateInfo{};
		queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
		queueCreateInfo.queueFamilyIndex = queueFamily;
		queueCreateInfo.queueCount = 1;
		queueCreateInfo.pQueuePriorities = &queuePriority;
		queueCreateInfos.push_back(queueCreateInfo);
	}

	VkPhysicalDeviceFeatures device_features{};
	//TODO(ches) Select device features

	VkDeviceCreateInfo create_info{};
	create_info.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
	create_info.pQueueCreateInfos = queueCreateInfos.data();
	create_info.queueCreateInfoCount = 
		static_cast<uint32_t>(queueCreateInfos.size());;
	create_info.pEnabledFeatures = &device_features;
	create_info.enabledExtensionCount = 
		static_cast<uint32_t>(REQUIRED_EXTENSIONS.size());
	create_info.ppEnabledExtensionNames = REQUIRED_EXTENSIONS.data();

	if (ENABLE_VALIDATION_LAYERS)
	{
		create_info.enabledLayerCount =
			static_cast<uint32_t>(VALIDATION_LAYERS.size());
		create_info.ppEnabledLayerNames = VALIDATION_LAYERS.data();
	}
	else
	{
		create_info.enabledLayerCount = 0;
	}

	if (vkCreateDevice(physical_device, &create_info, nullptr,
		&logical_device) != VK_SUCCESS)
	{
		LOG_FATAL("Could not create a logical device");
	}
}

[[nodiscard]]
bool Device::supports_required_extensions(const VkPhysicalDevice device) 
	const noexcept
{
	uint32_t extension_count;
	vkEnumerateDeviceExtensionProperties(device, nullptr, 
		&extension_count, nullptr);

	std::vector<VkExtensionProperties> available_extensions(extension_count);
	vkEnumerateDeviceExtensionProperties(device, nullptr, &extension_count,
		available_extensions.data());

	std::set<std::string> required_extensions(REQUIRED_EXTENSIONS.begin(), 
		REQUIRED_EXTENSIONS.end());

	for (const auto& extension : available_extensions)
	{
		required_extensions.erase(extension.extensionName);
	}

	return required_extensions.empty();
}