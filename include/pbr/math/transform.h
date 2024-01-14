#pragma once

namespace loquat
{
	struct Transform
	{
	public:
		[[nodiscard]]
		inline Ray3f apply_inverse(const Ray3f& ray, 
			Float* t_max = nullptr) const noexcept;

		[[nodiscard]]
		inline Ray3f apply_inverse(const RayDifferential& ray,
			Float* t_max = nullptr) const noexcept;

		template <typename T>
		[[nodiscard]]
		inline Vec3<T> apply_inverse(Vec3<T> vec) const noexcept;

		[[nodiscard]]
		std::string to_string() const noexcept;

		Transform() = default;

		Transform(const Mat4& matrix) noexcept
			: matrix{ matrix }
		{
			std::optional<Mat4> inverse = matrix::invert(matrix);
			if (inverse)
			{
				matrix_inverse = *inverse;
			}
			else
			{
				matrix_inverse = Mat4_NaN;
			}
		}

		Transform(const Mat4& matrix, const Mat4& matrix_inverse)
			: matrix{ matrix }
			, matrix_inverse{ matrix_inverse }
		{}

		[[nodiscard]]
		const Mat4& get_matrix() const noexcept
		{
			return matrix;
		}

		[[nodiscard]]
		const Mat4& get_inverse_matrix() const noexcept
		{
			return matrix_inverse;
		}

		[[nodiscard]]
		bool operator==(const Transform& t) const noexcept
		{
			return t.matrix == matrix;
		}

		[[nodiscard]]
		bool operator!=(const Transform& t) const noexcept
		{
			return t.matrix != matrix;
		}

		[[nodiscard]]
		bool is_identity() const noexcept
		{
			return matrix == Mat4{ 1 };
		}

	private:
		Mat4 matrix;
		Mat4 matrix_inverse;
	};
}