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