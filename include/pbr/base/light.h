// pbrt is Copyright(c) 1998-2020 Matt Pharr, Wenzel Jakob, and Greg Humphreys.
// The pbrt source code is licensed under the Apache License, Version 2.0.
// SPDX: Apache-2.0

// This file has been modified from the original, original notice is above.

#pragma once

#include "main/loquat.h"

#include "pbr/base/medium.h"
#include "pbr/base/shape.h"
#include "pbr/base/texture.h"
#include "pbr/util/tagged_pointer.h"

namespace loquat
{
	enum class LightType
	{
		/// <summary>
		/// Light defined by a shape and directional distribution of radiance
		/// at each point on the surface.
		/// </summary>
		Area,
		/// <summary>
		/// Emit radiance along a single direction. Delta as in Dirac delta,
		/// not difference.
		/// </summary>
		DeltaDirection,
		/// <summary>
		/// Emit radiance from a single point in space. Delta as in Dirac
		/// delta, not difference.
		/// </summary>
		DeltaPosition,
		/// <summary>
		/// Light that is infinitely far away and don't have any geometry,
		/// but provide radiance to rays that escape the scene.
		/// </summary>
		Infinite
	};

	/// <summary>
	/// A light that is a single point in space, with possibly angularly
	/// varying distribution of outgoing light.
	/// </summary>
	class PointLight;
	class DistantLight;
	class ProjectionLight;
	class GoniometricLight;
	class DiffuseAreaLight;
	class UniformInfiniteLight;
	class ImageInfiniteLight;
	class PortalImageInfiniteLight;
	/// <summary>
	/// A point light but only emitting in a cone of directions from the
	/// position.
	/// </summary>
	class SpotLight;

	class LightSampleContext;
	class LightBounds;
	class CompactLightBounds;
	struct LightIncidentSample;
	struct LightEmissiveSample;

	class Light : public TaggedPointer<PointLight, DistantLight,
		ProjectionLight, GoniometricLight, SpotLight, DiffuseAreaLight,
		UniformInfiniteLight, ImageInfiniteLight, PortalImageInfiniteLight>
	{
	public:
		using TaggedPointer::TaggedPointer;

		[[nodiscard]]
		static Light create(const std::string_view name,
			const ParameterDictionary& parameters,
			const Transform& render_from_light,
			const CameraTransform& camera_transform, Medium outside_medium,
			Allocator alloc);

		[[nodiscard]]
		static Light create_area(const std::string_view name,
			const ParameterDictionary& parameters,
			const Transform& render_from_light,
			const MediumInterface& medium_interface, const Shape shape,
			FloatTexture alpha, Allocator alloc);

		/// <summary>
		/// Total emitted power.
		/// </summary>
		/// <param name="lambda">The wavelengths to calculate power of.
		/// </param>
		/// <returns>The total output of the light.</returns>
		[[nodiscard]]
		SampledSpectrum total_emitted_power(SampledWavelengths lambda)
			const noexcept;

		/// <summary>
		/// What kind of light this is.
		/// </summary>
		/// <returns>The type of light.</returns>
		[[nodiscard]]
		inline LightType type() const noexcept;

		/// <summary>
		/// Samples lights in the scen based on the directions where light is
		/// potentially visible.
		/// </summary>
		/// <param name="context">Light information about a reference point
		/// in the scene.</param>
		/// <param name="point">The sampled point.</param>
		/// <param name="lambda">The range of wavelengths we are sampling.
		/// </param>
		/// <param name="allow_incomplete_PDF">Whether we can skip generating
		/// samples for directions where the light's contribution is small.
		/// </param>
		/// <returns>Incoming radience, which may not be there if it's
		/// impossible to light to reach the reference point or 
		/// there is no valid light source associated with the point.
		/// </returns>
		[[nodiscard]]
		inline std::optional<LightIncidentSample> sample_light_incoming(
			LightSampleContext context, Point2f point,
			SampledWavelengths lambda, bool allow_incomplete_PDF = false)
			const noexcept;

		/// <summary>
		/// Calculates the PDF for sampling a point in a given direction.
		/// </summary>
		/// <param name="context">The context of a point being sampled.</param>
		/// <param name="direction">The sampling direction.</param>
		/// <param name="allow_incomplete_PDF">Whether we can skip generating
		/// samples for directions where the light's contribution is small.
		/// </param>
		/// <returns>The value of the PDF.</returns>
		[[nodiscard]]
		inline Float sample_PDF(LightSampleContext context, Vec3f direction,
			bool allow_incomplete_PDF = false) const noexcept;

		[[nodiscard]]
		std::string to_string() const noexcept;

		/// <summary>
		/// Takes in local information about an intersection point and outgoing
		/// direction, and calculates the radiance emitted back along the ray.
		/// Only used when rays intersect a light source with geometry.
		/// </summary>
		/// <param name="point">The intersection point.</param>
		/// <param name="normal">The surface normal.</param>
		/// <param name="uv">The UV coordinates of the point.</param>
		/// <param name="direction">The outgoing direction.</param>
		/// <param name="lambda">The sampled wavelengths.</param>
		/// <returns>Radiance reflected back along the ray.</returns>
		[[nodiscard]]
		inline SampledSpectrum radiance_reflected_back(Point3f point,
			Normal3f normal, Point2f uv, Vec3f direction, 
			const SampledWavelengths& lambda) const noexcept;

		/// <summary>
		/// Calculates infinite area light contributions to rays that don't hit
		/// any scene geometry. Ony used for infinite lights.
		/// </summary>
		/// <param name="ray">The ray.</param>
		/// <param name="lambda">The sampled wavelengths.</param>
		/// <returns>Radiance from the infinite light.</returns>
		[[nodiscard]]
		inline SampledSpectrum infinite_light_contribution(const Ray& ray,
			const SampledWavelengths& lambda) const noexcept;

		[[nodiscard]]
		void preprocess(const AABB3f& sceneBounds);

		std::optional<LightBounds> bounds() const noexcept;

		std::optional<LightEmissiveSample> sample_emissive(Point2f sample1,
			Point2f sample2, SampledWavelengths& lambda, Float time)
			const noexcept;

		/// <summary>
		/// Fetch the position and direction PDFs for a given ray.
		/// </summary>
		/// <param name="ray">The ray.</param>
		/// <param name="pdf_position">The position PDF for the given ray.
		/// </param>
		/// <param name="pdf_direction">The direction PDF for the given ray.
		/// </param>
		void get_PDFs(const Ray& ray, Float* pdf_position, 
			Float* pdf_direction) const noexcept;

		/// <summary>
		/// Fetch the position and direction pdfs, given an interaction and
		/// direction.
		/// </summary>
		/// <param name="interaction">The interaction.</param>
		/// <param name="direction">The direction.</param>
		///  <param name="pdf_position">The position PDF for the given ray.
		/// </param>
		/// <param name="pdf_direction">The direction PDF for the given ray.
		/// </param>
		void get_PDFs(const Interaction& interaction, Vec3f direction, 
			Float* pdf_position, Float* pdf_direction) const noexcept;
	};
}