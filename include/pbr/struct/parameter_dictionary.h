// pbrt is Copyright(c) 1998-2020 Matt Pharr, Wenzel Jakob, and Greg Humphreys.
// The pbrt source code is licensed under the Apache License, Version 2.0.
// SPDX: Apache-2.0

// This file has been modified from the original, original notice is above.

#pragma once

#include <limits>
#include <map>
#include <memory>
#include <string>
#include <vector>

#include "main/loquat.h"

#include "debug/logger.h"
#include "pbr/base/texture.h"
#include "pbr/struct/containers.h"
#include "pbr/math/vector_math.h"
#include "pbr/util/error.h"
#include "pbr/util/memory.h"
#include "pbr/util/pstd.h"
#include "pbr/util/spectrum.h"

namespace loquat
{
	class ParsedParameter
	{
    public:
        ParsedParameter(FileLoc loc)
            : loc(loc)
        {}

        void add_float(Float v);
        void add_int(int i);
        void add_string(std::string_view str);
        void add_bool(bool v);

        std::string to_string() const;

        std::string type;
        std::string name;
        FileLoc loc;
        pstd::vector<Float> floats;
        pstd::vector<int> ints;
        pstd::vector<std::string> strings;
        pstd::vector<uint8_t> bools;
        mutable bool looked_up = false;
        mutable const RGBColorSpace* color_space = nullptr;
        bool may_be_unused = false;
	};

    using ParsedParameterVector = InlinedVector<ParsedParameter*, 8>;

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
        Vec2f,
        Vec3f
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
        std::map<std::string, FloatTexture> float_textures;
        std::map<std::string, SpectrumTexture> albedo_spectrum_textures;
        std::map<std::string, SpectrumTexture> illuminant_spectrum_textures;
        std::map<std::string, SpectrumTexture> unbounded_spectrum_textures;
    };

    template <ParameterType PT>
    struct ParameterTypeTraits {};

    class ParameterDictionary
    {
    public:
        ParameterDictionary() = default;
        ParameterDictionary(ParsedParameterVector params,
            const RGBColorSpace* color_space);

        ParameterDictionary(ParsedParameterVector params0,
            const ParsedParameterVector& params1,
            const RGBColorSpace* color_space);

        std::string get_texture(const std::string& name) const;

        std::vector<RGB> get_RGB_array(const std::string& name) const;

        // For --upgrade only
        pstd::optional<RGB> get_one_RGB(const std::string& name) const;
        // Unfortunately, this is most easily done here...
        Float upgrade_blackbody(const std::string& name);
        void remove_float(const std::string&);
        void remove_int(const std::string&);
        void remove_bool(const std::string&);
        void remove_point2f(const std::string&);
        void remove_vec2f(const std::string&);
        void remove_point3f(const std::string&);
        void remove_vec3f(const std::string&);
        void remove_normal3f(const std::string&);
        void remove_string(const std::string&);
        void remove_texture(const std::string&);
        void remove_spectrum(const std::string&);

        void rename_parameter(const std::string& before, const std::string& after);
        void rename_used_textures(const std::map<std::string, std::string>& m);

        const RGBColorSpace* get_color_space() const { return color_space; }

        std::string to_parameter_list(int indent = 0) const;
        std::string to_parameter_definition(const std::string&) const;
        std::string to_string() const;

        const FileLoc* loc(const std::string&) const;

        const ParsedParameterVector& get_paremeter_vector() const { return params; }

        void free_parameters();

        Float get_one_float(const std::string& name, Float def) const;
        int get_one_int(const std::string& name, int def) const;
        bool get_one_bool(const std::string& name, bool def) const;
        std::string get_one_string(const std::string& name, const std::string& def) const;

        Point2f get_one_point2f(const std::string& name, Point2f def) const;
        Vec2f get_one_vec2f(const std::string& name, Vec2f def) const;
        Point3f get_one_point3f(const std::string& name, Point3f def) const;
        Vec3f get_one_vector3f(const std::string& name, Vec3f def) const;
        Normal3f get_one_normal3f(const std::string& name, Normal3f def) const;

        Spectrum get_one_spectrum(const std::string& name, Spectrum def,
            SpectrumType spectrum_type, Allocator alloc) const;

        std::vector<Float> get_float_array(const std::string& name) const;
        std::vector<int> get_int_array(const std::string& name) const;
        std::vector<uint8_t> get_bool_array(const std::string& name) const;

        std::vector<Point2f> get_point2f_array(const std::string& name) const;
        std::vector<Vec2f> get_vec2f_array(const std::string& name) const;
        std::vector<Point3f> get_point3f_array(const std::string& name) const;
        std::vector<Vec3f> get_vector3f_array(const std::string& name) const;
        std::vector<Normal3f> get_normal3f_array(const std::string& name) const;
        std::vector<Spectrum> get_spectrum_array(const std::string& name,
            SpectrumType spectrum_type,
            Allocator alloc) const;
        std::vector<std::string> get_string_array(const std::string& name) const;

        void report_unused() const;

    private:
        friend class TextureParameterDictionary;

        template <ParameterType PT>
        typename ParameterTypeTraits<PT>::ReturnType lookup_single(
            const std::string& name,
            typename ParameterTypeTraits<PT>::ReturnType defaultValue) const;

        template <ParameterType PT>
        std::vector<typename ParameterTypeTraits<PT>::ReturnType> lookup_array(
            const std::string& name) const;

        template <typename ReturnType, typename G, typename C>
        std::vector<ReturnType> lookup_array(const std::string& name, ParameterType type,
            const char* typeName, int nPerItem, G getValues,
            C convert) const;

        std::vector<Spectrum> extract_spectrum_array(const ParsedParameter& param,
            SpectrumType spectrum_type,
            Allocator alloc) const;

        void remove(const std::string& name, const char* typeName);
        void check_parameter_types();
        static std::string to_parameter_definition(const ParsedParameter* p,
            int indentCount);

        ParsedParameterVector params;
        const RGBColorSpace* color_space = nullptr;
        int nOwnedParams;
    };

    class TextureParameterDictionary
    {
    public:
        TextureParameterDictionary(const ParameterDictionary* dict,
            const NamedTextures* textures);

        operator const ParameterDictionary& () const { return *dict; }

        Float get_one_float(const std::string& name, Float def) const;
        int get_one_int(const std::string& name, int def) const;
        bool get_one_bool(const std::string& name, bool def) const;
        Point2f get_one_point2f(const std::string& name, Point2f def) const;
        Vec2f get_one_vec2f(const std::string& name, Vec2f def) const;
        Point3f get_one_point3f(const std::string& name, Point3f def) const;
        Vec3f get_one_vector3f(const std::string& name, Vec3f def) const;
        Normal3f get_one_normal3f(const std::string& name, Normal3f def) const;
        Spectrum get_one_spectrum(const std::string& name, Spectrum def,
            SpectrumType spectrum_type, Allocator alloc) const;
        std::string get_one_string(const std::string& name, const std::string& def) const;

        std::vector<Float> get_float_array(const std::string& name) const;
        std::vector<int> get_int_array(const std::string& name) const;
        std::vector<uint8_t> get_bool_array(const std::string& name) const;
        std::vector<Point2f> get_point2f_array(const std::string& name) const;
        std::vector<Vec2f> get_vec2f_array(const std::string& name) const;
        std::vector<Point3f> get_point3f_array(const std::string& name) const;
        std::vector<Vec3f> get_vector3f_array(const std::string& name) const;
        std::vector<Normal3f> get_normal3f_array(const std::string& name) const;
        std::vector<Spectrum> get_spectrum_array(const std::string& name,
            SpectrumType spectrum_type,
            Allocator alloc) const;
        std::vector<std::string> get_string_array(const std::string& name) const;

        FloatTexture get_float_texture(const std::string& name,
            Float defaultValue, Allocator alloc) const;
        FloatTexture get_float_texture_or_null(const std::string& name,
            Allocator alloc) const;

        void report_unused() const;

        SpectrumTexture get_spectrum_texture(std::string name,
            Spectrum defaultValue, SpectrumType spectrum_type,
            Allocator alloc) const;
        SpectrumTexture get_spectrum_texture_or_null(std::string name,
            SpectrumType spectrum_type, Allocator alloc) const;

    private:
        const ParameterDictionary* dict;
        const NamedTextures* textures;
    };
}
