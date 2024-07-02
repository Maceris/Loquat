// pbrt is Copyright(c) 1998-2020 Matt Pharr, Wenzel Jakob, and Greg Humphreys.
// The pbrt source code is licensed under the Apache License, Version 2.0.
// SPDX: Apache-2.0

// This file has been modified from the original, original notice is above.

#pragma once

#include <algorithm>
#include <array>
#include <cmath>
#include <functional>
#include <ostream>
#include <span>
#include <string>
#include <type_traits>
#include <vector>

#include "main/loquat.h"
#include "pbr/struct/containers.h"
#include "pbr/math/math.h"
#include "pbr/math/rng.h"
#include "pbr/math/vector_math.h"

namespace loquat
{
	
	std::array<Float, 3> sample_spherical_triangle(
		const std::array<Point3f, 3>& v, Point3f p, Point2f u,
		Float* pdf = nullptr);
	
	Point2f invert_spherical_triangle_sample(
		const std::array<Point3f, 3>& v, Point3f p, Vec3f w);
	
	Point3f sample_spherical_rectangle(Point3f p, Point3f v00, Vec3f eu,
		Vec3f ev, Point2f u, Float* pdf = nullptr);
	
	Point2f invert_spherical_rectangle_sample(Point3f pRef, Point3f v00,
		Vec3f eu, Vec3f ev, Point3f pRect);
	
	Vec3f sample_henyey_greenstein(Vec3f wo, Float g, Point2f u,
		Float* pdf = nullptr);

	Float sample_catmull_rom(std::span<const Float> nodes,
		std::span<const Float> f, std::span<const Float> cdf, Float sample,
		Float* fval = nullptr,
		Float* pdf = nullptr);
	
	Float sample_catmull_rom_2D(std::span<const Float> nodes1,
		std::span<const Float> nodes2, std::span<const Float> values,
		std::span<const Float> cdf, Float alpha, Float sample,
		Float* fval = nullptr, Float* pdf = nullptr);

	inline Float balance_heuristic(int sample_count1, Float sample1, 
		int sample_count2, Float sample2)
	{
		return (sample_count1 * sample1) 
			/ (sample_count1 * sample1 + sample_count2 * sample2);
	}

	inline Float power_heuristic(int sample_count1, Float sample1,
		int sample_count2, Float sample2)
	{
		Float f = sample_count1 * sample1;
		Float g = sample_count2 * sample2;
		if (is_inf(square(f)))
		{
			return 1;
		}
		return square(f) / (square(f) + square(g));
	}

	inline int sample_discrete(std::span<const Float> weights, Float u,
		Float* pmf = nullptr, Float* u_remapped = nullptr)
	{
		if (weights.empty())
		{
			if (pmf)
			{
				*pmf = 0;
			}
			return -1;
		}
		Float weight_sum = 0;
		for (Float weight : weights)
		{
			weight_sum += weight;
		}

		Float up = u * weight_sum;
		if (up == weight_sum)
		{
			up = next_float_down(up);
		}

		int offset = 0;
		Float sum = 0;
		while (sum + weights[offset] <= up)
		{
			sum += weights[offset];
			offset++;
			LOG_ASSERT(offset < weights.size());
		}

		if (pmf)
		{
			*pmf = weights[offset] / weight_sum;
		}
		if (u_remapped)
		{
			*u_remapped = std::min((up - sum) / weights[offset], ONE_MINUS_EPSILON);
		}

		return offset;
	}

	inline Float linear_PDF(Float x, Float a, Float b)
	{
		LOG_ASSERT(a >= 0 && b >= 0);

		if (x < 0 || x > 1)
		{
			return 0;
		}

		return 2 * lerp(x, a, b) / (a + b);
	}

	inline Float sample_linear(Float sample, Float a, Float b)
	{
		LOG_ASSERT(a >= 0 && b >= 0);
		
		if (sample == 0 && a == 0)
		{
			return 0;
		}

		Float x = sample * (a + b) / (a + std::sqrt(lerp(sample, square(a), square(b))));
		return std::min(x, ONE_MINUS_EPSILON);
	}

	inline Float invert_linear_sample(Float x, Float a, Float b)
	{
		return x * (a * (2 - x) + b * x) / (a + b);
	}

	/// <summary>
	/// Interpolates between 4 values at the four corners of [0, 1]^2,
	/// and returns the probability of that sample.
	/// </summary>
	/// 
	/// <param name="sample">The coordinates we are interested in.</param>
	/// <param name="values">The valuse at (0, 0), (1, 0), (0, 1), and (1, 1) respectively.</param>
	/// <returns></returns>
	inline Float bilinear_PDF(Point2f sample, std::span<const Float> values)
	{
		LOG_ASSERT(values.size() == 4 && "Exactly 4 values are required");
		if (sample.x < 0 || sample.x > 1 || sample.y < 0 || sample.y > 1)
		{
			return 0;
		}
		if (values[0] + values[1] + values[2] + values[3] == 0)
		{
			return 1;
		}
		return 4 *
			(
				(1 - sample[0]) * (1 - sample[1]) * values[0]
				+ sample[0] * (1 - sample[1]) * values[1]
				+ (1 - sample[0]) * sample[1] * values[2]
				+ sample[0] * sample[1] * values[3]
			)
			/ (values[0] + values[1] + values[2] + values[3]);
	}

	/// <summary>
	/// Interpolates between 4 values at the four corners of [0, 1]^2,
	/// and returns the sampled point.
	/// </summary>
	/// 
	/// <param name="sample">The coordinates we are interested in.</param>
	/// <param name="values">The valuse at (0, 0), (1, 0), (0, 1), and (1, 1) respectively.</param>
	/// <returns></returns>
	inline Point2f sample_bilinear(Point2f sample,
		std::span<const Float> values)
	{
		LOG_ASSERT(values.size() == 4 && "Exactly 4 values are required");
		Point2f result;

		result.y = sample_linear(
			sample[1], 
			values[0] + values[1], 
			values[2] + values[3]
		);

		result.x = sample_linear(
			sample[0],
			lerp(result.y, values[0], values[2]),
			lerp(result.y, values[1], values[3])
		);
		
		return result;
	}

	inline Point2f invert_bilinear_sample(Point2f sample,
		std::span<const Float> values)
	{
		return {
			invert_linear_sample(
				sample.x, 
				lerp(sample.y, values[0], values[2]), 
				lerp(sample.y, values[1], values[3])
			),
			invert_linear_sample(
				sample.y,
				values[0] + values[1],
				values[2] + values[3]
			)
		};
	}

	/// <summary>
	/// Calculates the probability of sampling a particular wavelength.
	/// </summary>
	/// <param name="wavelength">The wavelenth, in nanometers.
	/// Should be between 360 and 830 for nonzero results.</param>
	/// <returns>The probability of sampling the provided wavelength.</returns>
	inline Float visible_wavelengths_PDF(Float wavelength)
	{
		if (wavelength < 360 || wavelength > 830)
		{
			return 0;
		}
		return 0.0039398042f / square(std::cosh(0.0072f * (wavelength - 538)));
	}

	/// <summary>
	/// Samples the visible wavelengths (between 360 and 830 nm).
	/// </summary>
	/// <param name="sample">The sample value, in the range [0, 1).</param>
	/// <returns>The wavelength, in nm, between 360 and 830.</returns>
	inline Float sample_visible_wavelengths(Float sample)
	{
		return 538 - 138.888889f * std::atanh(0.85691062f - 1.82750197f * sample);
	}

	inline std::array<Float, 3> sample_uniform_triangle(Point2f sample)
	{
		Float b0;
		Float b1;

		if (sample[0] < sample[1])
		{
			b0 = sample[0] / 2;
			b1 = sample[1] - b0;
		}
		else
		{
			b1 = sample[1] / 2;
			b0 = sample[0] - b1;
		}
		return { b0, b1, 1 - b0 - b1 };
	}

	inline Point2f invert_uniform_triangle_sample(const std::array<Float, 3>& b)
	{
		if (b[0] > b[1])
		{
			return { b[0] + b[1], 2 * b[1] };
		}
		return { 2 * b[0], b[1] + b[0] };
	}

	inline Float tent_PDF(Float x, Float r)
	{
		if (std::abs(x) >= r)
		{
			return 0;
		}
		return 1 / r - std::abs(x) / square(r);
	}

	inline Float invert_tent_sample(Float x, Float r)
	{
		if (x <= 0)
		{
			return (1 - invert_linear_sample(-x / r, 1, 0)) / 2;
		}
		return 0.5f + invert_linear_sample(x / r, 1, 0) / 2;
	}

	inline Float exponential_PDF(Float x, Float a)
	{
		LOG_ASSERT(a > 0);
		return a * std::exp(-1 * x);
	}

	inline Float sample_exponential(Float u, Float a)
	{
		LOG_ASSERT(a > 0);
		return -std::log(1 - u) / a;
	}

	inline Float invert_exponential_sample(Float x, Float a)
	{
		LOG_ASSERT(a > 0);
		return 1 - std::exp(-a * x);
	}

	inline Float normal_PDF(Float x, Float mu = 0, Float sigma = 1)
	{
		return gaussian(x, mu, sigma);
	}

	inline Float sample_normal(Float u, Float mu = 0, Float sigma = 1)
	{
		return mu + SQRT2 * sigma * error_function_inverse(2 * u - 1);
	}

	inline Float invert_normal_sample(Float x, Float mu = 0, Float sigma = 1)
	{
		return 0.5f * (1 + std::erf((x - mu) / (sigma * SQRT2)));
	}

	//TODO(ches) complete this

	template <typename Float = Float>
	class VarianceEstimator
	{

	};

	template <typename T>
	class WeightedReservoirSampler
	{

	};

	class PiecewiseConstant1D
	{
	public:

		size_t size() const noexcept
		{
			return function.size();
		}

		Float sample(Float u, Float* pdf = nullptr, int* offset = nullptr) const noexcept
		{
			int off = find_interval((int)cdf.size(), [&](int index) { return cdf[index] <= u; });
			if (off)
			{
				*offset = off;
			}

			Float du = u - cdf[off];
			if (cdf[off + 1] - cdf[off] > 0)
			{
				du /= cdf[off + 1] - cdf[off];
			}
			LOG_ASSERT(!is_NaN(du));

			if (pdf)
			{
				*pdf = (function_integral > 0) ? function[off] / function_integral : 0;
			}

			return lerp((off + du) / size(), min, max);
		}

		std::vector<Float> function;
		std::vector<Float> cdf;
		Float min;
		Float max;
		Float function_integral = 0;
	};

	class PiecewiseConstant2D
	{
	public:

		Point2f sample(Point2f sample_2d, Float* pdf = nullptr, Point2i* offset = nullptr) const noexcept
		{
			Float pdfs[2];
			Point2i uv;
			Float d1 = marginal_density.sample(sample_2d[1], &pdfs[1], &uv[1]);
			Float d0 = conditional_densities[uv[1]].sample(sample_2d[0], &pdfs[0], &uv[0]);
			if (pdf)
			{
				*pdf = pdfs[0] * pdfs[1];
			}
			if (offset)
			{
				*offset = uv;
			}
			return Point2f{ d0, d1 };
		}

	private:
		AABB2f domain;
		std::vector<PiecewiseConstant1D> conditional_densities;
		PiecewiseConstant1D marginal_density;
	};

	class AliasTable
	{
	};

	class SummedAreaTable
	{

	};

	class WindowedPiecewiseConstant2D
	{

	};

	namespace detail
	{
		template <typename Iterator>
		class IndexingIterator
		{

		};

		template <typename Generator, typename Iterator>
		class IndexingGenerator
		{

		};

		class Uniform1DIter;
		class Uniform2DIter;
		class Uniform3DIter;
		class Hammersley2DIter;
		class Hammersley3DIter;
		class Stratified1DIter;
		class Stratified2DIter;
		class Stratified3DIter;
		template <typename Iterator>
		class RNGIterator;

		template <size_t Dimension = 0>
		class PiecewiseLinear2D
		{

		};

	}
}