// pbrt is Copyright(c) 1998-2020 Matt Pharr, Wenzel Jakob, and Greg Humphreys.
// The pbrt source code is licensed under the Apache License, Version 2.0.
// SPDX: Apache-2.0

// This file has been modified from the original, original notice is above.

#pragma once

#include "main/loquat.h"
#include "pbr/base/film.h"
#include "pbr/base/medium.h"
#include "pbr/util/tagged_pointer.h"

#include <string_view>

namespace loquat
{
	struct CameraRay;
	struct CameraRayDifferential;
	struct CameraWiSample;

	struct CameraSample;
	class CameraTransform;

	class PerspectiveCamera;
	class OrthographicCamera;
	class SphericalCamera;
	class RealisticCamera;

	class Camera : public TaggedPointer<PerspectiveCamera, OrthographicCamera,
		SphericalCamera, RealisticCamera>
	{
	public:
		using TaggedPointer::TaggedPointer;

		static Camera create(std::string_view name,
			const ParameterDictionary& parameters, Medium medium,
			const CameraTransform& camera_transform, Film film,
			Allocator allocactor);

		[[nodiscard]]
		std::string to_string() const;

		LOQUAT_CPU_GPU
		inline pstd::optional<CameraRay> generate_ray(CameraSample sample,
			SampledWavelengths& wavelengths) const;

		LOQUAT_CPU_GPU
		inline pstd::optional<CameraRay> generate_ray_differential(
			CameraSample sample, 
			SampledWavelengths& wavelengths) const;

		LOQUAT_CPU_GPU
		inline Film get_film() const;

		LOQUAT_CPU_GPU
		inline Float sample_time(Float sample_1D) const;

		void init_metadata(ImageMetadata* metadata) const;

		LOQUAT_CPU_GPU
		inline const CameraTransform& get_camera_transform() const;

		LOQUAT_CPU_GPU
		void approximate_dp_dxy(Point3f point, Normal3f normal, Float time,
			int samples_per_pixel, Vec3f* dpdx, Vec3f* dpdy) const;

		LOQUAT_CPU_GPU
		SampledSpectrum importance(const Ray& ray,
			SampledWavelengths& wavelengths, Point2f* raster_out = nullptr)
			const;

		LOQUAT_CPU_GPU
		void importance_PDF(const Ray& ray, Float* pdf_position,
			Float* pdf_direction) const;

		LOQUAT_CPU_GPU
		pstd::optional<CameraWiSample> sample_light_incoming(
			const Interaction& reference, Point2f sample_2D,
			SampledWavelengths& wavelengths) const;
	};
}