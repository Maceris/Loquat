// pbrt is Copyright(c) 1998-2020 Matt Pharr, Wenzel Jakob, and Greg Humphreys.
// The pbrt source code is licensed under the Apache License, Version 2.0.
// SPDX: Apache-2.0

// This file has been modified from the original, original notice is above.

#pragma once

#include "main/loquat.h"
#include "pbr/base/bssrdf.h"
#include "pbr/base/texture.h"
#include "pbr/util/tagged_pointer.h"

#include <map>
#include <string>

namespace loquat
{
    struct MaterialEvalContext;

    class CoatedDiffuseMaterial;
    class CoatedConductorMaterial;
    class ConductorMaterial;
    class DielectricMaterial;
    class DiffuseMaterial;
    class DiffuseTransmissionMaterial;
    class HairMaterial;
    class MeasuredMaterial;
    class SubsurfaceMaterial;
    class ThinDielectricMaterial;
    class MixMaterial;

    // Material Definition
    class Material : public TaggedPointer<CoatedDiffuseMaterial,
        CoatedConductorMaterial, ConductorMaterial, DielectricMaterial,
        DiffuseMaterial, DiffuseTransmissionMaterial, HairMaterial,
        MeasuredMaterial, SubsurfaceMaterial, ThinDielectricMaterial,
        MixMaterial>
    {
    public:
        using TaggedPointer::TaggedPointer;

        static Material create(const std::string& name,
            const TextureParameterDictionary& parameters, Image* normal_map,
            std::map<std::string, Material>& named_materials,
            Allocator allocator);

        [[nodiscard]]
        std::string to_string() const noexcept;

        template <typename TextureEvaluator>
        inline BSDF GetBSDF(TextureEvaluator texture_evaluator,
            MaterialEvalContext context, SampledWavelengths& lambda,
            ScratchBuffer& buffer) const noexcept;

        template <typename TextureEvaluator>
        [[nodiscard]]
        inline BSSRDF get_BSSRDF(TextureEvaluator texture_evaluator,
            MaterialEvalContext context, SampledWavelengths& lambda,
            ScratchBuffer& buffer) const noexcept;

        template <typename TextureEvaluator>
        [[nodiscard]]
        inline bool can_evaluate_textures(TextureEvaluator texture_evaluator)
            const noexcept;

        [[nodiscard]]
        inline const Image* get_normal_map() const noexcept;

        [[nodiscard]]
        inline FloatTexture get_displacement() const noexcept;

        [[nodiscard]]
        inline bool has_subsurface_scattering() const noexcept;
    };

}