// pbrt is Copyright(c) 1998-2020 Matt Pharr, Wenzel Jakob, and Greg Humphreys.
// The pbrt source code is licensed under the Apache License, Version 2.0.
// SPDX: Apache-2.0

// This file has been modified from the original, original notice is above.

#pragma once

#include <string>

#include "main/loquat.h"

namespace loquat
{
	struct CameraSample
	{
		Point2f pFilm;
		Point2f pLens;
		Float time = 0;
		Float filterWeight = 1;
		std::string to_string() const noexcept;
	};

	class HaltonSampler;
	class PaddedSobolSampler;
	/// <summary>
	/// Progressive Multi-Jittered Sample Sequence sampler, from
	/// https://graphics.pixar.com/library/ProgressiveMultiJitteredSampling/paper.pdf
	/// </summary>
	class PMJ02BNSampler;
	class IndependentSampler;
	class SobolSampler;
	class StratifiedSampler;
	class ZSobolSampler;
	class MLTSampler;
	class DebugMLTSampler;


	class Sampler
	{

	};
}