// pbrt is Copyright(c) 1998-2020 Matt Pharr, Wenzel Jakob, and Greg Humphreys.
// The pbrt source code is licensed under the Apache License, Version 2.0.
// SPDX: Apache-2.0

// This file has been modified from the original, original notice is above.

#pragma once

//TODO(ches) fill this out


#include <memory>
#include <string>
#include <vector>

#include "main/loquat.h"
#include "pbr/struct/image.h"
#include "pbr/util/pstd.h"
#include "pbr/math/vector_math.h"

namespace loquat
{

    enum class FilterFunction
    {
        Point,
        Bilinear,
        Trilinear,
        EWA
    };

    inline pstd::optional<FilterFunction> parse_filter(const std::string& f)
    {
        if (f == "ewa" || f == "EWA")
        {
            return FilterFunction::EWA;
        }
        else if (f == "trilinear")
        {
            return FilterFunction::Trilinear;
        }
        else if (f == "bilinear")
        {
            return FilterFunction::Bilinear;
        }
        else if (f == "point")
        {
            return FilterFunction::Point;
        }
        else
        {
            return {};
        }
    }

    [[nodiscard]]
    std::string to_string(FilterFunction f);

    struct MIPMapFilterOptions
    {
        FilterFunction filter = FilterFunction::EWA;
        Float maxAnisotropy = 8.0f;
        bool operator<(MIPMapFilterOptions o) const
        {
            return std::tie(filter, maxAnisotropy) 
                < std::tie(o.filter, o.maxAnisotropy);
        }

        [[nodiscard]]
        std::string to_string() const;
    };

    class MIPMap
    {
    public:
        MIPMap(Image image, const RGBColorSpace* color_space, WrapMode wrap_mode,
            Allocator alloc, const MIPMapFilterOptions& options);

        static MIPMap* create_from_file(const std::string& filename,
            const MIPMapFilterOptions& options, WrapMode wrap_mode,
            ColorEncoding encoding, Allocator alloc);

        template <typename T>
        T Filter(Point2f st, Vec2f dstdx, Vec2f dstdy) const;

        [[nodiscard]]
        std::string to_string() const;

        Point2i level_resolution(int level) const
        {
            LOG_ASSERT(level >= 0 && level < pyramid.size());
            return pyramid[level].get_resoution();
        }
        int levels() const
        {
            return static_cast<int>(pyramid.size());
        }
        const RGBColorSpace* get_RGB_color_space() const
        {
            return color_space;
        }
        const Image& get_level(int level) const
        {
            return pyramid[level];
        }

    private:
        template <typename T>
        T texel(int level, Point2i st) const;
        template <typename T>
        T bilerp(int level, Point2f st) const;
        template <typename T>
        T EWA(int level, Point2f st, Vec2f dst0, Vec2f dst1) const;

        pstd::vector<Image> pyramid;
        const RGBColorSpace* color_space;
        WrapMode wrap_mode;
        MIPMapFilterOptions options;
    };

}