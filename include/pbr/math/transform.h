// pbrt is Copyright(c) 1998-2020 Matt Pharr, Wenzel Jakob, and Greg Humphreys.
// The pbrt source code is licensed under the Apache License, Version 2.0.
// SPDX: Apache-2.0

// This file has been modified from the original, original notice is above.

#pragma once

#include "main/loquat.h"
#include "pbr/math/vector_math.h"

namespace loquat
{
	struct Transform
	{
	public:
		[[nodiscard]]
		LOQUAT_CPU_GPU
		inline Ray apply_inverse(const Ray& ray, 
			Float* t_max = nullptr) const noexcept;

		[[nodiscard]]
		LOQUAT_CPU_GPU
		inline Ray apply_inverse(const RayDifferential& ray,
			Float* t_max = nullptr) const noexcept;

		template <typename T>
		[[nodiscard]]
		LOQUAT_CPU_GPU
		inline Vec3<T> apply_inverse(Vec3<T> vec) const noexcept;

		[[nodiscard]]
		std::string to_string() const noexcept;

		Transform() = default;

		LOQUAT_CPU_GPU
		Transform(const Mat4& matrix) noexcept
			: matrix{ matrix }
		{
			std::optional<Mat4> inverse = glm::inverse(matrix);
			if (inverse)
			{
				matrix_inverse = *inverse;
			}
			else
			{
				matrix_inverse = Mat4_NaN;
			}
		}

		LOQUAT_CPU_GPU
		Transform(const Mat4& matrix, const Mat4& matrix_inverse)
			: matrix{ matrix }
			, matrix_inverse{ matrix_inverse }
		{}

		[[nodiscard]]
		LOQUAT_CPU_GPU
		const Mat4& get_matrix() const noexcept
		{
			return matrix;
		}

		[[nodiscard]]
		LOQUAT_CPU_GPU
		const Mat4& get_inverse_matrix() const noexcept
		{
			return matrix_inverse;
		}

		[[nodiscard]]
		LOQUAT_CPU_GPU
		bool operator==(const Transform& t) const noexcept
		{
			return t.matrix == matrix;
		}

		[[nodiscard]]
		LOQUAT_CPU_GPU
		bool operator!=(const Transform& t) const noexcept
		{
			return t.matrix != matrix;
		}

		[[nodiscard]]
		LOQUAT_CPU_GPU
		bool is_identity() const noexcept
		{
			return matrix == Mat4{ 1 };
		}

		[[nodiscard]]
		LOQUAT_CPU_GPU
		bool has_scale(Float tolerance = 1.0e-3f) const noexcept
		{
			Mat3 mat = matrix;
			Float length_a_2 = vector::length_squared(mat * (Vec3f(1, 0, 0)));
			Float length_b_2 = vector::length_squared(mat * (Vec3f(0, 1, 0)));
			Float length_c_2 = vector::length_squared(mat * (Vec3f(0, 0, 1)));
			return (std::abs(length_a_2 - 1) > tolerance
				|| std::abs(length_b_2 - 1) > tolerance
				|| std::abs(length_c_2 - 1) > tolerance);
		}

		template <typename T>
		[[nodiscard]]
		LOQUAT_CPU_GPU
		Point3<T> operator()(Point3<T> point) const noexcept;

		[[nodiscard]]
		LOQUAT_CPU_GPU
		Ray operator()(const Ray& ray, Float* t_max = nullptr)
			const noexcept;

		[[nodiscard]]
		LOQUAT_CPU_GPU
		AABB3f operator()(const AABB3f& bounds) const noexcept;

		[[nodiscard]]
		LOQUAT_CPU_GPU
		Transform operator*(const Transform& t2) const noexcept;

		[[nodiscard]]
		LOQUAT_CPU_GPU
		bool swaps_handedness() const noexcept;

		LOQUAT_CPU_GPU
		explicit Transform(const Frame& frame) noexcept;

		LOQUAT_CPU_GPU
		explicit Transform(Quaternion q) noexcept;

		LOQUAT_CPU_GPU
		explicit operator Quaternion() const noexcept;

		void decompose(Vec3f* transformation, Mat4* rotation, Mat4* scale) 
			const noexcept;

		[[nodiscard]]
		LOQUAT_CPU_GPU
		Interaction operator()(const Interaction& in) const noexcept;

		[[nodiscard]]
		LOQUAT_CPU_GPU
		Interaction apply_inverse(const Interaction& in) const noexcept;

		[[nodiscard]]
		LOQUAT_CPU_GPU
		SurfaceInteraction operator()(const SurfaceInteraction& in)
			const noexcept;

		[[nodiscard]]
		LOQUAT_CPU_GPU
		SurfaceInteraction apply_inverse(const SurfaceInteraction& in)
			const noexcept;

		[[nodiscard]]
		LOQUAT_CPU_GPU
		Point3fi operator()(const Point3fi& point) const noexcept;

		[[nodiscard]]
		LOQUAT_CPU_GPU
		Point3fi apply_inverse(const Point3fi& point) const noexcept;

	private:
		Mat4 matrix;
		Mat4 matrix_inverse;
	};

	class AnimatedTransform
	{
	public:
		AnimatedTransform() = default;

		LOQUAT_CPU_GPU
		AABB3f motion_bounds(const AABB3f& bound) const;

		//TODO(ches) finish this
	};
}
