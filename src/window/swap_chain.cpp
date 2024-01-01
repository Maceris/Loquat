#include "window/swap_chain.h"

#include <cstdint>
#include <limits>
#include <algorithm>

#include "debug/logger.h"
#include "main/global_state.h"
#include "window/window.h"
#include "window/window_surface.h"

namespace loquat
{
	SwapChain::SwapChain()
	{
		const WindowSurface& surface = *g_global_state->window_state->surface;
		const Device& device = *g_global_state->device;
		const SwapChainSupport& support =
			device.check_swap_chain_support(device.physical_device);

		VkExtent2D extent = select_extent(support.capabilities);
		uint32_t image_count = support.capabilities.minImageCount + 1;
		if (support.capabilities.maxImageCount > 0
			&& image_count > support.capabilities.maxImageCount)
		{
			image_count = support.capabilities.maxImageCount;
		}

		VkSwapchainCreateInfoKHR create_info{};
		create_info.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
		create_info.surface = surface.vulkan_surface;
		create_info.minImageCount = image_count;
		create_info.imageFormat = surface.surface_format->format;
		create_info.imageColorSpace = surface.surface_format->colorSpace;
		create_info.imageExtent = extent;
		create_info.imageArrayLayers = 1;
		create_info.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
		create_info.preTransform = support.capabilities.currentTransform;
		create_info.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
		create_info.presentMode = surface.present_mode;
		create_info.clipped = VK_TRUE;
		//TODO(ches) This will eventually need handling
		create_info.oldSwapchain = VK_NULL_HANDLE;

		QueueFamilyIndices indices =
			device.find_queue_families(device.physical_device);

		uint32_t queue_family_indices[] = {
			indices.graphics_family.value(),
			indices.present_family.value()
		};

		if (indices.graphics_family != indices.present_family)
		{
			create_info.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
			create_info.queueFamilyIndexCount = 2;
			create_info.pQueueFamilyIndices = queue_family_indices;
		}
		else
		{
			create_info.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
			create_info.queueFamilyIndexCount = 0;
			create_info.pQueueFamilyIndices = nullptr;
		}
		if (vkCreateSwapchainKHR(device.logical_device, &create_info, nullptr,
			&vulkan_swap_chain) != VK_SUCCESS)
		{
			LOG_FATAL("Failed to create swap chain");
		}
	}

	SwapChain::~SwapChain()
	{
		vkDestroySwapchainKHR(g_global_state->device->logical_device,
			vulkan_swap_chain, nullptr);
	}

	[[nodiscard]] VkExtent2D
		SwapChain::select_extent(const VkSurfaceCapabilitiesKHR& capabilities)
		const noexcept
	{
		if (capabilities.currentExtent.width
			!= std::numeric_limits<uint32_t>::max())
		{
			return capabilities.currentExtent;
		}
		else
		{
			Window* window = g_global_state->window_state->window;
			int width = window->get_width();
			int height = window->get_height();

			VkExtent2D actualExtent = {
				static_cast<uint32_t>(width),
				static_cast<uint32_t>(height)
			};

			actualExtent.width = std::clamp(actualExtent.width,
				capabilities.minImageExtent.width,
				capabilities.maxImageExtent.width);
			actualExtent.height = std::clamp(actualExtent.height,
				capabilities.minImageExtent.height,
				capabilities.maxImageExtent.height);

			return actualExtent;
		}
	}
}