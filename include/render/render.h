#pragma once

namespace loquat::render
{
	/// <summary>
	/// Render a single frame.
	/// </summary>
	void draw_frame() noexcept;

	/// <summary>
	/// Prepare things for the UI.
	/// </summary>
	void init_UI() noexcept;

	/// <summary>
	/// Stop rendering for now, such as when minimized.
	/// </summary>
	void stop_rendering() noexcept;

	/// <summary>
	/// Resume rendering, such as when restoring from being minimized.
	/// </summary>
	void resume_rendering() noexcept;

	/// <summary>
	/// Destroy the UI while we are shutting down.
	/// </summary>
	void teardown_UI() noexcept;
}