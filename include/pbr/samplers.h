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
#include "pbr/math/pmj02_tables.h"
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

		void start_pixel_sample(Point2i p, int sample_index, int dim)
			noexcept
		{
			halton_index = 0;
			int sample_stride = base_scales[0] * base_scales[1];
			if (sample_stride > 1)
			{
				Point2i pm{
					p.x % MAX_HALTON_RESOLUTION,
					p.y % MAX_HALTON_RESOLUTION
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
			if (randomize == RandomizeStrategy::PermuteDigits)
			{
				return scrambled_radical_inverse(dimension, halton_index,
					(*digit_permutations)[dimension]);
			}
			
			LOG_ASSERT(randomize == RandomizeStrategy::Owen);
			return owen_scrambled_radical_inverse(dimension, halton_index,
				mix_bits(1ull + (static_cast<uint64_t>(dimension) << 4)));
			
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

		void start_pixel_sample(Point2i p, int index, int dim) noexcept
		{
			pixel = p;
			sample_index = index;
			dimension = dim;
		}

		[[nodiscard]]
		Float get_1D() noexcept
		{
			const uint64_t hash = loquat::hash(pixel, dimension, seed);
			const int index = permutation_element(sample_index,
				samples_per_pixel, hash);

			const int dim = dimension++;
			return sample_dimension(0, index, hash >> 32);
		}

		[[nodiscard]]
		Point2f get_2D() noexcept
		{
			const uint64_t hash = loquat::hash(pixel, dimension, seed);
			const int index = permutation_element(sample_index,
				samples_per_pixel, hash);

			const int dim = dimension;
			dimension += 2;
			return Point2f{
				sample_dimension(0, index, static_cast<uint32_t>(hash)),
				sample_dimension(1, index, hash >> 32)
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
			if (randomize == RandomizeStrategy::PermuteDigits)
			{
				return sobol_sample(a, dimension, BinaryPermuteScrambler(hash));
			}
			if (randomize == RandomizeStrategy::FastOwen)
			{
				return sobol_sample(a, dimension, FastOwenScrambler(hash));
			}
			return sobol_sample(a, dimension, OwenScrambler(hash));
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
	public:
		PMJ02BNSampler(int samples_per_pixel, int seed = 0,
			Allocator allocator = {}) noexcept;

		static constexpr const char* get_name() noexcept
		{
			return "PMJ02BNSampler";
		}

		static PMJ02BNSampler* create(
			const ParameterDictionary& parameters, Allocator allocator)
			noexcept;

		int get_samples_per_pixel() const noexcept
		{
			return samples_per_pixel;
		}

		void start_pixel_sample(Point2i p, int index, int dim) noexcept
		{
			pixel = p;
			sample_index = index;
			dimension = std::max(2, dim);
		}

		[[nodiscard]]
		Float get_1D() noexcept
		{
			const uint64_t hash = loquat::hash(pixel, dimension, seed);
			const int index = permutation_element(sample_index,
				samples_per_pixel, hash);

			Float delta = blue_noise(dimension, pixel);
			++dimension;
			return std::min((index + delta) / samples_per_pixel,
				ONE_MINUS_EPSILON);
		}

		[[nodiscard]]
		Point2f get_2D() noexcept
		{
			int index = sample_index;
			int pmj_instance = dimension / 2;
			if (pmj_instance >= PMJ02BN_SET_COUNT)
			{
				uint64_t hash = loquat::hash(pixel, dimension, seed);
				index = permutation_element(sample_index, samples_per_pixel,
					hash);
			}

			Point2f u = get_PMJ02BN_sample(pmj_instance, index);
			u += Vec2f{
				blue_noise(dimension, pixel), 
				blue_noise(dimension + 1, pixel)
			};
			if (u.x >= 1)
			{
				u.x -= 1;
			}
			if (u.y >= 1)
			{
				u.y -= 1;
			}

			dimension += 2;
			return {
				std::min(u.x, ONE_MINUS_EPSILON),
				std::min(u.y, ONE_MINUS_EPSILON)
			};
		}

		[[nodiscard]]
		Point2f get_pixel_2D() noexcept
		{
			size_t px = pixel.x % pixel_tile_size;
			size_t py = pixel.y % pixel_tile_size;
			size_t offset = (px + py * pixel_tile_size) * samples_per_pixel;
			return (*pixel_samples)[offset + sample_index];
		}

		Sampler clone(Allocator allocator) noexcept;

		[[nodiscard]]
		std::string to_string() const noexcept;

	private:
		int samples_per_pixel;
		int seed;
		int pixel_tile_size;
		std::vector<Point2f>* pixel_samples;
		Point2i pixel;
		int sample_index;
		int dimension;
	};

	class IndependentSampler
	{
	public:
		IndependentSampler(int samples_per_pixel, int seed = 0)
			: samples_per_pixel{ samples_per_pixel }
			, seed{ seed }
		{}

		static IndependentSampler* create(
			const ParameterDictionary& parameters, Allocator allocator)
			noexcept;

		static constexpr const char* get_name() noexcept
		{
			return "IndependentSampler";
		}

		int get_samples_per_pixel() const noexcept
		{
			return samples_per_pixel;
		}

		void start_pixel_sample(Point2i p, int index, int dimension) noexcept
		{
			rng.set_sequence(hash(p, seed));
			rng.advance(index * 65536ull + dimension);
		}

		[[nodiscard]]
		Float get_1D() noexcept
		{
			return rng.uniform<Float>();
		}

		[[nodiscard]]
		Point2f get_2D() noexcept
		{
			return { rng.uniform<Float>(), rng.uniform<Float>() };
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
		int samples_per_pixel;
		int seed;
		RNG rng;
	};

	class SobolSampler
	{
	public:
		SobolSampler(int samples_per_pixel, Point2i full_resolution,
			RandomizeStrategy randomize, int seed = 0) noexcept
			: samples_per_pixel{ samples_per_pixel }
			, seed{ seed }
			, randomize{ randomize }
		{
			if (!is_power_of_2(samples_per_pixel))
			{
				LOG_WARNING(std::format(
					"Sobol sampler has a sample count of {}, which is suboptimal as it is not a power of 2."
					, samples_per_pixel));
			}
			scale = round_up_pow2(std::max(full_resolution.x,
				full_resolution.y));
		}

		static constexpr const char* get_name() noexcept
		{
			return "SobolSampler";
		}

		int get_samples_per_pixel() const noexcept
		{
			return samples_per_pixel;
		}

		void start_pixel_sample(Point2i p, int index, int dimension) noexcept
		{
			pixel = p;
			dimension = std::max(2, dimension);
			sobol_index = sobol_interval_to_index(log2_int(scale), index,
				pixel);
		}

		[[nodiscard]]
		Float get_1D() noexcept
		{
			if (dimension >= SOBOL_DIMENSIONS) {
				dimension = 2;
			}
			return sample_dimension(dimension++);
		}

		[[nodiscard]]
		Point2f get_2D() noexcept
		{
			if (dimension >= SOBOL_DIMENSIONS) {
				dimension = 2;
			}
			Point2f sample{
				sample_dimension(dimension),
				sample_dimension(dimension + 1)
			};
			dimension += 2;
			return sample;
		}

		[[nodiscard]]
		Point2f get_pixel_2D() noexcept
		{
			Point2f sample{
				sobol_sample(sobol_index, 0, NoRandomizer()),
				sobol_sample(sobol_index, 1, NoRandomizer())
			};
			for (int dim = 0; dim < 2; ++dim)
			{
				sample[dim] = clamp(sample[dim] * scale - pixel[dim], 0, 
					ONE_MINUS_EPSILON);
			}
			return sample;
		}

		Sampler clone(Allocator allocator) noexcept;

		[[nodiscard]]
		std::string to_string() const noexcept;

	private:
		Float sample_dimension(int dimension) const noexcept
		{
			if (randomize == RandomizeStrategy::None)
			{
				return sobol_sample(sobol_index, dimension, NoRandomizer());
			}

			uint32_t hash = loquat::hash(dimension, seed);
			if (randomize == RandomizeStrategy::PermuteDigits)
			{
				return sobol_sample(sobol_index, dimension, BinaryPermuteScrambler(hash));
			}
			if (randomize == RandomizeStrategy::FastOwen)
			{
				return sobol_sample(sobol_index, dimension, FastOwenScrambler(hash));
			}
			return sobol_sample(sobol_index, dimension, OwenScrambler(hash));
		}

		int samples_per_pixel;
		int scale;
		int seed;
		RandomizeStrategy randomize;
		Point2i pixel;
		int dimension;
		int64_t sobol_index;
	};

	class StratifiedSampler
	{
	public:
		StratifiedSampler(int pixel_samples_x, int pixel_samples_y,
			bool jitter, int seed = 0) noexcept
			: pixel_samples_x{ pixel_samples_x }
			, pixel_samples_y{ pixel_samples_y }
			, jitter{ jitter }
			, seed{ seed }
		{}

		static constexpr const char* get_name() noexcept
		{
			return "StratifiedSampler";
		}

		static StratifiedSampler* create(
			const ParameterDictionary& parameters, Allocator allocator)
			noexcept;

		int get_samples_per_pixel() const noexcept
		{
			return pixel_samples_x * pixel_samples_y;
		}

		void start_pixel_sample(Point2i p, int index, int dim) noexcept
		{
			pixel = p;
			sample_index = index;
			dimension = dim;
			rng.set_sequence(hash(p, seed));
			rng.advance(sample_index * 65536ull + dimension);
		}

		[[nodiscard]]
		Float get_1D() noexcept
		{
			uint64_t hash = loquat::hash(pixel, dimension, seed);
			int stratum = permutation_element(sample_index,
				get_samples_per_pixel(), hash);

			++dimension;
			Float delta = jitter ? rng.uniform<Float>() : 0.5f;
			return (stratum + delta) / get_samples_per_pixel();
		}

		[[nodiscard]]
		Point2f get_2D() noexcept
		{
			uint64_t hash = loquat::hash(pixel, dimension, seed);
			int stratum = permutation_element(sample_index,
				get_samples_per_pixel(), hash);
			dimension += 2;
			int x = stratum % pixel_samples_x;
			int y = stratum / pixel_samples_x;
			Float dx = jitter ? rng.uniform<Float>() : 0.5f;
			Float dy = jitter ? rng.uniform<Float>() : 0.5f;
			return {
				(x + dx) / pixel_samples_x,
				(y + dy) / pixel_samples_y
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
		int pixel_samples_x;
		int pixel_samples_y;
		int seed;
		bool jitter;
		RNG rng;
		Point2i pixel;
		int sample_index = 0;
		int dimension = 0;
	};

	class ZSobolSampler
	{
	public:
		ZSobolSampler(int samples_per_pixel, Point2i full_resolution,
			RandomizeStrategy randomize, int seed = 0) noexcept
			: randomize{ randomize }
			, seed{ seed }
		{
			if (!is_power_of_2(samples_per_pixel))
			{
				LOG_WARNING(std::format(
					"Sobol sampler has a sample count of {}, which is suboptimal as it is not a power of 2."
					, samples_per_pixel));
			}
			log2_samples_per_pixel = log2_int(samples_per_pixel);
			int res = round_up_pow2(std::max(full_resolution.x,
				full_resolution.y));
			int log4_samples_per_pixel = (log2_samples_per_pixel + 1) / 2;
			base_4_digit_count = log2_int(res) + log4_samples_per_pixel;
		}

		static constexpr const char* get_name() noexcept
		{
			return "ZSobolSampler";
		}

		static ZSobolSampler* create(const ParameterDictionary& parameters,
			Point2i full_resolution, Allocator allocator) noexcept;

		int samples_per_pixel() const noexcept
		{
			return 1 << log2_samples_per_pixel;
		}

		void start_pixel_sample(Point2i p, int index, int dim) noexcept
		{
			dimension = dim;
			morton_index =
				(encode_morton_2(p.x, p.y) << log2_samples_per_pixel)
				| index;
		}

		[[nodiscard]]
		Float get_1D() noexcept
		{
			uint64_t sample_index = get_sample_index();
			++dimension;
			uint32_t sample_hash = hash(dimension, seed);
			if (randomize == RandomizeStrategy::None)
			{
				return sobol_sample(sample_index, dimension, NoRandomizer());
			}
			else if (randomize == RandomizeStrategy::PermuteDigits)
			{
				return sobol_sample(sample_index, dimension,
					BinaryPermuteScrambler(sample_hash));
			}
			else if (randomize == RandomizeStrategy::FastOwen)
			{
				return sobol_sample(sample_index, dimension,
					FastOwenScrambler(sample_hash));
			}
			else
			{
				return sobol_sample(sample_index, dimension,
					OwenScrambler(sample_hash));
			}
		}

		[[nodiscard]]
		Point2f get_2D() noexcept
		{
			uint64_t sample_index = get_sample_index();
			dimension += 2;
			uint64_t bits = hash(dimension, seed);
			uint32_t sample_hash[2] = {
				static_cast<uint32_t>(bits),
				static_cast<uint32_t>(bits >> 32)
			};
			if (randomize == RandomizeStrategy::None)
			{
				return {
					sobol_sample(sample_index, 0, NoRandomizer()),
					sobol_sample(sample_index, 1, NoRandomizer()),
				};
			}
			else if (randomize == RandomizeStrategy::PermuteDigits)
			{
				return {
					sobol_sample(sample_index, 0,
						BinaryPermuteScrambler(sample_hash[0])),
					sobol_sample(sample_index, 1,
						BinaryPermuteScrambler(sample_hash[1])),
				};
			}
			else if (randomize == RandomizeStrategy::FastOwen)
			{
				return {
					sobol_sample(sample_index, 0,
						FastOwenScrambler(sample_hash[0])),
					sobol_sample(sample_index, 1,
						FastOwenScrambler(sample_hash[1])),
				};
			}
			else
			{
				return {
					sobol_sample(sample_index, 0,
						OwenScrambler(sample_hash[0])),
					sobol_sample(sample_index, 1,
						OwenScrambler(sample_hash[1])),
				};
			}
		}

		[[nodiscard]]
		Point2f get_pixel_2D() noexcept
		{
			return get_2D();
		}

		Sampler clone(Allocator allocator) noexcept;

		[[nodiscard]]
		std::string to_string() const noexcept;

		[[nodiscard]]
		uint64_t get_sample_index() const noexcept
		{
			static const uint8_t permutations[24][4] =
			{
			   {0, 1, 2, 3}, {0, 1, 3, 2}, {0, 2, 1, 3}, {0, 2, 3, 1},
			   {0, 3, 2, 1}, {0, 3, 1, 2}, {1, 0, 2, 3}, {1, 0, 3, 2},
			   {1, 2, 0, 3}, {1, 2, 3, 0}, {1, 3, 2, 0}, {1, 3, 0, 2},
			   {2, 1, 0, 3}, {2, 1, 3, 0}, {2, 0, 1, 3}, {2, 0, 3, 1},
			   {2, 3, 0, 1}, {2, 3, 1, 0}, {3, 1, 2, 0}, {3, 1, 0, 2},
			   {3, 2, 1, 0}, {3, 2, 0, 1}, {3, 0, 2, 1}, {3, 0, 1, 2}
			};

			uint64_t sample_index = 0;
			bool pow2_samples = log2_samples_per_pixel & 1;
			int last_digit = pow2_samples ? 1 : 0;
			for (int i = base_4_digit_count - 1; i >= last_digit; --i)
			{
				int digit_shift = 2 * i - (pow2_samples ? 1 : 0);
				int digit = (morton_index >> digit_shift) & 3;
				uint64_t higher_digits = morton_index >> (digit_shift + 2);
				int p = (mix_bits(
					higher_digits ^ 
					(static_cast<uint64_t>(0x55555555u) * dimension)) >> 24) 
					% 24;

				digit = permutations[p][digit];
				sample_index |= static_cast<uint64_t>(digit) << digit_shift;
			}
			if (pow2_samples)
			{
				int digit = morton_index & 1;
				sample_index |= digit ^ (mix_bits((morton_index >> 1) ^
					(static_cast<uint64_t>(0x55555555u) * dimension)) & 1);
			}
			return sample_index;
		}

	private:
		RandomizeStrategy randomize;
		int seed;
		int log2_samples_per_pixel;
		int base_4_digit_count;
		uint64_t morton_index;
		int dimension;

	};

	class MLTSampler
	{
	public:
		MLTSampler(int mutations_per_pixel, int rng_sequence_index,
			Float sigma, Float large_step_probability, int stream_count)
			: mutations_per_pixel{ mutations_per_pixel }
			, rng{ mix_bits(rng_sequence_index) ^ mix_bits(options->seed) }
			, sigma{ sigma }
			, large_step_probability{ large_step_probability }
			, stream_count{ stream_count }
		{}

		void start_iteration() noexcept;

		void reject() noexcept;

		void start_stream(int index) noexcept;

		int get_next_index() noexcept
		{
			return stream_index + stream_count * sample_index++;
		}

		int get_samples_per_pixel() const noexcept
		{
			return mutations_per_pixel;
		}

		void start_pixel_sample(Point2i p, int sample_index, int dim) noexcept
		{
			rng.set_sequence(hash(p));
			rng.advance(sample_index * 65536 + dim * 8192);
		}

		Float get_1D() noexcept;

		Point2f get_2D() noexcept;

		Point2f get_pixel_2D() noexcept;

		Sampler clone(Allocator allocator) noexcept;

		void accept() noexcept;

		[[nodiscard]]
		std::string dump_state() const noexcept;

		[[nodiscard]]
		std::string to_string() const noexcept
		{
			return std::format(
				"[ MLTSampler rng: {} sigma: {} large_step_probability: {} ]"
				"stream_count: {} X: {} current_iteration: {} "
				"large_step: {} last_large_step_iteration: {} "
				"stream_index: {} sample_index: {}"
				,
				rng, sigma, large_step_probability, stream_count, X,
				current_iteration, large_step, last_large_step_iteration,
				stream_index, sample_index);
		}

	protected:
		struct PrimarySample {
			Float value = 0;
			void backup() noexcept
			{
				value_backup = value;
				modify_backup = last_modification_iteration;
			}
			void restore() noexcept
			{
				value = value_backup;
				last_modification_iteration = modify_backup;
			}

			[[nodiscard]]
			std::string to_string() const noexcept
			{
				return std::format(
					"[ PrimarySample last_modification_iteration: {} "
					"value_backup: {} modify_backup: {} ]",
					last_modification_iteration, value_backup, modify_backup);
			}

			int64_t last_modification_iteration = 0;
			Float value_backup = 0;
			int64_t modify_backup = 0;
		};

		void ensure_ready(int index) noexcept;

		int mutations_per_pixel;
		RNG rng;
		Float sigma;
		Float large_step_probability;
		int stream_count;
		std::vector<PrimarySample> X;
		int64_t current_iteration = 0;
		bool large_step = true;
		int64_t last_large_step_iteration = 0;
		int stream_index;
		int sample_index;
	};

	class DebugMLTSampler : public MLTSampler
	{
	public:
		static DebugMLTSampler create(std::span<const std::string> state,
			int sample_stream_count) noexcept;
		
		Float get_1D() noexcept
		{
			int index = get_next_index();

			LOG_ASSERT(index <= sampler.size());
			return sampler[index];
		}

		Point2f get_2D() noexcept
		{
			return { get_1D(), get_1D() };
		}

		Point2f get_pixel_2D() noexcept
		{
			return get_2D();
		}

		[[nodiscard]]
		std::string to_string() const noexcept
		{
			return std::format(
				"[ DebugMLTSampler {} sampler: {} ",
				static_cast<const MLTSampler*>(this)->to_string(),
				sampler);
		}

	private:
		DebugMLTSampler(int sample_stream_count)
			: MLTSampler(1, 0, 0.5, 0.5, sample_stream_count)
		{}

		std::vector<Float> sampler;
	};
	//TODO(ches) fill this out
}