#pragma once

namespace loquat::render
{
	/// <summary>
	/// Render a single frame.
	/// </summary>
	void draw_frame() noexcept;

	/// <summary>
	/// Stop rendering for now, such as when minimized.
	/// </summary>
	void stop_rendering() noexcept;

	/// <summary>
	/// Resume rendering, such as when restoring from being minimized.
	/// </summary>
	void resume_rendering() noexcept;
}