// pbrt is Copyright(c) 1998-2020 Matt Pharr, Wenzel Jakob, and Greg Humphreys.
// The pbrt source code is licensed under the Apache License, Version 2.0.
// SPDX: Apache-2.0

// This file has been modified from the original, original notice is above.

#pragma once

#include <vector>

#include "main/loquat.h"

#include "pbr/filters.h"
#include "pbr/options.h"
#include "pbr/base/sampler.h"
#include "pbr/math/blue_noise.h"
#include "pbr/math/hash.h"
#include "pbr/math/low_discrepancy.h"
#include "pbr/math/math.h"
#include "pbr/math/rng.h"
#include "pbr/math/vector_math.h"

namespace loquat
{

	class HaltonSampler
	{
	public:
		HaltonSampler(int samples_per_pixel, Point2i full_resolution,
			RandomizeStrategy randomize = RandomizeStrategy::PermuteDigits,
			int seed = 0, Allocator allocator = {}) noexcept;

		static constexpr const char* get_name() noexcept
		{
			return "HaltonSampler";
		}

		static HaltonSampler* create(const ParameterDictionary& parameters,
			Point2i full_resolution, Allocator allocator) noexcept;

		int get_samples_per_pixel() const noexcept
		{
			return samples_per_pixel;
		}

		RandomizeStrategy get_randomize_strategy() const noexcept
		{
			return randomize;
		}

		void start_pixel_sample(Point2i point, int sample_index, int dim)
			noexcept
		{
			halton_index = 0;
			int sample_stride = base_scales[0] * base_scales[1];
			if (sample_stride > 1)
			{
				Point2i pm{
					point.x % MAX_HALTON_RESOLUTION,
					point.y % MAX_HALTON_RESOLUTION
				};
				for (int i = 0; i < 2; ++i)
				{
					uint64_t dim_offset = i == 0
						? inverse_radical_inverse(pm[i], 2, base_exponents[i])
						: inverse_radical_inverse(pm[i], 3, base_exponents[i]);
					halton_index += dim_offset 
						* (sample_stride / base_scales[i]) * mult_inverse[i];
				}
				halton_index %= sample_stride;
			}
			halton_index += sample_index * sample_stride;
			dimension = std::max(2, dim);
		}

		[[nodiscard]]
		Float get_1D() noexcept
		{
			if (dimension >= PRIME_TABLE_SIZE)
			{
				dimension = 2;
			}
			return sample_dimension(dimension++);
		}

		[[nodiscard]]
		Point2f get_2D() noexcept
		{
			if (dimension +1 >= PRIME_TABLE_SIZE)
			{
				dimension = 2;
			}
			int dim = dimension;
			dimension += 2;
			return { sample_dimension(dim), sample_dimension(dim + 1) };
		}

		[[nodiscard]]
		Point2f get_pixel_2D() noexcept
		{
			return {
				radical_inverse(0, halton_index >> base_exponents[0]),
				radical_inverse(1, halton_index / base_scales[1])
			};
		}

		Sampler clone(Allocator allocator) noexcept;

		[[nodiscard]]
		std::string to_string() const noexcept;

	private:

		static uint64_t multiplicative_inverse(int64_t a, int64_t n) noexcept
		{
			int64_t x;
			int64_t y;
			extended_GCD(a, n, &x, &y);
			return x % n;
		}

		static void extended_GCD(uint64_t a, uint64_t b, int64_t* x,
			int64_t* y) noexcept
		{
			if (b == 0)
			{
				*x = 1;
				*y = 0;
				return;
			}
			int64_t d = a / b;
			int64_t xp;
			int64_t yp;
			extended_GCD(b, a % b, &xp, &yp);
			*x = yp;
			*y = xp - (d * yp);
		}

		Float sample_dimension(int dimension) const noexcept
		{
			if (randomize == RandomizeStrategy::None)
			{
				return radical_inverse(dimension, halton_index);
			}
			else if (randomize == RandomizeStrategy::PermuteDigits)
			{
				return scrambled_radical_inverse(dimension, halton_index,
					(*digit_permutations)[dimension]);
			}
			else
			{
				LOG_ASSERT(randomize == RandomizeStrategy::Owen);
				return owen_scrambled_radical_inverse(dimension, halton_index,
					mix_bits(1ull + (static_cast<uint64_t>(dimension) << 4)));
			}
		}

		int samples_per_pixel;
		RandomizeStrategy randomize;
		std::vector<DigitPermutation>* digit_permutations = nullptr;
		static constexpr int MAX_HALTON_RESOLUTION = 128;
		Point2i base_scales;
		Point2i base_exponents;
		int mult_inverse[2];
		int64_t halton_index = 0;
		int dimension = 0;
	};

	class PaddedSobolSampler
	{
	public:
		static constexpr const char* get_name() noexcept
		{
			return "PaddedSobolSampler";
		}

		static PaddedSobolSampler* create(
			const ParameterDictionary& parameters, Allocator allocator)
			noexcept;

		PaddedSobolSampler(int samples_per_pixel,
			RandomizeStrategy randomizer, int seed = 0)
			: samples_per_pixel{ samples_per_pixel }
			, randomize{ randomizer }
			, seed{ seed }
		{
			if (!is_power_of_2(samples_per_pixel))
			{
				LOG_WARNING(std::format(
					"Sobol sampler has a sample count of {}, which is suboptimal as it is not a power of 2."
					, samples_per_pixel));
			}
		}

		int get_samples_per_pixel() const noexcept
		{
			return samples_per_pixel;
		}

		RandomizeStrategy get_randomize_strategy() const noexcept
		{
			return randomize;
		}

		void start_pixel_sample(Point2i point, int index, int dim) noexcept
		{
			pixel = point;
			sample_index = index;
			dimension = dim;
		}

		[[nodiscard]]
		Float get_1D() noexcept
		{
			const uint64_t hash_value = hash(pixel, dimension, seed);
			const int index = permutation_element(sample_index,
				samples_per_pixel, hash_value);

			const int dim = dimension++;
			return sample_dimension(0, index, hash_value >> 32);
		}

		[[nodiscard]]
		Point2f get_2D() noexcept
		{
			const uint64_t hash_value = hash(pixel, dimension, seed);
			const int index = permutation_element(sample_index,
				samples_per_pixel, hash_value);

			const int dim = dimension;
			dimension += 2;
			return Point2f{
				sample_dimension(0, index, static_cast<uint32_t>(hash_value)),
				sample_dimension(1, index, hash_value >> 32)
			};
		}

		[[nodiscard]]
		Point2f get_pixel_2D() noexcept
		{
			return get_2D();
		}

		Sampler clone(Allocator allocator) noexcept;

		[[nodiscard]]
		std::string to_string() const noexcept;

	private:
		Float sample_dimension(int dimension, uint32_t a, uint32_t hash)
			const noexcept
		{
			if (randomize == RandomizeStrategy::None)
			{
				return sobol_sample(a, dimension, NoRandomizer());
			}
			else if (randomize == RandomizeStrategy::PermuteDigits)
			{
				return sobol_sample(a, dimension, BinaryPermuteScrambler(hash));
			}
			else if (randomize == RandomizeStrategy::FastOwen)
			{
				return sobol_sample(a, dimension, FastOwenScrambler(hash));
			}
			else
			{
				return sobol_sample(a, dimension, OwenScrambler(hash));
			}
		}

		int samples_per_pixel;
		int seed;
		RandomizeStrategy randomize;
		Point2i pixel;
		int sample_index;
		int dimension;
	};

	class PMJ02BNSampler
	{

	};

	class IndependentSampler
	{

	};

	class SobolSampler
	{

	};

	class StratifiedSampler
	{

	};

	class ZSobolSampler
	{

	};

	class MLTSampler
	{

	};

	class DebugMLTSampler
	{

	};

}