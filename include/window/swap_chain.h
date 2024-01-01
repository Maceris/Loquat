#pragma once

#include <vector>

#include <vulkan/vulkan.h>

namespace loquat
{
	/// <summary>
	/// Information about swap chain support.
	/// </summary>
	struct SwapChainSupport
	{
		VkSurfaceCapabilitiesKHR capabilities{};
		std::vector<VkSurfaceFormatKHR> formats;
		std::vector<VkPresentModeKHR> present_modes;
	};

	/// <summary>
	/// Handles the lifecycle of a swap chain.
	/// </summary>
	struct SwapChain
	{
	public:
		SwapChain();
		~SwapChain();
		SwapChain(const SwapChain&) = delete;
		SwapChain& operator=(const SwapChain&) = delete;
		SwapChain(SwapChain&&) = delete;
		SwapChain& operator=(SwapChain&&) = delete;

		VkSwapchainKHR vulkan_swap_chain;

	private:
		[[nodiscard]] VkExtent2D
			select_extent(const VkSurfaceCapabilitiesKHR& capabilities) const noexcept;
	};
}