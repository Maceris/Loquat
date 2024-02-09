// pbrt is Copyright(c) 1998-2020 Matt Pharr, Wenzel Jakob, and Greg Humphreys.
// The pbrt source code is licensed under the Apache License, Version 2.0.
// SPDX: Apache-2.0

// This file has been modified from the original, original notice is above.

#pragma once

#include "main/loquat.h"
#include "pbr/util/tagged_pointer.h"

#include <string_view>

namespace loquat
{
	struct TextureEvalContext;

	class FloatConstantTexture;
	class FloatBilerpTexture;
	class FloatCheckerboardTexture;
	class FloatDotsTexture;
	/// <summary>
	/// Fractional Brownian Motion noise.
	/// </summary>
	class FBMTexture;
	class GPUFloatImageTexture;
	class FloatImageTexture;
	class FloatMixTexture;
	class FloatDirectionMixTexture;
	class FloatPtexTexture;
	class GPUFloatPtexTexture;
	class FloatScaledTexture;
	class WindyTexture;
	class WrinkledTexture;

	class FloatTexture : public TaggedPointer<FloatImageTexture,
		GPUFloatImageTexture, FloatMixTexture, FloatDirectionMixTexture,
		FloatScaledTexture, FloatConstantTexture, FloatBilerpTexture,
		FloatCheckerboardTexture, FloatDotsTexture, FBMTexture,
		FloatPtexTexture, GPUFloatPtexTexture, WindyTexture, WrinkledTexture>
	{
	public:
		using TaggedPointer::TaggedPointer;

		static FloatTexture create(std::string_view name,
			const Transform& render_from_texture,
			const TextureParameterDictionary& parameters,
			Allocator allocator, bool gpu) noexcept;

		[[nodiscard]]
		std::string to_string() const noexcept;

		inline Float evaluate(TextureEvalContext context) const noexcept;
	};
	
	class RGBConstantTexture;
	class RGBReflectanceConstantTexture;
	class SpectrumConstantTexture;
	class SpectrumBilerpTexture;
	class SpectrumCheckerboardTexture;
	class SpectrumImageTexture;
	class GPUSpectrumImageTexture;
	class MarbleTexture;
	class SpectrumMixTexture;
	class SpectrumDirectionMixTexture;
	class SpectrumDotsTexture;
	class SpectrumPtexTexture;
	class GPUSpectrumPtexTexture;
	class SpectrumScaledTexture;

	class SpectrumTexture : public TaggedPointer<SpectrumImageTexture,
		GPUSpectrumImageTexture, SpectrumMixTexture,
		SpectrumDirectionMixTexture, SpectrumScaledTexture,
		SpectrumConstantTexture, SpectrumBilerpTexture,
		SpectrumCheckerboardTexture, MarbleTexture, SpectrumDotsTexture,
		SpectrumPtexTexture, GPUSpectrumPtexTexture>
	{
	public:
		using TaggedPointer::TaggedPointer;

		static SpectrumTexture create(std::string_view name,
			const Transform& render_from_texture,
			const TextureParameterDictionary& parameters,
			SpectrumType spectrum_type, Allocator allocator, bool gpu)
			noexcept;

		[[nodiscard]]
		std::string to_string() const noexcept;

		inline Float evaluate(TextureEvalContext context) const noexcept;
	};
}