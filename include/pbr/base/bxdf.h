// pbrt is Copyright(c) 1998-2020 Matt Pharr, Wenzel Jakob, and Greg Humphreys.
// The pbrt source code is licensed under the Apache License, Version 2.0.
// SPDX: Apache-2.0

// This file has been modified from the original, original notice is above.

#pragma once

#include <span>

#include "main/loquat.h"
#include "pbr/base/spectrum.h"
#include "pbr/util/tagged_pointer.h"

namespace loquat
{
    enum class TransportMode
    {
        Radiance,
        Importance
    };

    /// <summary>
    /// Flags to indicate whether we are calculating reflectance and/or
    /// transmission.
    /// </summary>
    enum class BxDFReflTransFlags
    {
        Unset = 0,
        Reflection = 1 << 0,
        Transmission = 1 << 1,
        All = Reflection | Transmission
    };

    inline BxDFReflTransFlags operator|(BxDFReflTransFlags a,
        BxDFReflTransFlags b)
    {
        return BxDFReflTransFlags((int)a | (int)b);
    }

    inline int operator&(BxDFReflTransFlags a, BxDFReflTransFlags b)
    {
        return ((int)a & (int)b);
    }

    inline BxDFReflTransFlags& operator|=(BxDFReflTransFlags& a,
        BxDFReflTransFlags b)
    {
        (int&)a |= int(b);
        return a;
    }

    /// <summary>
    /// Flags for indicating different properties of materials.
    /// </summary>
    enum BxDFFlags
    {
        Unset = 0,
        Reflection = 1 << 0,
        Transmission = 1 << 1,
        Diffuse = 1 << 2,
        /// <summary>
        /// Retroreflection.
        /// </summary>
        Glossy = 1 << 3,
        Specular = 1 << 4,
        DiffuseReflection = Diffuse | Reflection,
        DiffuseTransmission = Diffuse | Transmission,
        GlossyReflection = Glossy | Reflection,
        GlossyTransmission = Glossy | Transmission,
        SpecularReflection = Specular | Reflection,
        SpecularTransmission = Specular | Transmission,
        All = Diffuse | Glossy | Specular | Reflection | Transmission
    };

    inline BxDFFlags operator|(BxDFFlags a, BxDFFlags b)
    {
        return BxDFFlags(static_cast<int>(a) | static_cast<int>(b));
    }

    inline int operator&(BxDFFlags a, BxDFFlags b)
    {
        return (static_cast<int>(a) & static_cast<int>(b));
    }

    inline int operator&(BxDFFlags a, BxDFReflTransFlags b)
    {
        return (static_cast<int>(a) & static_cast<int>(b));
    }

    inline BxDFFlags& operator|=(BxDFFlags& a, BxDFFlags b)
    {
        (int&) a |= int(b);
        return a;
    }

    inline bool is_reflective(BxDFFlags flags)
    {
        return flags & BxDFFlags::Reflection;
    }

    inline bool is_transmissive(BxDFFlags flags)
    {
        return flags & BxDFFlags::Transmission;
    }

    inline bool is_diffuse(BxDFFlags flags)
    {
        return flags & BxDFFlags::Diffuse;
    }

    inline bool is_glossy(BxDFFlags flags)
    {
        return flags & BxDFFlags::Glossy;
    }

    inline bool is_specular(BxDFFlags flags)
    {
        return flags & BxDFFlags::Specular;
    }

    inline bool is_non_specular(BxDFFlags flags)
    {
        return flags & (BxDFFlags::Diffuse | BxDFFlags::Glossy);
    }

    std::string to_string(BxDFReflTransFlags flags);

    struct BSDFSample {
        BSDFSample() = default;
        BSDFSample(SampledSpectrum spectrum, Vec3f incoming, Float pdf,
            BxDFFlags flags, Float eta = 1, bool pdf_is_proportional = false)
            : spectrum{ spectrum },
            incoming{ incoming },
            pdf{ pdf },
            flags{ flags },
            eta{ eta },
            pdf_is_proportional{ pdf_is_proportional }
        {}

        bool is_reflection() const noexcept
        {
            return loquat::is_reflective(flags);
        }

        bool is_transmissive() const noexcept
        {
            return loquat::is_transmissive(flags);
        }

        bool is_diffuse() const noexcept
        {
            return loquat::is_diffuse(flags);
        }

        bool is_glossy() const noexcept
        {
            return loquat::is_glossy(flags);
        }

        bool is_specular() const noexcept
        {
            return loquat::is_specular(flags);
        }

        std::string to_string() const;

        SampledSpectrum spectrum;
        Vec3f incoming;
        Float pdf = 0;
        BxDFFlags flags;
        Float eta = 1;
        bool pdf_is_proportional = false;
    };

    class DiffuseBxDF;
    class DiffuseTransmissionBxDF;
    class DielectricBxDF;
    class ThinDielectricBxDF;
    class HairBxDF;
    class MeasuredBxDF;
    class ConductorBxDF;
    class NormalizedFresnelBxDF;
    class CoatedDiffuseBxDF;
    class CoatedConductorBxDF;

    class BxDF
        : public TaggedPointer<DiffuseTransmissionBxDF, DiffuseBxDF, CoatedDiffuseBxDF,
        CoatedConductorBxDF, DielectricBxDF, ThinDielectricBxDF,
        HairBxDF, MeasuredBxDF, ConductorBxDF, NormalizedFresnelBxDF>
    {
    public:
        using TaggedPointer::TaggedPointer;

        [[nodiscard]]
        std::string to_string() const noexcept;

        inline BxDFFlags get_flags() const noexcept;

        [[nodiscard]]
        inline SampledSpectrum f(Vec3f outgoing, Vec3f incoming,
            TransportMode mode) const noexcept;

        [[nodiscard]]
        inline std::optional<BSDFSample> sample_f(Vec3f outgoing, 
            Float sample_1D, Point2f sample_2D,
            TransportMode mode = TransportMode::Radiance,
            BxDFReflTransFlags sample_flags = BxDFReflTransFlags::All)
            const noexcept;

        [[nodiscard]]
        inline Float PDF(Vec3f outgoing, Vec3f incoming, TransportMode mode,
            BxDFReflTransFlags sample_flags = BxDFReflTransFlags::All)
            const noexcept;

        SampledSpectrum reflectance(Vec3f outgoing,
            std::span<const Float> sample_1D,
            std::span<const Point2f> sample_2D) const noexcept;

        /// <summary>
        /// Hemispherical-hemispherical reflectance of a BRDF, calculating
        /// the amount of incident light reflected when incident light is
        /// the same from all directions.
        /// </summary>
        /// <param name="hemisphere_sample">Uniform sample values for sampling
        /// the hemisphere.</param>
        /// <param name="samples_1D">1D samples for the Monte Carlo estimator.
        /// </param>
        /// <param name="samples_2D">2D samples for the Monte Carlo estimator.
        /// </param>
        /// <returns></returns>
        SampledSpectrum reflectance(std::span<const Point2f> hemisphere_sample,
            std::span<const Float> sample_1D,
            std::span<const Point2f> sample_2D) const noexcept;

        inline void regularize() noexcept;
    };

    template<typename T>
    concept is_BxDF =
        std::same_as<T, BxDF>
        || std::same_as <T, DiffuseBxDF>
        || std::same_as <T, DiffuseTransmissionBxDF>
        || std::same_as <T, DielectricBxDF>
        || std::same_as <T, ThinDielectricBxDF>
        || std::same_as <T, HairBxDF>
        || std::same_as <T, MeasuredBxDF>
        || std::same_as <T, ConductorBxDF>
        || std::same_as <T, NormalizedFresnelBxDF>
        || std::same_as <T, CoatedDiffuseBxDF>
        || std::same_as <T, CoatedConductorBxDF>;
}