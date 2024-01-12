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
		std::vector<VkImage> images;
		std::vector<VkImageView> image_views;
		VkExtent2D extent;
		VkFormat image_format;

	private:
		/// <summary>
		/// Set up the swap chain.
		/// </summary>
		void initialize_swap_chain() noexcept;

		/// <summary>
		/// Set up the image views after initializing.
		/// </summary>
		void initialize_image_views() noexcept;

		/// <summary>
		/// Select an extent to use based on the surface capabilities.
		/// </summary>
		[[nodiscard]] VkExtent2D
			select_extent(const VkSurfaceCapabilitiesKHR& capabilities) const noexcept;
	};

	/// <summary>
	/// Create a swap chain when we don't have one.
	/// </summary>
	void create_swap_chain() noexcept;

	/// <summary>
	/// Clean up the existing swap chain and create a new swap chain,
	/// image views, and frame buffers.
	/// </summary>
	void recreate_swap_chain() noexcept;
}