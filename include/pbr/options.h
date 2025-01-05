// pbrt is Copyright(c) 1998-2020 Matt Pharr, Wenzel Jakob, and Greg Humphreys.
// The pbrt source code is licensed under the Apache License, Version 2.0.
// SPDX: Apache-2.0

// This file has been modified from the original, original notice is above.

#pragma once

#include "main/loquat.h"

#include "pbr/util/pstd.h"

namespace loquat
{
	enum class RenderingCoordinateSystem
	{
		Camera,
		CameraWorld,
		World
	};

	[[nodiscard]]
	std::string to_string(const RenderingCoordinateSystem&) noexcept;

    struct BasicPBROptions
    {
        int seed = 0;
        bool quiet = false;
        bool disable_pixel_jitter = false;
        bool disable_wavelength_jitter = false;
        bool disable_texture_filtering = false;
        bool disable_image_textures = false;
        bool force_diffuse = false;
        bool use_GPU = false;
        bool wavefront = false;
        bool interactive = false;
        RenderingCoordinateSystem renderingSpace = 
            RenderingCoordinateSystem::CameraWorld;
    };

    struct PBROptions : BasicPBROptions
    {
        int thread_count = 0;
        bool log_utilization = false;
        bool write_partial_images = false;
        bool record_pixel_statistics = false;
        bool print_statistics = false;
        pstd::optional<int> pixel_samples;
        bool quick_render = false;
        bool upgrade = false;
        std::string image_file;
        std::string mse_reference_image;
        std::string mse_reference_output;
        std::string debug_start;
        std::string display_server;
        pstd::optional<AABB2f> crop_window;
        pstd::optional<AABB2i> pixel_bounds;
        pstd::optional<Point2i> pixel_material;
        Float displacement_edge_scale = 1;

        [[nodiscard]]
        std::string to_string() const noexcept;
    };

    extern PBROptions* options;

#if defined(LOQUAT_BUILD_GPU_RENDERER)
#if defined(__CUDACC__)
    extern __constant__ BasicPBROptions options_GPU;
#endif
    void copy_options_to_gpu();
#endif

    LOQUAT_CPU_GPU inline const BasicPBROptions& get_options()
    {
#if defined(LOQUAT_IS_GPU_CODE)
        return options_GPU;
#else
        return *options;
#endif
    }

}