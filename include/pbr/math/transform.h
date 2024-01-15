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

		[[nodiscard]]
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
		Point3<T> operator()(Point3<T> point) const noexcept;

		[[nodiscard]]
		Ray3f operator()(const Ray3f& ray, Float* t_max = nullptr)
			const noexcept;

		[[nodiscard]]
		AABB3f operator()(const AABB3f& bounds) const noexcept;

		[[nodiscard]]
		Transform operator*(const Transform& t2) const noexcept;

		[[nodiscard]]
		bool swaps_handedness() const noexcept;

		explicit Transform(const Frame& frame) noexcept;

		explicit Transform(Quaternion q) noexcept;

		explicit operator Quaternion() const noexcept;

		void decompose(Vec3f* transformation, Mat4* rotation, Mat4* scale) 
			const noexcept;

		[[nodiscard]]
		Interaction operator()(const Interaction& in) const noexcept;

		[[nodiscard]]
		Interaction apply_inverse(const Interaction& in) const noexcept;

		[[nodiscard]]
		SurfaceInteraction operator()(const SurfaceInteraction& in)
			const noexcept;

		[[nodiscard]]
		SurfaceInteraction apply_inverse(const SurfaceInteraction& in)
			const noexcept;

		[[nodiscard]]
		Vec3fi operator()(const Vec3fi& point) const noexcept;

		[[nodiscard]]
		Point3fi apply_inverse(const Point3fi& point) const noexcept;

	private:
		Mat4 matrix;
		Mat4 matrix_inverse;
	};
}