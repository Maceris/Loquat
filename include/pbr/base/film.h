// pbrt is Copyright(c) 1998-2020 Matt Pharr, Wenzel Jakob, and Greg Humphreys.
// The pbrt source code is licensed under the Apache License, Version 2.0.
// SPDX: Apache-2.0

// This file has been modified from the original, original notice is above.

#pragma once

#include "main/loquat.h"
#include "pbr/base/filter.h"
#include "pbr/util/pstd.h"
#include "pbr/util/tagged_pointer.h"

#include <string_view>

namespace loquat
{
	class VisibleSurface;
	class RGBFilm;
	class GBufferFilm;
	class SpectralFilm;
	class PixelSensor;

	class Film : public TaggedPointer<RGBFilm, GBufferFilm, SpectralFilm>
	{
	public:
		LOQUAT_CPU_GPU
		inline void add_sample(Point2i point_film, SampledSpectrum spectrum,
			const SampledWavelengths& wavelengths,
			const VisibleSurface* visible_surface, Float weight);

		LOQUAT_CPU_GPU
		inline AABB2f sample_bounds() const;

		LOQUAT_CPU_GPU
		bool uses_visible_surface() const;

		LOQUAT_CPU_GPU
		void add_splat(Point2f point, SampledSpectrum light_spectrum,
			const SampledWavelengths& wavelengths);

		LOQUAT_CPU_GPU
		inline SampledWavelengths sample_wavelengths(Float sample_1D) const;

		LOQUAT_CPU_GPU
		inline Point2i get_full_resolution() const;
		LOQUAT_CPU_GPU
		inline AABB2i get_pixel_bounds() const;
		LOQUAT_CPU_GPU
		inline Float get_diagonal() const;

		void write_image(ImageMetadata metadata, Float splat_scale = 1);

		LOQUAT_CPU_GPU
		inline RGB to_output_RGB(SampledSpectrum light_spectrum,
			const SampledWavelengths& wavelengths) const;

		Image get_image(ImageMetadata* metadata, Float splat_scale = 1);

		LOQUAT_CPU_GPU
		RGB get_pixel_RGB(Point2i point, Float splat_scale = 1) const;

		LOQUAT_CPU_GPU
		inline Filter get_filter() const;
		LOQUAT_CPU_GPU
		inline const PixelSensor* get_pixel_sensor() const;

		std::string get_filename() const;

		using TaggedPointer::TaggedPointer;

		static Film create(std::string_view name,
			const ParameterDictionary& parameters, Float exposure_time,
			const CameraTransform& camera_transform, Filter filter,
			const FileLoc* loc, Allocator allocator);

		[[nodiscard]]
		std::string to_string() const;

		LOQUAT_CPU_GPU
		inline void reset_pixel(Point2i point);
	};
}