// pbrt is Copyright(c) 1998-2020 Matt Pharr, Wenzel Jakob, and Greg Humphreys.
// The pbrt source code is licensed under the Apache License, Version 2.0.
// SPDX: Apache-2.0

// This file has been modified from the original, original notice is above.

#pragma once

#include "main/loquat.h"

#include "pbr/math/vector_math.h"

namespace loquat
{

	static constexpr int BLUE_NOISE_RESOLUTION = 128;
	static constexpr int BLUE_NOISE_TEXTURE_COUNT = 48;

	extern const uint16_t blue_noise_textures[BLUE_NOISE_TEXTURE_COUNT]
		[BLUE_NOISE_RESOLUTION][BLUE_NOISE_RESOLUTION];

	inline float blue_noise(int texture_index, Point2i point)
	{
		LOG_ASSERT(texture_index >= 0 && point.x >= 0 && point.y >= 0);
		texture_index %= BLUE_NOISE_TEXTURE_COUNT;
		int x = point.x % BLUE_NOISE_RESOLUTION;
		int y = point.y % BLUE_NOISE_RESOLUTION;
		return blue_noise_textures[texture_index][x][y] / 65535.0f;
	}
}