// pbrt is Copyright(c) 1998-2020 Matt Pharr, Wenzel Jakob, and Greg Humphreys.
// The pbrt source code is licensed under the Apache License, Version 2.0.
// SPDX: Apache-2.0

// This file has been modified from the original, original notice is above.

#pragma once

#include <cstdint>
#include <cstring>
#include <format>
#include <map>
#include <memory>
#include <vector>

#include "pbr/util/color.h"

#include "main/loquat.h"

#include "pbr/math/float.h"
#include "pbr/math/math.h"
#include "pbr/math/vector_math.h"
#include "pbr/struct/containers.h"
#include "pbr/util/color.h"
#include "pbr/util/parallel.h"
#include "pbr/util/pstd.h"

//TODO(ches) fill this out

namespace loquat
{
    enum class PixelFormat
    {
        U256,
        Half,
        Float
    };

    LOQUAT_CPU_GPU
    inline bool is_8bit(PixelFormat format)
    {
        return format == PixelFormat::U256;
    }
    LOQUAT_CPU_GPU
    inline bool is_16bit(PixelFormat format)
    {
        return format == PixelFormat::Half;
    }
    LOQUAT_CPU_GPU
    inline bool is_32bit(PixelFormat format)
    {
        return format == PixelFormat::Float;
    }

    [[nodiscard]]
    std::string to_string(PixelFormat format);

    LOQUAT_CPU_GPU
    int texel_bytes(PixelFormat format);

    struct ResampleWeight
    {
        int first_pixel;
        Float weight[4];
    };

    enum class WrapMode
    {
        Black,
        Clamp,
        Repeat,
        OctahedralSphere
    };

    struct WrapMode2D
    {
        LOQUAT_CPU_GPU
        WrapMode2D(WrapMode w)
            : wrap{ w, w }
        {}
        LOQUAT_CPU_GPU
        WrapMode2D(WrapMode x, WrapMode y)
            : wrap{ x, y }
        {}

        pstd::array<WrapMode, 2> wrap;
    };

    inline pstd::optional<WrapMode> parse_wrap_mode(const char* w)
    {
        if (!strcmp(w, "clamp"))
        {
            return WrapMode::Clamp;
        }
        else if (!strcmp(w, "repeat"))
        {
            return WrapMode::Repeat;
        }
        else if (!strcmp(w, "black"))
        {
            return WrapMode::Black;
        }
        else if (!strcmp(w, "octahedralsphere"))
        {
            return WrapMode::OctahedralSphere;
        }
        else
        {
            return {};
        }
    }

    inline std::string to_string(WrapMode mode)
    {
        switch (mode) {
        case WrapMode::Clamp:
            return "clamp";
        case WrapMode::Repeat:
            return "repeat";
        case WrapMode::Black:
            return "black";
        case WrapMode::OctahedralSphere:
            return "octahedralsphere";
        default:
            LOG_FATAL("Unhandled wrap mode");
            return nullptr;
        }
    }

    LOQUAT_CPU_GPU
    inline bool remap_pixel_coordinates(Point2i* pp, Point2i resolution,
        WrapMode2D wrap_mode);

    LOQUAT_CPU_GPU
        inline bool remap_pixel_coordinates(Point2i* pp, Point2i resolution,
        WrapMode2D wrap_mode)
    {
        Point2i& p = *pp;

        if (wrap_mode.wrap[0] == WrapMode::OctahedralSphere)
        {
            LOG_ASSERT(wrap_mode.wrap[1] == WrapMode::OctahedralSphere);
            if (p[0] < 0)
            {
                p[0] = -p[0];                        // mirror across u = 0
                p[1] = resolution[1] - 1 - p[1];     // mirror across v = 0.5
            }
            else if (p[0] >= resolution[0])
            {
                p[0] = 2 * resolution[0] - 1 - p[0]; // mirror across u = 1
                p[1] = resolution[1] - 1 - p[1];     // mirror across v = 0.5
            }

            if (p[1] < 0)
            {
                p[0] = resolution[0] - 1 - p[0];     // mirror across u = 0.5
                p[1] = -p[1];                        // mirror across v = 0;
            }
            else if (p[1] >= resolution[1])
            {
                p[0] = resolution[0] - 1 - p[0];     // mirror across u = 0.5
                p[1] = 2 * resolution[1] - 1 - p[1]; // mirror across v = 1
            }

            // Bleh: things don't go as expected for 1x1 images.
            if (resolution[0] == 1)
            {
                p[0] = 0;
            }
            if (resolution[1] == 1)
            {
                p[1] = 0;
            }

            return true;
        }

        for (int c = 0; c < 2; ++c)
        {
            if (p[c] >= 0 && p[c] < resolution[c])
            {
                // in bounds
                continue;
            }

            switch (wrap_mode.wrap[c])
            {
            case WrapMode::Repeat:
                p[c] = glm::mod(p[c], resolution[c]);
                break;
            case WrapMode::Clamp:
                p[c] = clamp(p[c], 0, resolution[c] - 1);
                break;
            case WrapMode::Black:
                return false;
            default:
                LOG_FATAL("Unhandled WrapMode mode");
            }
        }
        return true;
    }

    struct ImageMetadata
    {
        const RGBColorSpace* get_color_space() const;
        std::string to_string() const;

        pstd::optional<float> render_time_seconds;
        pstd::optional<SquareMatrix<4>> camera_from_world;
        pstd::optional<SquareMatrix<4>> NDC_from_world;
        pstd::optional<AABB2i> pixel_bounds;
        pstd::optional<Point2i> full_resolution;
        pstd::optional<int> samples_per_pixel;
        pstd::optional<float> MSE;
        pstd::optional<const RGBColorSpace*> color_space;
        std::map<std::string, std::string> strings;
        std::map<std::string, std::vector<std::string>> string_vectors;
    };

    struct ImageAndMetadata;

    struct ImageChannelDesc
    {
        operator bool() const
        {
            return size() > 0;
        }

        size_t size() const
        {
            return offset.size();
        }

        bool is_identity() const
        {
            for (size_t i = 0; i < offset.size(); ++i)
            {
                if (offset[i] != i)
                {
                    return false;
                }
            }
            return true;
        }

        [[nodiscard]]
        std::string to_string() const;

        InlinedVector<int, 4> offset;
    };

    struct ImageChannelValues : public InlinedVector<Float, 4>
    {
        ImageChannelValues() = default;
        explicit ImageChannelValues(size_t sz, Float v = {})
            : InlinedVector<Float, 4>(sz, v)
        {}

        operator Float() const
        {
            LOG_ASSERT(1 == size());
            return (*this)[0];
        }
        operator pstd::array<Float, 3>() const
        {
            LOG_ASSERT(3 == size());
            return { (*this)[0], (*this)[1], (*this)[2] };
        }

        Float max_value() const
        {
            Float m = (*this)[0];
            for (int i = 1; i < size(); ++i)
            {
                m = std::max(m, (*this)[i]);
            }
            return m;
        }
        Float average() const
        {
            Float sum = 0;
            for (int i = 0; i < size(); ++i)
            {
                sum += (*this)[i];
            }
            return sum / size();
        }

        [[nodiscard]]
        std::string to_string() const;
    };

    class Image
    {
    public:
        Image(Allocator allocator = {})
            : p8{ allocator }
            , p16{ allocator }
            , p32{ allocator }
            , format{ PixelFormat::U256 }
            , resolution{ 0, 0 }
        {}

        Image(pstd::vector<uint8_t> p8, Point2i resolution,
            pstd::span<const std::string> channels, ColorEncoding encoding);
        Image(pstd::vector<Half> p16, Point2i resolution,
            pstd::span<const std::string> channels);
        Image(pstd::vector<float> p32, Point2i resolution,
            pstd::span<const std::string> channels);

        Image(PixelFormat format, Point2i resolution,
            pstd::span<const std::string> channel_names,
            ColorEncoding encoding = nullptr, Allocator allocator = {});

        LOQUAT_CPU_GPU
        PixelFormat get_format() const
        {
            return format;
        }

        LOQUAT_CPU_GPU
        Point2i get_resoution() const
        {
            return resolution;
        }

        LOQUAT_CPU_GPU
        int get_channel_count() const
        {
            return channel_names.size();
        }

        std::vector<std::string> get_channel_names() const;
        const ColorEncoding get_encoding() const
        {
            return encoding;
        }

        LOQUAT_CPU_GPU
        operator bool() const
        {
            return resolution.x > 0 && resolution.y > 0;
        }

        LOQUAT_CPU_GPU
        size_t pixel_offset(Point2i p) const
        {
            LOG_ASSERT(inside_exclusive(p, AABB2i({ 0, 0 }, resolution)));
            return get_channel_count() * (p.y * resolution.x + p.x);
        }

        LOQUAT_CPU_GPU
        Float get_channel(Point2i p, int c,
            WrapMode2D wrap_mode = WrapMode::Clamp) const
        {
            // Remap provided pixel coordinates before reading channel
            if (!remap_pixel_coordinates(&p, resolution, wrap_mode))
            {
                return 0;
            }

            switch (format)
            {
            case PixelFormat::U256:
            {  // Return _U256_-encoded pixel channel value
                Float r;
                encoding.to_linear({ &p8[pixel_offset(p) + c], 1 }, { &r, 1 });
                return r;
            }
            case PixelFormat::Half:
            {  // Return _Half_-encoded pixel channel value
                return Float(p16[pixel_offset(p) + c]);
            }
            case PixelFormat::Float:
            {  // Return _Float_-encoded pixel channel value
                return p32[pixel_offset(p) + c];
            }
            default:
                LOG_FATAL("Unhandled PixelFormat");
                return 0;
            }
        }

        LOQUAT_CPU_GPU
        Float bilerp_channel(Point2f p, int c,
            WrapMode2D wrap_mode = WrapMode::Clamp) const
        {
            // Compute discrete pixel coordinates and offsets for _p_
            Float x = p[0] * resolution.x - 0.5f;
            Float y = p[1] * resolution.y - 0.5f;
            int xi = pstd::floor(x);
            int yi = pstd::floor(y);
            Float dx = x - xi;
            Float dy = y - yi;

            // Load pixel channel values and return bilinearly interpolated value
            pstd::array<Float, 4> v = {
                get_channel({xi, yi}, c, wrap_mode),
                get_channel({xi + 1, yi}, c, wrap_mode),
                get_channel({xi, yi + 1}, c, wrap_mode),
                get_channel({xi + 1, yi + 1}, c, wrap_mode)
            };
            return ((1 - dx) * (1 - dy) * v[0] 
                + dx * (1 - dy) * v[1] 
                + (1 - dx) * dy * v[2] 
                + dx * dy * v[3]);
        }

        LOQUAT_CPU_GPU
        void set_channel(Point2i p, int c, Float value);

        ImageChannelValues get_channels(Point2i p,
            WrapMode2D wrap_mode = WrapMode::Clamp) const;

        ImageChannelDesc get_channel_desc(
            pstd::span<const std::string> channels) const;

        ImageChannelDesc all_channels_desc() const
        {
            ImageChannelDesc desc;
            desc.offset.resize(get_channel_count());
            for (int i = 0; i < get_channel_count(); ++i)
            {
                desc.offset[i] = i;
            }
            return desc;
        }

        ImageChannelValues get_channels(Point2i p, const ImageChannelDesc& desc,
            WrapMode2D wrap_mode = WrapMode::Clamp) const;

        Image select_channels(const ImageChannelDesc& desc,
            Allocator allocator = {}) const;
        Image crop(const AABB2i& bounds, Allocator allocator = {}) const;

        void copy_rect_out(const AABB2i& extent, pstd::span<float> buf,
            WrapMode2D wrap_mode = WrapMode::Clamp) const;
        void copy_rect_in(const AABB2i& extent, pstd::span<const float> buf);

        ImageChannelValues average(const ImageChannelDesc& desc) const;

        bool has_any_infinite_pixels() const;
        bool has_any_NaN_pixels() const;

        ImageChannelValues MAE(const ImageChannelDesc& desc, const Image& ref,
            Image* errorImage = nullptr) const;
        ImageChannelValues MSE(const ImageChannelDesc& desc, const Image& ref,
            Image* mseImage = nullptr) const;
        ImageChannelValues MRSE(const ImageChannelDesc& desc, const Image& ref,
            Image* mrseImage = nullptr) const;

        Image gaussian_filter(const ImageChannelDesc& desc, int half_width,
            Float sigma) const;

        template <typename F>
        Array2D<Float> get_sampling_distribution(
            F dxdA, const AABB2f& domain = AABB2f(Point2f(0, 0), Point2f(1, 1)),
            Allocator allocator = {});
        Array2D<Float> get_sampling_distribution()
        {
            return get_sampling_distribution([](Point2f) { return Float(1); });
        }

        static ImageAndMetadata read(std::string filename, Allocator allocator = {},
            ColorEncoding encoding = nullptr);

        bool write(std::string name, const ImageMetadata& metadata = {}) const;

        Image convert_to_format(PixelFormat format,
            ColorEncoding encoding = nullptr) const;

        // TODO(ches)? provide an iterator to iterate over all pixels and channels?

        LOQUAT_CPU_GPU
        Float lookup_nearest_channel(Point2f p, int c,
                WrapMode2D wrap_mode = WrapMode::Clamp) const
        {
            Point2i pi(p.x * resolution.x, p.y * resolution.y);
            return get_channel(pi, c, wrap_mode);
        }

        ImageChannelValues lookup_nearest(Point2f p,
            WrapMode2D wrap_mode = WrapMode::Clamp) const;
        ImageChannelValues lookup_nearest(Point2f p, const ImageChannelDesc& desc,
            WrapMode2D wrap_mode = WrapMode::Clamp) const;

        ImageChannelValues bilerp(Point2f p, WrapMode2D wrap_mode = WrapMode::Clamp) const;
        ImageChannelValues bilerp(Point2f p, const ImageChannelDesc& desc,
            WrapMode2D wrap_mode = WrapMode::Clamp) const;

        void set_channels(Point2i p, const ImageChannelValues& values);
        void set_channels(Point2i p, pstd::span<const Float> values);
        void set_channels(Point2i p, const ImageChannelDesc& desc,
            pstd::span<const Float> values);

        Image float_resize_up(Point2i new_resolutionolution, WrapMode2D wrap) const;
        void flip_y();
        static pstd::vector<Image> generate_pyramid(Image image,
            WrapMode2D wrap_mode,  Allocator allocator = {});

        std::vector<std::string> get_channel_names(const ImageChannelDesc&) const;

        LOQUAT_CPU_GPU
        size_t bytes_used() const
        {
            return p8.size() + 2 * p16.size() + 4 * p32.size();
        }

        LOQUAT_CPU_GPU
        const void* raw_pointer(Point2i p) const
        {
            if (is_8bit(format))
            {
                return p8.data() + pixel_offset(p);
            }
            if (is_16bit(format))
            {
                return p16.data() + pixel_offset(p);
            }
            else
            {
                LOG_ASSERT(is_32bit(format));
                return p32.data() + pixel_offset(p);
            }
        }
        LOQUAT_CPU_GPU
        void* raw_pointer(Point2i p)
        {
            return const_cast<void*>(((const Image*)this)->raw_pointer(p));
        }

        Image joint_bilateral_filter(const ImageChannelDesc& to_filter,
            int half_width, const Float xy_sigma[2],
            const ImageChannelDesc& joint,
            const ImageChannelValues& joint_sigma) const;

        [[nodiscard]]
        std::string to_string() const;

    private:
        static std::vector<ResampleWeight> resample_weights(int old_resolution,
            int new_resolution);
        bool write_EXR(const std::string& name, const ImageMetadata& metadata) const;
        bool write_PFM(const std::string& name, const ImageMetadata& metadata) const;
        bool write_PNG(const std::string& name, const ImageMetadata& metadata) const;
        bool write_QOI(const std::string& name, const ImageMetadata& metadata) const;

        std::unique_ptr<uint8_t[]> quantize_pixels_to_U256(int* nOutOfGamut) const;

        PixelFormat format;
        Point2i resolution;
        pstd::vector<std::string> channel_names;
        ColorEncoding encoding = nullptr;
        pstd::vector<uint8_t> p8;
        pstd::vector<Half> p16;
        pstd::vector<float> p32;
    };

    inline void Image::set_channel(Point2i p, int c, Float value) {
        if (is_NaN(value))
        {
#ifndef LOQUAT_IS_GPU_CODE
            LOG_ERROR(std::format("NaN at pixel {},{} comp {}", p.x, p.y, c));
#endif
            value = 0;
        }

        switch (format)
        {
        case PixelFormat::U256:
            encoding.from_linear({ &value, 1 }, { &p8[pixel_offset(p) + c], 1 });
            break;
        case PixelFormat::Half:
            p16[pixel_offset(p) + c] = Half(value);
            break;
        case PixelFormat::Float:
            p32[pixel_offset(p) + c] = value;
            break;
        default:
            LOG_FATAL("Unhandled PixelFormat in Image::set_channel()");
        }
    }

    template <typename F>
    inline Array2D<Float> Image::get_sampling_distribution(F dxdA,
        const AABB2f& domain, Allocator allocator)
    {
        Array2D<Float> dist(resolution[0], resolution[1], allocator);
        parallel_for(0, resolution[1], [&](int64_t y0, int64_t y1)
            {
            for (int y = y0; y < y1; ++y) {
                for (int x = 0; x < resolution[0]; ++x) {
                    // This is noticeably better than max_value: discuss / show
                    // example..
                    Float value = get_channels({ x, y }).average();

                    // Assume Jacobian term is basically constant over the
                    // region.
                    Point2f p = domain.lerp(
                        Point2f((x + .5f) / resolution[0], (y + .5f) / resolution[1]));
                    dist(x, y) = value * dxdA(p);
                }
            }
            });
        return dist;
    }

    struct ImageAndMetadata
    {
        Image image;
        ImageMetadata metadata;
    };
}