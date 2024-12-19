// pbrt is Copyright(c) 1998-2020 Matt Pharr, Wenzel Jakob, and Greg Humphreys.
// The pbrt source code is licensed under the Apache License, Version 2.0.
// SPDX: Apache-2.0

// This file has been modified from the original, original notice is above.

#pragma once

#include "main/loquat.h"

#include "pbr/base/filter.h"
#include "pbr/math/math.h"
#include "pbr/math/sampling.h"
#include "pbr/struct/containers.h"

namespace loquat
{
	struct FilterSample
	{
		Point2f sample;
		Float weight;
	};

	class FilterSampler
	{
	public:
		FilterSampler(Filter filter, Allocator allocator = {});

		[[nodiscard]]
		std::string to_string() const noexcept;

		LOQUAT_CPU_GPU
		FilterSample sample(Point2f sample_2d) const noexcept
		{
			Float pdf;
			Point2i intersection;
			Point2f sample = distribution.sample(sample_2d, &pdf, &intersection);
			return FilterSample{ sample, filter_function_values[intersection] / pdf };
		}

	private:
		AABB2f domain;
		Array2D<Float> filter_function_values;
		PiecewiseConstant2D distribution;
	};

	class BoxFilter
	{
	public:
		BoxFilter(Vec2f radius = Vec2f(0.5, 0.5))
			: radius(radius)
		{}

		static BoxFilter* create(const ParameterDictionary& parameters,
			const FileLoc* loc, Allocator allocatorator);

		LOQUAT_CPU_GPU
		Vec2f get_radius() const
		{
			return radius;
		}

		std::string to_string() const;

		LOQUAT_CPU_GPU
		Float evaluate(Point2f p) const
		{
			return (std::abs(p.x) <= radius.x && std::abs(p.y) <= radius.y) ? 1 : 0;
		}

		LOQUAT_CPU_GPU
		FilterSample sample(Point2f u) const
		{
			Point2f p(lerp(u[0], -radius.x, radius.x), 
				lerp(u[1], -radius.y, radius.y));
			return { p, Float(1) };
		}

		LOQUAT_CPU_GPU
		Float integral() const
		{
			return 2 * radius.x * 2 * radius.y;
		}

	private:
		Vec2f radius;
	};

	class GaussianFilter
	{
	public:
		GaussianFilter(Vec2f radius, Float sigma = 0.5f,
			Allocator allocator = {})
			: radius{ radius }
			, sigma{ sigma }
			, exp_x{ gaussian(radius.x, 0, sigma) }
			, exp_y{ gaussian(radius.y, 0, sigma) }
			, sampler{ this, allocator }
		{}

		static GaussianFilter* create(const ParameterDictionary& parameters,
			const FileLoc* loc, Allocator allocator);

		LOQUAT_CPU_GPU
		Vec2f get_radius() const
		{
			return radius;
		}

		std::string to_string() const;

		LOQUAT_CPU_GPU
		Float evaluate(Point2f p) const
		{
			return (std::max<Float>(0, gaussian(p.x, 0, sigma) - exp_x) *
				std::max<Float>(0, gaussian(p.y, 0, sigma) - exp_y));
		}

		LOQUAT_CPU_GPU
		Float integral() const
		{
			return (
				(gaussian_integral(-radius.x, radius.x, 0, sigma) - 2 * radius.x * exp_x) *
				(gaussian_integral(-radius.y, radius.y, 0, sigma) - 2 * radius.y * exp_y)
				);
		}

		LOQUAT_CPU_GPU
		FilterSample sample(Point2f u) const
		{
			return sampler.sample(u);
		}

	private:
		Vec2f radius;
		Float sigma;
		Float exp_x;
		Float exp_y;
		FilterSampler sampler;
	};

	class MitchellFilter
	{
	public:
		MitchellFilter(Vec2f radius, Float b = 1.f / 3.f, Float c = 1.f / 3.f,
			Allocator allocator = {})
			: radius{ radius }
			, b{ b }
			, c{ c }
			, sampler{ this, allocator }
		{}

		static MitchellFilter* create(const ParameterDictionary& parameters,
			const FileLoc* loc, Allocator allocator);

		LOQUAT_CPU_GPU
		Vec2f get_radius() const
		{
			return radius;
		}

		std::string to_string() const;

		LOQUAT_CPU_GPU
		Float evaluate(Point2f p) const
		{
			return mitchell_1D(2 * p.x / radius.x) * mitchell_1D(2 * p.y / radius.y);
		}

		LOQUAT_CPU_GPU
		FilterSample sample(Point2f u) const
		{
			return sampler.sample(u);
		}

		LOQUAT_CPU_GPU
		Float integral() const
		{
			return radius.x * radius.y / 4;
		}

	private:
		LOQUAT_CPU_GPU
		Float mitchell_1D(Float x) const
		{
			x = std::abs(x);
			if (x <= 1)
			{
				return (
					(12 - 9 * b - 6 * c) * x * x * x 
					+ (-18 + 12 * b + 6 * c) * x * x
					+ (6 - 2 * b)
					) * (1.f / 6.f);
			}
			else if (x <= 2)
			{
				return (
					(-b - 6 * c) * x * x * x 
					+ (6 * b + 30 * c) * x * x 
					+ (-12 * b - 48 * c) * x 
					+ (8 * b + 24 * c)
					) * (1.f / 6.f);
			}
			else
			{
				return 0;
			}
		}

		Vec2f radius;
		Float b;
		Float c;
		FilterSampler sampler;
	};

	class LanczosSincFilter
	{
	public:
		LanczosSincFilter(Vec2f radius, Float tau = 3.f, Allocator allocator = {})
			: radius{ radius }
			, tau{ tau }
			, sampler{ this, allocator }
		{}

		static LanczosSincFilter* create(const ParameterDictionary& parameters,
			const FileLoc* loc, Allocator allocator);

		LOQUAT_CPU_GPU
		Vec2f get_radius() const
		{
			return radius;
		}

		std::string to_string() const;

		LOQUAT_CPU_GPU
		Float evaluate(Point2f p) const
		{
			return windowed_sinc(p.x, radius.x, tau) * windowed_sinc(p.y, radius.y, tau);
		}

		LOQUAT_CPU_GPU
		FilterSample sample(Point2f u) const
		{
			return sampler.sample(u);
		}

		LOQUAT_CPU_GPU
		Float integral() const;

	private:
		Vec2f radius;
		Float tau;
		FilterSampler sampler;
	};

	class TriangleFilter
	{
	public:
		TriangleFilter(Vec2f radius)
			: radius(radius)
		{}

		static TriangleFilter* create(const ParameterDictionary& parameters,
			const FileLoc* loc, Allocator allocator);

		LOQUAT_CPU_GPU
		Vec2f get_radius() const
		{
			return radius;
		}

		std::string to_string() const;

		LOQUAT_CPU_GPU
		Float evaluate(Point2f p) const
		{
			return std::max<Float>(0, radius.x - std::abs(p.x)) *
				std::max<Float>(0, radius.y - std::abs(p.y));
		}

		LOQUAT_CPU_GPU
		FilterSample sample(Point2f u) const
		{
			return {
				Point2f(sample_tent(u[0], radius.x), sample_tent(u[1], radius.y)),
					Float(1)
			};
		}

		LOQUAT_CPU_GPU
		Float integral() const
		{
			return square(radius.x) * square(radius.y);
		}

	private:
		Vec2f radius;
	};

	LOQUAT_CPU_GPU
	inline Float Filter::evaluate(Point2f p) const
	{
		auto eval = [&](auto ptr) { return ptr->evaluate(p); };
		return dispatch(eval);
	}

	LOQUAT_CPU_GPU
	inline FilterSample Filter::sample(Point2f u) const
	{
		auto sample = [&](auto ptr) { return ptr->sample(u); };
		return dispatch(sample);
	}

	LOQUAT_CPU_GPU
	inline Vec2f Filter::get_radius() const
	{
		auto radius = [&](auto ptr) { return ptr->get_radius(); };
		return dispatch(radius);
	}

	LOQUAT_CPU_GPU
	inline Float Filter::integral() const
	{
		auto integral = [&](auto ptr) { return ptr->integral(); };
		return dispatch(integral);
	}

}