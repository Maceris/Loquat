// pbrt is Copyright(c) 1998-2020 Matt Pharr, Wenzel Jakob, and Greg Humphreys.
// The pbrt source code is licensed under the Apache License, Version 2.0.
// SPDX: Apache-2.0

// This file has been modified from the original, original notice is above.

#pragma once

#include <initializer_list>
#include <map>
#include <mutex>
#include <string>

#include "main/loquat.h"

#include "pbr/base/texture.h"
#include "pbr/math/math.h"
#include "pbr/math/transform.h"
#include "pbr/math/vector_math.h"
#include "pbr/struct/interaction.h"
#include "pbr/struct/parameter_dictionary.h"
#include "pbr/util/color_space.h"
#include "pbr/util/mipmap.h"
#include "pbr/util/noise.h"
#include "pbr/util/spectrum.h"
#include "pbr/util/tagged_pointer.h"

//TODO(ches) fill this out


namespace loquat
{
	class UniversalTextureEvaluator
	{
    public:
        LOQUAT_CPU_GPU
        bool CanEvaluate(std::initializer_list<FloatTexture>,
            std::initializer_list<SpectrumTexture>) const {
            return true;
        }

        LOQUAT_CPU_GPU
        Float operator()(FloatTexture tex, TextureEvalContext ctx);

        LOQUAT_CPU_GPU
        SampledSpectrum operator()(SpectrumTexture tex, TextureEvalContext ctx,
            SampledWavelengths lambda);
	};
}