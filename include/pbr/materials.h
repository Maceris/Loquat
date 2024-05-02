// pbrt is Copyright(c) 1998-2020 Matt Pharr, Wenzel Jakob, and Greg Humphreys.
// The pbrt source code is licensed under the Apache License, Version 2.0.
// SPDX: Apache-2.0

// This file has been modified from the original, original notice is above.

#pragma once

#include "main/loquat.h"
#include "pbr/interaction.h"
#include "pbr/base/material.h"

namespace loquat
{

    struct MaterialEvalContext : public TextureEvalContext
    {
        MaterialEvalContext() = default;
        MaterialEvalContext(const SurfaceInteraction& interaction)
            : TextureEvalContext{si}
            , outgoing{interaction.wo}
            , normal(interaction.shading.n)
            , dpdu(interaction.shading.dpdu)
        {}
        
        [[nodiscard]]
        std::string to_string() const noexcept;
        
        Vector3f outgoing;
        Normal3f normal;
        Vector3f dpdu;
    }

//TODO(ches) finish this
}
