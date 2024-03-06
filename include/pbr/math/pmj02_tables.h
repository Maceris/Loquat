// pbrt is Copyright(c) 1998-2020 Matt Pharr, Wenzel Jakob, and Greg Humphreys.
// The pbrt source code is licensed under the Apache License, Version 2.0.
// SPDX: Apache-2.0

// This file has been modified from the original, original notice is above.

#pragma once

#include "main/loquat.h"

#include "pbr/math/vector_math.h"

namespace loquat
{
	constexpr int PMJ02BN_SET_COUNT = 5;
	constexpr int PMJ02BN_SAMPLES = 65536;
	extern const uint32_t pmj02bn_samples
		[PMJ02BN_SET_COUNT][PMJ02BN_SAMPLES][2];

	inline Point2f get_PMJ02BN_sample(int set_index, int sample_index) noexcept
	{
		set_index %= PMJ02BN_SET_COUNT;
		LOG_ASSERT(sample_index < PMJ02BN_SAMPLES);
		sample_index %= PMJ02BN_SAMPLES;
		return Point2f {
			pmj02bn_samples[set_index][sample_index][0] * 0x1p-32,
			pmj02bn_samples[set_index][sample_index][1] * 0x1p-32
		};
	}
}