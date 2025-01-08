// pbrt is Copyright(c) 1998-2020 Matt Pharr, Wenzel Jakob, and Greg Humphreys.
// The pbrt source code is licensed under the Apache License, Version 2.0.
// SPDX: Apache-2.0

// This file has been modified from the original, original notice is above.

#pragma once

#include <cstdio>
#include <cmath>
#include <functional>
#include <limits>
#include <memory>

#include "main/loquat.h"

#include "pbr/math/float.h"
#include "pbr/math/hash.h"
#include "pbr/math/math.h"
#include "pbr/math/vector_math.h"
#include "pbr/math/ray.h"
#include "pbr/util/pstd.h"

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
		inline RayDifferential apply_inverse(const RayDifferential& ray,
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
			pstd::optional<Mat4> inv = inverse(matrix);
			if (inv)
			{
				matrix_inverse = *inv;
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
			return matrix.is_identity();
		}

		[[nodiscard]]
		LOQUAT_CPU_GPU
		bool has_scale(Float tolerance = 1.0e-3f) const noexcept
		{
			Float length_a_2 = length_squared((*this)(Vec3f(1, 0, 0)));
			Float length_b_2 = length_squared((*this)(Vec3f(0, 1, 0)));
			Float length_c_2 = length_squared((*this)(Vec3f(0, 0, 1)));
			return (std::abs(length_a_2 - 1) > tolerance
				|| std::abs(length_b_2 - 1) > tolerance
				|| std::abs(length_c_2 - 1) > tolerance);
		}

		template <typename T>
		[[nodiscard]]
		LOQUAT_CPU_GPU
		Point3<T> operator()(Point3<T> point) const noexcept;

        LOQUAT_CPU_GPU
        inline Vec3fi operator()(const Vec3fi& v) const;

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

    LOQUAT_CPU_GPU
    Transform translate(Vec3f delta);

    LOQUAT_CPU_GPU
    Transform scale(Float x, Float y, Float z);

    LOQUAT_CPU_GPU
    Transform rotate_x(Float theta);
    LOQUAT_CPU_GPU
    Transform rotate_y(Float theta);
    LOQUAT_CPU_GPU
    Transform rotate_z(Float theta);

    LOQUAT_CPU_GPU
    Transform look_at(Point3f pos, Point3f look, Vec3f up);

    LOQUAT_CPU_GPU
    Transform orthographic(Float znear, Float zfar);

    LOQUAT_CPU_GPU
    Transform perspective(Float fov, Float znear, Float zfar);

    LOQUAT_CPU_GPU
    inline Transform Inverse(const Transform& t)
    {
        return Transform(t.get_inverse_matrix(), t.get_matrix());
    }

    LOQUAT_CPU_GPU
    inline Transform transpose(const Transform& t)
    {
        return Transform(transpose(t.get_matrix()), transpose(t.get_inverse_matrix()));
    }

    LOQUAT_CPU_GPU
    inline Transform rotate(Float sinTheta, Float cosTheta,
        Vec3f axis)
    {
        Vec3f a = normalize(axis);
        SquareMatrix<4> matrix;
        // Compute rotation of first basis vector
        matrix[0][0] = a.x * a.x + (1 - a.x * a.x) * cosTheta;
        matrix[0][1] = a.x * a.y * (1 - cosTheta) - a.z * sinTheta;
        matrix[0][2] = a.x * a.z * (1 - cosTheta) + a.y * sinTheta;
        matrix[0][3] = 0;

        // Compute rotations of second and third basis vectors
        matrix[1][0] = a.x * a.y * (1 - cosTheta) + a.z * sinTheta;
        matrix[1][1] = a.y * a.y + (1 - a.y * a.y) * cosTheta;
        matrix[1][2] = a.y * a.z * (1 - cosTheta) - a.x * sinTheta;
        matrix[1][3] = 0;

        matrix[2][0] = a.x * a.z * (1 - cosTheta) - a.y * sinTheta;
        matrix[2][1] = a.y * a.z * (1 - cosTheta) + a.x * sinTheta;
        matrix[2][2] = a.z * a.z + (1 - a.z * a.z) * cosTheta;
        matrix[2][3] = 0;

        return Transform(matrix, transpose(matrix));
    }

    LOQUAT_CPU_GPU
    inline Transform rotate(Float theta, Vec3f axis)
    {
        Float sinTheta = std::sin(radians(theta));
        Float cosTheta = std::cos(radians(theta));
        return rotate(sinTheta, cosTheta, axis);
    }

    LOQUAT_CPU_GPU
    inline Transform rotate_from_to(Vec3f from, Vec3f to)
    {
        // Compute intermediate vector for vector reflection
        Vec3f refl;
        if (std::abs(from.x) < 0.72f && std::abs(to.x) < 0.72f)
            refl = Vec3f(1, 0, 0);
        else if (std::abs(from.y) < 0.72f && std::abs(to.y) < 0.72f)
            refl = Vec3f(0, 1, 0);
        else
            refl = Vec3f(0, 0, 1);

        // Initialize matrix _r_ for rotation
        Vec3f u = refl - from, v = refl - to;
        SquareMatrix<4> r;
        for (int i = 0; i < 3; ++i)
            for (int j = 0; j < 3; ++j)
                // Initialize matrix element _r[i][j]_
                r[i][j] = ((i == j) ? 1 : 0) - 2 / dot(u, u) * u[i] * u[j] -
                2 / dot(v, v) * v[i] * v[j] +
                4 * dot(u, v) / (dot(u, u) * dot(v, v)) * v[i] * u[j];

        return Transform(r, transpose(r));
    }

    LOQUAT_CPU_GPU
    inline Vec3fi Transform::operator()(const Vec3fi& v) const
    {
        Float x = Float(v.x), y = Float(v.y), z = Float(v.z);
        Vec3f vOutError;
        if (Point3fi(v).is_exact()) {
            vOutError.x = gamma(3) * (std::abs(matrix[0][0] * x) + std::abs(matrix[0][1] * y) +
                std::abs(matrix[0][2] * z));
            vOutError.y = gamma(3) * (std::abs(matrix[1][0] * x) + std::abs(matrix[1][1] * y) +
                std::abs(matrix[1][2] * z));
            vOutError.z = gamma(3) * (std::abs(matrix[2][0] * x) + std::abs(matrix[2][1] * y) +
                std::abs(matrix[2][2] * z));
        }
        else {
            Vec3f vInError = Point3fi(v).error();
            vOutError.x = (gamma(3) + 1) * (std::abs(matrix[0][0]) * vInError.x +
                std::abs(matrix[0][1]) * vInError.y +
                std::abs(matrix[0][2]) * vInError.z) +
                gamma(3) * (std::abs(matrix[0][0] * x) + std::abs(matrix[0][1] * y) +
                    std::abs(matrix[0][2] * z));
            vOutError.y = (gamma(3) + 1) * (std::abs(matrix[1][0]) * vInError.x +
                std::abs(matrix[1][1]) * vInError.y +
                std::abs(matrix[1][2]) * vInError.z) +
                gamma(3) * (std::abs(matrix[1][0] * x) + std::abs(matrix[1][1] * y) +
                    std::abs(matrix[1][2] * z));
            vOutError.z = (gamma(3) + 1) * (std::abs(matrix[2][0]) * vInError.x +
                std::abs(matrix[2][1]) * vInError.y +
                std::abs(matrix[2][2]) * vInError.z) +
                gamma(3) * (std::abs(matrix[2][0] * x) + std::abs(matrix[2][1] * y) +
                    std::abs(matrix[2][2] * z));
        }

        Float xp = matrix[0][0] * x + matrix[0][1] * y + matrix[0][2] * z;
        Float yp = matrix[1][0] * x + matrix[1][1] * y + matrix[1][2] * z;
        Float zp = matrix[2][0] * x + matrix[2][1] * y + matrix[2][2] * z;

        return Point3fi(Vec3f(xp, yp, zp), vOutError);
    }

    template <typename T>
    LOQUAT_CPU_GPU
    inline Point3<T> Transform::operator()(Point3<T> p) const
    {
        T xp = matrix[0][0] * p.x + matrix[0][1] * p.y + matrix[0][2] * p.z + matrix[0][3];
        T yp = matrix[1][0] * p.x + matrix[1][1] * p.y + matrix[1][2] * p.z + matrix[1][3];
        T zp = matrix[2][0] * p.x + matrix[2][1] * p.y + matrix[2][2] * p.z + matrix[2][3];
        T wp = matrix[3][0] * p.x + matrix[3][1] * p.y + matrix[3][2] * p.z + matrix[3][3];
        if (wp == 1)
            return Point3<T>(xp, yp, zp);
        else
            return Point3<T>(xp, yp, zp) / wp;
    }

    template <typename T>
    LOQUAT_CPU_GPU
    inline Vec3<T> Transform::operator()(Vec3<T> v) const
    {
        return Vec3<T>(matrix[0][0] * v.x + matrix[0][1] * v.y + matrix[0][2] * v.z,
            matrix[1][0] * v.x + matrix[1][1] * v.y + matrix[1][2] * v.z,
            matrix[2][0] * v.x + matrix[2][1] * v.y + matrix[2][2] * v.z);
    }

    template <typename T>
    LOQUAT_CPU_GPU
    inline Normal3<T> Transform::operator()(Normal3<T> n) const
    {
        T x = n.x, y = n.y, z = n.z;
        return Normal3<T>(matrix_inverse[0][0] * x + matrix_inverse[1][0] * y + matrix_inverse[2][0] * z,
            matrix_inverse[0][1] * x + matrix_inverse[1][1] * y + matrix_inverse[2][1] * z,
            matrix_inverse[0][2] * x + matrix_inverse[1][2] * y + matrix_inverse[2][2] * z);
    }

    LOQUAT_CPU_GPU
    inline Ray Transform::operator()(const Ray& r, Float* tMax) const {
        Point3fi o = (*this)(Point3fi(r.origin));
        Vec3f d = (*this)(r.direction);
        // Offset ray origin to edge of error bounds and compute _tMax_
        if (Float lengthSquared = length_squared(d); lengthSquared > 0)
        {
            Float dt = dot(abs(d), o.error()) / lengthSquared;
            o += d * dt;
            if (tMax)
                *tMax -= dt;
        }

        return Ray(Point3f(o), d, r.time, r.medium);
    }

    LOQUAT_CPU_GPU
    inline RayDifferential Transform::operator()(const RayDifferential& r,
        Float* tMax) const
    {
        Ray tr = (*this)(Ray(r), tMax);
        RayDifferential ret(tr.origin, tr.direction, tr.time, tr.medium);
        ret.has_differentials = r.has_differentials;
        ret.x_offset_origin = (*this)(r.x_offset_origin);
        ret.y_offset_origin = (*this)(r.y_offset_origin);
        ret.x_offset_direction = (*this)(r.x_offset_direction);
        ret.y_offset_direction = (*this)(r.y_offset_direction);
        return ret;
    }

    LOQUAT_CPU_GPU
    inline Transform::Transform(const Frame& frame)
        : Transform(SquareMatrix<4>(frame.x.x, frame.x.y, frame.x.z, 0, frame.y.x, frame.y.y,
            frame.y.z, 0, frame.z.x, frame.z.y, frame.z.z, 0, 0, 0, 0,
            1)) {}

    LOQUAT_CPU_GPU
    inline Transform::Transform(Quaternion q)
    {
        Float xx = q.v.x * q.v.x, yy = q.v.y * q.v.y, zz = q.v.z * q.v.z;
        Float xy = q.v.x * q.v.y, xz = q.v.x * q.v.z, yz = q.v.y * q.v.z;
        Float wx = q.v.x * q.w, wy = q.v.y * q.w, wz = q.v.z * q.w;

        matrix_inverse[0][0] = 1 - 2 * (yy + zz);
        matrix_inverse[0][1] = 2 * (xy + wz);
        matrix_inverse[0][2] = 2 * (xz - wy);
        matrix_inverse[1][0] = 2 * (xy - wz);
        matrix_inverse[1][1] = 1 - 2 * (xx + zz);
        matrix_inverse[1][2] = 2 * (yz + wx);
        matrix_inverse[2][0] = 2 * (xz + wy);
        matrix_inverse[2][1] = 2 * (yz - wx);
        matrix_inverse[2][2] = 1 - 2 * (xx + yy);

        // transpose since we are left-handed.
        matrix = transpose(matrix_inverse);
    }

    template <typename T>
    LOQUAT_CPU_GPU
    inline Point3<T> Transform::apply_inverse(Point3<T> p) const
    {
        T x = p.x, y = p.y, z = p.z;
        T xp = (matrix_inverse[0][0] * x + matrix_inverse[0][1] * y) + (matrix_inverse[0][2] * z + matrix_inverse[0][3]);
        T yp = (matrix_inverse[1][0] * x + matrix_inverse[1][1] * y) + (matrix_inverse[1][2] * z + matrix_inverse[1][3]);
        T zp = (matrix_inverse[2][0] * x + matrix_inverse[2][1] * y) + (matrix_inverse[2][2] * z + matrix_inverse[2][3]);
        T wp = (matrix_inverse[3][0] * x + matrix_inverse[3][1] * y) + (matrix_inverse[3][2] * z + matrix_inverse[3][3]);
        CHECK_NE(wp, 0);
        if (wp == 1)
            return Point3<T>(xp, yp, zp);
        else
            return Point3<T>(xp, yp, zp) / wp;
    }

    template <typename T>
    LOQUAT_CPU_GPU
    inline Vec3<T> Transform::apply_inverse(Vec3<T> v) const
    {
        T x = v.x, y = v.y, z = v.z;
        return Vec3<T>(matrix_inverse[0][0] * x + matrix_inverse[0][1] * y + matrix_inverse[0][2] * z,
            matrix_inverse[1][0] * x + matrix_inverse[1][1] * y + matrix_inverse[1][2] * z,
            matrix_inverse[2][0] * x + matrix_inverse[2][1] * y + matrix_inverse[2][2] * z);
    }

    template <typename T>
    LOQUAT_CPU_GPU
    inline Normal3<T> Transform::apply_inverse(Normal3<T> n) const
    {
        T x = n.x, y = n.y, z = n.z;
        return Normal3<T>(matrix[0][0] * x + matrix[1][0] * y + matrix[2][0] * z,
            matrix[0][1] * x + matrix[1][1] * y + matrix[2][1] * z,
            matrix[0][2] * x + matrix[1][2] * y + matrix[2][2] * z);
    }

    LOQUAT_CPU_GPU
    inline Ray Transform::apply_inverse(const Ray& r, Float* tMax) const
    {
        Point3fi o = apply_inverse(Point3fi(r.origin));
        Vec3f d = apply_inverse(r.direction);
        // Offset ray origin to edge of error bounds and compute _tMax_
        Float lengthSquared = length_squared(d);
        if (lengthSquared > 0) {
            Vec3f oError(o.x.width() / 2, o.y.width() / 2, o.z.width() / 2);
            Float dt = dot(abs(d), oError) / lengthSquared;
            o += d * dt;
            if (tMax)
                *tMax -= dt;
        }
        return Ray(Point3f(o), d, r.time, r.medium);
    }

    LOQUAT_CPU_GPU
    inline RayDifferential Transform::apply_inverse(const RayDifferential& r,
        Float* tMax) const
    {
        Ray tr = apply_inverse(Ray(r), tMax);
        RayDifferential ret(tr.origin, tr.direction, tr.time, tr.medium);
        ret.has_differentials = r.has_differentials;
        ret.x_offset_origin = apply_inverse(r.x_offset_origin);
        ret.y_offset_origin = apply_inverse(r.y_offset_origin);
        ret.x_offset_direction = apply_inverse(r.x_offset_direction);
        ret.y_offset_direction = apply_inverse(r.y_offset_direction);
        return ret;
    }

    class AnimatedTransform {
    public:
        AnimatedTransform() = default;
        explicit AnimatedTransform(const Transform& t) : AnimatedTransform(t, 0, t, 1) {}
        AnimatedTransform(const Transform& start_transform, Float startTime,
            const Transform& endTransform, Float endTime);

        LOQUAT_CPU_GPU
        bool is_animated() const { return actually_animated; }

        LOQUAT_CPU_GPU
        Ray apply_inverse(const Ray& r, Float* tMax = nullptr) const;

        LOQUAT_CPU_GPU
        Point3f apply_inverse(Point3f p, Float time) const
        {
            if (!actually_animated)
            {
                return start_transform.apply_inverse(p);
            }
            return interpolate(time).apply_inverse(p);
        }
        LOQUAT_CPU_GPU
        Vec3f apply_inverse(Vec3f v, Float time) const
        {
            if (!actually_animated)
            {
                return start_transform.apply_inverse(v);
            }
            return interpolate(time).apply_inverse(v);
        }
        LOQUAT_CPU_GPU
        Normal3f operator()(Normal3f n, Float time) const;
        LOQUAT_CPU_GPU
        Normal3f apply_inverse(Normal3f n, Float time) const
        {
            if (!actually_animated)
            {
                return start_transform.apply_inverse(n);
            }
            return interpolate(time).apply_inverse(n);
        }
        LOQUAT_CPU_GPU
        Interaction operator()(const Interaction& it) const;
        LOQUAT_CPU_GPU
        Interaction apply_inverse(const Interaction& it) const;
        LOQUAT_CPU_GPU
        SurfaceInteraction operator()(const SurfaceInteraction& it) const;
        LOQUAT_CPU_GPU
        SurfaceInteraction apply_inverse(const SurfaceInteraction& it) const;
        LOQUAT_CPU_GPU
        bool has_scale() const { return start_transform.has_scale() || endTransform.has_scale(); }

        std::string to_string() const;

        LOQUAT_CPU_GPU
        Transform interpolate(Float time) const;

        LOQUAT_CPU_GPU
        Ray operator()(const Ray& r, Float* tMax = nullptr) const;
        LOQUAT_CPU_GPU
        RayDifferential operator()(const RayDifferential& r, Float* tMax = nullptr) const;
        LOQUAT_CPU_GPU
        Point3f operator()(Point3f p, Float time) const;
        LOQUAT_CPU_GPU
        Vec3f operator()(Vec3f v, Float time) const;

        LOQUAT_CPU_GPU
        AABB3f motion_bounds(const AABB3f& b) const;

        LOQUAT_CPU_GPU
        AABB3f bound_point_motion(Point3f p) const;

        // AnimatedTransform Public Members
        Transform start_transform;
        Transform endTransform;
        Float startTime = 0;
        Float endTime = 1;

    private:
        LOQUAT_CPU_GPU
        static void find_zeroes(Float c1, Float c2, Float c3, Float c4, Float c5, Float theta,
            Interval tInterval, pstd::span<Float> zeros, int* nZeros,
            int depth = 8);

        // AnimatedTransform Private Members
        bool actually_animated = false;
        Vec3f T[2];
        Quaternion R[2];
        SquareMatrix<4> S[2];
        bool hasRotation;
        struct DerivativeTerm
        {
            LOQUAT_CPU_GPU
            DerivativeTerm() {}
            LOQUAT_CPU_GPU
            DerivativeTerm(Float c, Float x, Float y, Float z) : kc(c), kx(x), ky(y), kz(z) {}
            Float kc, kx, ky, kz;
            LOQUAT_CPU_GPU
            Float eval(Point3f p) const { return kc + kx * p.x + ky * p.y + kz * p.z; }
        };
        DerivativeTerm c1[3], c2[3], c3[3], c4[3], c5[3];
    };

}

namespace std {

	template <>
	struct hash<loquat::Transform> {
		LOQUAT_CPU_GPU
		size_t operator()(const loquat::Transform& t) const {
			loquat::SquareMatrix<4> matrix = t.get_matrix();
			return loquat::hash(matrix);
		}
	};

}