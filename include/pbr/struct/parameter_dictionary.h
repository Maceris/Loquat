#pragma once

#include <map>

#include "debug/logger.h"
#include "pbr/base/texture.h"

namespace loquat
{
	class ParsedParameter
	{

	};

    enum class ParameterType
    {
        Boolean,
        Float,
        Integer,
        Normal3f,
        Point2f,
        Point3f,
        Spectrum,
        String,
        Texture,
        Vector2f,
        Vector3f
    };

    enum class SpectrumType
    { 
        Albedo,
        Illuminant, 
        Unbounded 
    };

    inline std::string to_string(SpectrumType t) {
        switch (t) {
        case SpectrumType::Albedo:
            return "Albedo";
        case SpectrumType::Illuminant:
            return "Illuminant";
        case SpectrumType::Unbounded:
            return "Unbounded";
        default:
            LOG_FATAL("Unhandled SpectrumType");
        }
    }

    struct NamedTextures
    {
        std::map<std::string, FloatTexture> floatTextures;
        std::map<std::string, SpectrumTexture> albedoSpectrumTextures;
        std::map<std::string, SpectrumTexture> illuminantSpectrumTextures;
        std::map<std::string, SpectrumTexture> unboundedSpectrumTextures;
    };

    template <ParameterType PT>
    struct ParameterTypeTraits {};

    class ParameterDictionary
    {

    };
}