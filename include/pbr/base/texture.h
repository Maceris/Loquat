// pbrt is Copyright(c) 1998-2020 Matt Pharr, Wenzel Jakob, and Greg Humphreys.
// The pbrt source code is licensed under the Apache License, Version 2.0.
// SPDX: Apache-2.0

// This file has been modified from the original, original notice is above.

#pragma once

namespace loquat
{
	struct TextureEvalContext;

	/// <summary>
	/// Fractional Brownian Motion noise.
	/// </summary>
	class FBMTexture;
	class FloatBilerpTexture;
	class FloatCheckerboardTexture;
	class FloatConstantTexture;
	class FloatDirectionMixTexture;
	class FloatDotsTexture;
	class FloatImageTexture;
	class FloatMixTexture;
	class FloatPtexTexture;
	class FloatScaledTexture;
	class GPUFloatImageTexture;
	class GPUFloatPtexTexture;
	class GPUSpectrumImageTexture;
	class GPUSpectrumPtexTexture;
	class MarbleTexture;
	class RGBConstantTexture;
	class RGBReflectanceConstantTexture;
	class SpectrumBilerpTexture;
	class SpectrumCheckerboardTexture;
	class SpectrumConstantTexture;
	class SpectrumDirectionMixTexture;
	class SpectrumDotsTexture;
	class SpectrumImageTexture;
	class SpectrumMixTexture;
	class SpectrumPtexTexture;
	class SpectrumScaledTexture;
	class WindyTexture;
	class WrinkledTexture;

	class FloatTexture;
	class SpectrumTexture;
}