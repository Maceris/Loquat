// pbrt is Copyright(c) 1998-2020 Matt Pharr, Wenzel Jakob, and Greg Humphreys.
// The pbrt source code is licensed under the Apache License, Version 2.0.
// SPDX: Apache-2.0

// This file has been modified from the original, original notice is above.

#pragma once

#include <algorithm>
#include <cmath>
#include <limits>
#include <string>

#include "main/loquat.h"

#include "pbr/struct/interaction.h"
#include "pbr/media.h"
#include "pbr/options.h"
#include "pbr/base/bxdf.h"
#include "pbr/math/math.h"
#include "pbr/math/vector_math.h"
#include "pbr/util/memory.h"
#include "pbr/util/pstd.h"
#include "pbr/util/scattering.h"
#include "pbr/util/spectrum.h"
#include "pbr/util/tagged_pointer.h"

namespace loquat
{
    class DiffuseBxDF
    {
    public:
        DiffuseBxDF() = default;
        LOQUAT_CPU_GPU
            DiffuseBxDF(SampledSpectrum R) : R(R) {}

        LOQUAT_CPU_GPU
        SampledSpectrum f(Vec3f outgoing, Vec3f incoming, TransportMode mode) const
        {
            if (!same_hemisphere(outgoing, incoming))
            {
                return SampledSpectrum(0.0f);
            }
            return R * INV_PI;
        }

        LOQUAT_CPU_GPU
        pstd::optional<BSDFSample> sample_f(
            Vec3f outgoing, Float uc, Point2f u, TransportMode mode,
            BxDFReflTransFlags sample_flags = BxDFReflTransFlags::All) const
        {
            if (!(sample_flags & BxDFReflTransFlags::Reflection))
            {
                return {};
            }
            // Sample cosine-weighted hemisphere to compute _incoming_ and _pdf_
            Vec3f incoming = sample_cosine_hemisphere(u);
            if (outgoing.z < 0)
            {
                incoming.z *= -1;
            }
            Float pdf = cosine_hemisphere_PDF(absolute_cos_theta(incoming));

            return BSDFSample(R * INV_PI, incoming, pdf,
                BxDFFlags::DiffuseReflection);
        }

        LOQUAT_CPU_GPU
        Float PDF(Vec3f outgoing, Vec3f incoming, TransportMode mode,
                BxDFReflTransFlags sample_flags = BxDFReflTransFlags::All) const
        {
            if (!(sample_flags & BxDFReflTransFlags::Reflection) 
                || !same_hemisphere(outgoing, incoming))
            {
                return 0;
            }
            return cosine_hemisphere_PDF(absolute_cos_theta(incoming));
        }

        LOQUAT_CPU_GPU
        static constexpr const char* get_name()
        {
            return "DiffuseBxDF";
        }

        [[nodiscard]]
        std::string to_string() const;

        LOQUAT_CPU_GPU
        void Regularize()
        {}

        LOQUAT_CPU_GPU
        BxDFFlags Flags() const
        {
            return R ? BxDFFlags::DiffuseReflection : BxDFFlags::Unset;
        }

    private:
        SampledSpectrum R;
    };

}
//TODO(ches) fill this out