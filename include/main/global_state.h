#pragma once

typedef struct Window;

/// <summary>
/// Global state information about the rendering ecosystem.
/// </summary>
struct GlobalState
{
public:
	Window* window;

	/// <summary>
	/// Check if we currently have a window. We should not be able to create
	/// two windows.
	/// </summary>
	/// <returns>If there is currently a window created.</returns>
	[[nodiscard]] bool has_window() const noexcept
	{
		return window != nullptr;
	}
};

extern GlobalState* g_global_state;
