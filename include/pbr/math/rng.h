// pbrt is Copyright(c) 1998-2020 Matt Pharr, Wenzel Jakob, and Greg Humphreys.
// The pbrt source code is licensed under the Apache License, Version 2.0.
// SPDX: Apache-2.0

// This file has been modified from the original, original notice is above.

#pragma once

#include <algorithm>
#include <cstdint>
#include <limits>
#include <string>
#include <type_traits>

#include "main/loquat.h"
#include "pbr/math/hash.h"

namespace loquat
{
	constexpr auto PCG32_DEFAULT_STATE = 0x853c49e6748fea9bULL;
	constexpr auto PCG32_DEFAULT_STREAM = 0xda3e39cb94b95bdbULL;
	constexpr auto PCG32_MULT = 0x5851f42d4c957f2dULL;

	class RNG
	{
	public:
		LOQUAT_CPU_GPU
		RNG() noexcept
			: state{ PCG32_DEFAULT_STATE }
			, inc{ PCG32_DEFAULT_STREAM }
		{}
		LOQUAT_CPU_GPU
		RNG(uint64_t sequence_index, uint64_t offset) noexcept
		{
			set_sequence(sequence_index, offset);
		}
		LOQUAT_CPU_GPU
		RNG(uint64_t sequence_index) noexcept
		{
			set_sequence(sequence_index);
		}

		LOQUAT_CPU_GPU
		void set_sequence(uint64_t sequence_index, uint64_t offset) noexcept;
		LOQUAT_CPU_GPU
		void set_sequence(uint64_t sequence_index) noexcept
		{
			set_sequence(sequence_index, mix_bits(sequence_index));
		}

		template <typename T>
		LOQUAT_CPU_GPU
		T uniform() noexcept;

		template <std::integral T>
		LOQUAT_CPU_GPU
		T uniform(T b) noexcept
		{
			T threshold = (~b + 1u) % b;
			while (true)
			{
				T r = uniform<T>();
				if (r >= threshold)
				{
					return r % b;
				}
			}
		}

		LOQUAT_CPU_GPU
		void advance(int64_t delta) noexcept;
		LOQUAT_CPU_GPU
		int64_t operator-(const RNG& other) const noexcept;

		[[nodiscard]]
		std::string to_string() const noexcept;

	private:
		uint64_t state;
		uint64_t inc;
	};

}
//TODO(ches) finish this