// pbrt is Copyright(c) 1998-2020 Matt Pharr, Wenzel Jakob, and Greg Humphreys.
// The pbrt source code is licensed under the Apache License, Version 2.0.
// SPDX: Apache-2.0

// This file has been modified from the original, original notice is above.

#pragma once

#include "main/loquat.h"
#include "pbr/bxdfs.h"
#include "pbr/math/vec.h"
#include "pbr/math/vector_math.h"

namespace loquat
{
	class BSDF
	{
	public:
		BSDF() noexcept = default;

		BSDF(Normal3f ns, Vec3f dpdus, BxDF bxdf) noexcept
			: bxdf{ bxdf }
			, shading_frame{ Frame::from_XZ(glm::normalize(dpdus), ns) }
		{}

		operator bool() const noexcept
		{
			return (bool) bxdf;
		}

		BxDFFlags get_flags() const noexcept
		{
			return bxdf.get_flags();
		}

		Vec3f render_to_local(Vec3f vector) const noexcept
		{
			return shading_frame.to_local(vector);
		}

		Vec3f local_to_render(Vec3f vector) const noexcept
		{
			return shading_frame.from_local(vector);
		}

		[[nodiscard]]
		SampledSpectrum f(Vec3f outgoing_render, Vec3f incoming_render,
			TransportMode mode = TransportMode::Radiance) const noexcept
		{
			Vec3f incoming = render_to_local(incoming_render);
			Vec3f outgoing = render_to_local(outgoing_render);
			if (outgoing.z == 0)
			{
				return {};
			}
			return bxdf.f(outgoing, incoming, mode);
		}


		template <is_BxDF BxDF>
		[[nodiscard]]
		SampledSpectrum f(Vec3f outgoing_render, Vec3f incoming_render,
			TransportMode mode = TransportMode::Radiance) const noexcept
		{
			Vec3f incoming = render_to_local(incoming_render);
			Vec3f outgoing = render_to_local(outgoing_render);
			if (outgoing.z == 0)
			{
				return {};
			}
			const BxDF* specific_BxDF = bxdf.cast_or_nullptr<BxDF>();
			return specific_BxDF->f(outgoing, incoming, mode);
		}

		[[nodiscard]]
		std::optional<BSDFSample> sample_f(Vec3f outgoing_render,
			Float sample_1D, Point2f sample_2D,
			TransportMode mode = TransportMode::Radiance,
			BxDFReflTransFlags sample_flags = BxDFReflTransFlags::All)
			const noexcept
		{
			Vec3f outgoing = render_to_local(outgoing_render);
			if (outgoing.z == 0 || !(bxdf.get_flags() & sample_flags))
			{
				return {};
			}
			std::optional<BSDFSample> result = bxdf.sample_f(outgoing,
				sample_1D, sample_2D, mode, sample_flags);
			if (result)
			{
				LOG_ASSERT(result->pdf >= 0);
			}
			if (!result || !result->flags || result->pdf == 0
				|| result->incoming.z == 0)
			{
				return {};
			}
			result->incoming = local_to_render(result->incoming);
			return result;
		}

		[[nodiscard]]
		inline Float PDF(Vec3f outgoing, Vec3f incoming, TransportMode mode,
			BxDFReflTransFlags sample_flags = BxDFReflTransFlags::All)
			const noexcept
		{
			if (outgoing.z == 0)
			{
				return 0;
			}
			return bxdf.PDF(outgoing, incoming, mode, sample_flags);
		} 

		template<is_BxDF BxDF>
		[[nodiscard]]
		std::optional<BSDFSample> sample_f(Vec3f outgoing_render,
			Float sample_1D, Point2f sample_2D,
			TransportMode mode = TransportMode::Radiance,
			BxDFReflTransFlags sample_flags = BxDFReflTransFlags::All)
			const noexcept
		{
			Vec3f outgoing = render_to_local(outgoing_render);
			if (outgoing.z == 0)
			{
				return {};
			}
			const BxDF* specific_BxDF = bxdf.cast<BxDF>();
			if (!(specific_BxDF.get_flags() & sample_flags))
			{
				return {};
			}

			std::optional<BSDFSample> result = specific_BxDF->sample_f(
				outgoing, sample_1D, sample_2D, mode, sample_flags);

			if (!result || !result->flags || result->pdf == 0
				|| result->incoming.z == 0)
			{
				return {};
			}

			LOG_ASSERT(result->pdf >= 0);

			result->incoming = local_to_render(result->incoming);
			return result;
		}

		template<is_BxDF BxDF>
		[[nodiscard]]
		inline Float PDF(Vec3f outgoing, Vec3f incoming, TransportMode mode,
			BxDFReflTransFlags sample_flags = BxDFReflTransFlags::All)
			const noexcept
		{
			if (outgoing.z == 0)
			{
				return 0;
			}
			const BxDF* specific_BxDF = bxdf.cast<BxDF>();
			return specific_BxDF->PDF(outgoing, incoming, mode, sample_flags);
		}

		[[nodiscard]]
		std::string to_string() const noexcept;

		SampledSpectrum reflectance(Vec3f outgoing_render,
			std::span<const Float> sample_1D,
			std::span<const Point2f> sample_2D) const noexcept
		{
			Vec3f outgoing = render_to_local(outgoing_render);
			return bxdf.reflectance(outgoing, sample_1D, sample_2D);
		}

		SampledSpectrum reflectance(std::span<const Point2f> hemisphere_sample,
			std::span<const Float> sample_1D,
			std::span<const Point2f> sample_2D) const noexcept
		{
			return bxdf.reflectance(hemisphere_sample, sample_1D, sample_2D);
		}

		void regularize() noexcept
		{
			bxdf.regularize();
		}

	private:
		BxDF bxdf;
		Frame shading_frame;
	};
}