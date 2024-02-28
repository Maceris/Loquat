// pbrt is Copyright(c) 1998-2020 Matt Pharr, Wenzel Jakob, and Greg Humphreys.
// The pbrt source code is licensed under the Apache License, Version 2.0.
// SPDX: Apache-2.0

// This file has been modified from the original, original notice is above.

#pragma once

#include <optional>

namespace loquat
{
	/// <summary>
	/// A square matrix of Floats.
	/// </summary>
	/// <typeparam name="Size">The width and height of the matrix.</typeparam>
	template <unsigned int Size>
	using SquareMatrix = glm::mat<Size, Size, Float, glm::defaultp>;

	/// <summary>
	/// A 3x3 matrix of Floats.
	/// </summary>
	using Mat3 = SquareMatrix<3>;

	/// <summary>
	/// A 4x4 matrix of Floats.
	/// </summary>
	using Mat4 = SquareMatrix<4>;

	/// <summary>
	/// A Mat4 full of NaNs.
	/// </summary>
	constexpr Mat4 Mat4_NaN = Mat4{ Vec4_NaN, Vec4_NaN, Vec4_NaN, Vec4_NaN };

	namespace matrix
	{
		template <unsigned int Size>
		[[nodiscard]]
		std::optional<SquareMatrix<Size>> invert(
			const SquareMatrix<Size>& matrix) noexcept;
    
        [[nodiscard]]
        std::optional<Mat3> invert(const Mat3& matrix) noexcept
        {
            return invert(matrix);
        }
    
        [[nodiscard]]
        std::optional<Mat4> invert(const Mat4& matrix) noexcept
        {
            return invert(matrix);
        }
	}
}
