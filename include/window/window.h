#pragma once

#include <atomic>

#define GLFW_INCLUDE_VULKAN
#include "GLFW/glfw3.h"

namespace loquat
{
	/// <summary>
	/// A wrapper around the GLFW window.
	/// </summary>
	struct Window
	{
		friend struct WindowSurface;
	public:
		Window();
		Window(const Window&) = delete;
		Window& operator=(const Window&) = delete;
		Window(const Window&&) = delete;
		Window& operator=(const Window&&) = delete;
		~Window();

		[[nodiscard]] int get_height() const noexcept;
		[[nodiscard]] int get_width() const noexcept;
		[[nodiscard]] bool should_close() const noexcept;

	private:
		GLFWwindow* glfw_window = nullptr;

		std::atomic<int> height;
		std::atomic<int> width;
	};
}