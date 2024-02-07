// pbrt is Copyright(c) 1998-2020 Matt Pharr, Wenzel Jakob, and Greg Humphreys.
// The pbrt source code is licensed under the Apache License, Version 2.0.
// SPDX: Apache-2.0

// This file has been modified from the original, original notice is above.

#pragma once

#include "main/loquat.h"
#include "pbr/base/filter.h"
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
		using TaggedPointer::TaggedPointer;

		inline void add_sample(Point2i point_film, SampledSpectrum spectrum,
			const SampledWavelengths& wavelengths,
			const VisibleSurface* visible_surface, Float weight) noexcept;

		inline AABB2f sample_bounds() const noexcept;

		bool uses_visible_surface() const noexcept;

		void add_splat(Point2f point, SampledSpectrum light_spectrum,
			const SampledWavelengths& wavelengths) noexcept;

		inline SampledWavelengths sample_wavelengths(Float sample_1D)
			const noexcept;

		inline Point2i full_resolution() const noexcept;
		inline AABB2i pixel_bounds() const noexcept;
		inline Float diagonal() const noexcept;

		void write_image(ImageMetadata metadata, Float splat_scale = 1)
			noexcept;

		inline RGB to_output_RGB(SampledSpectrum light_spectrum,
			const SampledWavelengths& wavelengths) const noexcept;

		Image get_image(ImageMetadata* metadata, Float splat_scale = 1)
			noexcept;

		RGB get_pixel_RGB(Point2i point, Float splat_scale = 1) const noexcept;

		inline Filter get_filter() const noexcept;
		inline const PixelSensor* get_pixel_sensor() const noexcept;

		std::string get_filename() const noexcept;

		static Film create(std::string_view name,
			const ParameterDictionary& parameters, Float exposure_time,
			const CameraTransform& camera_transform, Filter filter,
			Allocator allocator) noexcept;

		[[nodiscard]]
		std::string to_string() const noexcept;

		inline void reset_pixel(Point2i point) noexcept;
	};
}