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

	private:
		BxDF bxdf;
		Frame shading_frame;
	};
}