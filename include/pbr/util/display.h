// pbrt is Copyright(c) 1998-2020 Matt Pharr, Wenzel Jakob, and Greg Humphreys.
// The pbrt source code is licensed under the Apache License, Version 2.0.
// SPDX: Apache-2.0

// This file has been modified from the original, original notice is above.

#pragma once

#include "main/loquat.h"

#include <functional>
#include <string>

#include "pbr/math/vector_math.h"
#include "pbr/struct/containers.h"
#include "pbr/struct/image.h"
#include "pbr/util/color.h"
#include "pbr/util/pstd.h"

namespace loquat
{
    void connect_to_display_server(const std::string& host);
    void disconnect_from_display_server();

    void display_dynamic(
        std::string title, Point2i resolution,
        std::vector<std::string> channel_names,
        std::function<void(AABB2i, pstd::span<pstd::span<float>>)> get_values);

    void display_dynamic(std::string title, const Image& image,
        pstd::optional<ImageChannelDesc> channel_description = {});

    void display_static(
        std::string title, Point2i resolution,
        std::vector<std::string> channel_names,
        std::function<void(AABB2i, pstd::span<pstd::span<float>>)> get_values);

    void display_static(std::string title, const Image& image,
        pstd::optional<ImageChannelDesc> channel_description = {});

    template <typename T>
    inline typename std::enable_if_t<std::is_arithmetic_v<T>, void>
    display_static(
        const std::string& title, pstd::span<const T> values, int x_resolution)
    {
        CHECK_EQ(0, values.size() % x_resolution);
        int yResolution = values.size() / x_resolution;
        display_static(title, { x_resolution, yResolution }, { "value" },
            [=](AABB2i b, pstd::span<pstd::span<float>> display_value) {
                LOG_ASSERT(1 == display_value.size());
                int index = 0;
                for (Point2i p : b)
                {
                    display_value[0][index++] = values[p.x + p.y * x_resolution];
                }
            });
    }

    template <typename T>
    inline typename std::enable_if_t<std::is_arithmetic_v<T>, void>
    display_dynamic(
        const std::string& title, pstd::span<const T> values, int x_resolution)
    {
        LOG_ASSERT(0 == values.size() % x_resolution);
        int yResolution = values.size() / x_resolution;
        display_dynamic(title, { x_resolution, yResolution }, { "value" },
            [=](AABB2i b, pstd::span<pstd::span<float>> display_value) {
                LOG_ASSERT(1 == display_value.size());
                int index = 0;
                for (Point2i p : b)
                {
                    display_value[0][index++] = values[p.x + p.y * x_resolution];
                }
            });
    }

    namespace detail
    {

        // https://stackoverflow.com/a/31306194
        // base case
        template <typename...>
        using void_t = void;

        template <class T, class Index, typename = void>
        struct has_subscript_operator : std::false_type {};

        template <class T, class Index>
        struct has_subscript_operator<T, Index,
            void_t<decltype(std::declval<T>()[std::declval<Index>()])>>
            : std::true_type {};

        template <class T, class Index>
        using has_subscript_operator_t
            = typename has_subscript_operator<T, Index>::type;
    }

    template <typename T>
    inline typename std::enable_if_t<detail::has_subscript_operator_t<T, int>::value, void>
    display_static(const std::string& title, pstd::span<const T> values,
            const std::vector<std::string>& channel_names, int x_resolution)
    {
        LOG_ASSERT(0 == values.size() % x_resolution);
        int yResolution = values.size() / x_resolution;
        display_static(title, { x_resolution, yResolution }, channel_names,
            [=](AABB2i b, pstd::span<pstd::span<float>> display_value) {
                LOG_ASSERT(channel_names.size() == display_value.size());
                int index = 0;
                for (Point2i p : b)
                {
                    int offset = p.x + p.y * x_resolution;
                    for (int i = 0; i < channel_names.size(); ++i)
                    {
                        display_value[i][index] = values[offset][i];
                    }
                    ++index;
                }
            });
    }

    template <typename T>
    inline typename std::enable_if_t<detail::has_subscript_operator_t<T, int>::value, void>
    display_dynamic(const std::string& title, pstd::span<const T> values,
            const std::vector<std::string>& channel_names, int x_resolution)
    {
        LOG_ASSERT(0 == values.size() % x_resolution);
        int yResolution = values.size() / x_resolution;
        display_dynamic(title, { x_resolution, yResolution }, channel_names,
            [=](AABB2i b, pstd::span<pstd::span<float>> display_value)
            {
                LOG_ASSERT(channel_names.size() == display_value.size());
                int index = 0;
                for (Point2i p : b)
                {
                    int offset = p.x + p.y * x_resolution;
                    for (int i = 0; i < channel_names.size(); ++i)
                    {
                        display_value[i][index] = values[offset][i];
                    }
                    ++index;
                }
            });
    }

    // TODO(ches) Array2D equivalents
}
