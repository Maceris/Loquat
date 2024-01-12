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
		
		/// <summary>
		/// Reset the resized flag.
		/// </summary>
		void reset_resized() noexcept;

		[[nodiscard]] bool should_close() const noexcept;

		/// <summary>
		/// If the window has recently been resized. Gets reset once we
		/// regenerate frame buffers.
		/// </summary>
		/// <returns>If the window has been resized.</returns>
		[[nodiscard]] bool was_resized() const noexcept;

		GLFWwindow* glfw_window = nullptr;

		static void callback_resized(GLFWwindow* window, int width, int height)
			noexcept;
	private:
		std::atomic<int> height;
		std::atomic<int> width;
		std::atomic_bool resized = false;
	};
}