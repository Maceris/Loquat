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
#include "pbr/math/float.h"
#include "pbr/math/hash.h"
#include "pbr/math/math.h"
#include "pbr/util/pstd.h"

namespace loquat
{
	#define PCG32_DEFAULT_STATE 0x853c49e6748fea9bULL
	#define PCG32_DEFAULT_STREAM 0xda3e39cb94b95bdbULL
	#define PCG32_MULT 0x5851f42d4c957f2dULL

	class RNG
	{
	public:
		LOQUAT_CPU_GPU
		RNG()
			: state{ PCG32_DEFAULT_STATE }
			, inc{ PCG32_DEFAULT_STREAM }
		{}
		LOQUAT_CPU_GPU
		RNG(uint64_t sequence_index, uint64_t offset)
		{
			set_sequence(sequence_index, offset);
		}
		LOQUAT_CPU_GPU
		RNG(uint64_t sequence_index)
		{
			set_sequence(sequence_index);
		}

		LOQUAT_CPU_GPU
		void set_sequence(uint64_t sequence_index, uint64_t offset);
		LOQUAT_CPU_GPU
		void set_sequence(uint64_t sequence_index)
		{
			set_sequence(sequence_index, mix_bits(sequence_index));
		}

		template <typename T>
		LOQUAT_CPU_GPU
		T uniform();

		template <typename T>
		LOQUAT_CPU_GPU
		typename std::enable_if_t<std::is_integral_v<T>, T> uniform(T b)
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
		void advance(int64_t delta);
		LOQUAT_CPU_GPU
		int64_t operator-(const RNG& other) const;

		[[nodiscard]]
		std::string to_string() const;

	private:
		uint64_t state;
		uint64_t inc;
	};

	template <typename T>
	LOQUAT_CPU_GPU
	inline T RNG::uniform()
	{
		return T::unimplemented;
	}

	template <>
	LOQUAT_CPU_GPU
	inline uint32_t RNG::uniform<uint32_t>();

	template <>
	LOQUAT_CPU_GPU
	inline uint32_t RNG::uniform<uint32_t>()
	{
		uint64_t oldstate = state;
		state = oldstate * PCG32_MULT + inc;
		uint32_t xorshifted = (uint32_t)(((oldstate >> 18u) ^ oldstate) >> 27u);
		uint32_t rot = (uint32_t)(oldstate >> 59u);
		return (xorshifted >> rot) | (xorshifted << ((~rot + 1u) & 31));
	}

	template <>
	LOQUAT_CPU_GPU
	inline uint64_t RNG::uniform<uint64_t>()
	{
		uint64_t v0 = uniform<uint32_t>(), v1 = uniform<uint32_t>();
		return (v0 << 32) | v1;
	}

	template <>
	LOQUAT_CPU_GPU
	inline int32_t RNG::uniform<int32_t>()
	{
		// https://stackoverflow.com/a/13208789
		uint32_t v = uniform<uint32_t>();
		if (v <= (uint32_t)std::numeric_limits<int32_t>::max())
		{
			return int32_t(v);
		}
		LOG_ASSERT(v >= (uint32_t)std::numeric_limits<int32_t>::min());
		return int32_t(v - std::numeric_limits<int32_t>::min()) +
			std::numeric_limits<int32_t>::min();
	}

	template <>
	LOQUAT_CPU_GPU
	inline int64_t RNG::uniform<int64_t>()
	{
		// https://stackoverflow.com/a/13208789
		uint64_t v = uniform<uint64_t>();
		if (v <= (uint64_t)std::numeric_limits<int64_t>::max())
		{
			// Safe to type convert directly.
			return int64_t(v);
		}
		LOG_ASSERT(v >= (uint64_t)std::numeric_limits<int64_t>::min());
		return int64_t(v - std::numeric_limits<int64_t>::min()) +
			std::numeric_limits<int64_t>::min();
	}

	LOQUAT_CPU_GPU
	inline void RNG::set_sequence(uint64_t sequenceIndex, uint64_t seed)
	{
		state = 0u;
		inc = (sequenceIndex << 1u) | 1u;
		uniform<uint32_t>();
		state += seed;
		uniform<uint32_t>();
	}

	template <>
	LOQUAT_CPU_GPU
	inline float RNG::uniform<float>()
	{
		return std::min<float>(ONE_MINUS_EPSILON, uniform<uint32_t>() * 0x1p-32f);
	}

	template <>
	LOQUAT_CPU_GPU
	inline double RNG::uniform<double>()
	{
		return std::min<double>(ONE_MINUS_EPSILON, uniform<uint64_t>() * 0x1p-64);
	}

	LOQUAT_CPU_GPU
	inline void RNG::advance(int64_t idelta)
	{
		uint64_t curMult = PCG32_MULT, curPlus = inc, accMult = 1u;
		uint64_t accPlus = 0u, delta = (uint64_t)idelta;
		while (delta > 0) {
			if (delta & 1) {
				accMult *= curMult;
				accPlus = accPlus * curMult + curPlus;
			}
			curPlus = (curMult + 1) * curPlus;
			curMult *= curMult;
			delta /= 2;
		}
		state = accMult * state + accPlus;
	}

	LOQUAT_CPU_GPU
	inline int64_t RNG::operator-(const RNG& other) const
	{
		LOG_ASSERT(inc == other.inc);
		uint64_t curMult = PCG32_MULT, curPlus = inc, curState = other.state;
		uint64_t theBit = 1u, distance = 0u;
		while (state != curState)
		{
			if ((state & theBit) != (curState & theBit))
			{
				curState = curState * curMult + curPlus;
				distance |= theBit;
			}
			LOG_ASSERT(state & theBit == curState & theBit);
			theBit <<= 1;
			curPlus = (curMult + 1ULL) * curPlus;
			curMult *= curMult;
		}
		return (int64_t)distance;
	}

}
