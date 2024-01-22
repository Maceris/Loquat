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
		operator bool() const
		{
			return (bool) bxdf;
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
	private:
		BxDF bxdf;
		Frame shading_frame;
	};
}