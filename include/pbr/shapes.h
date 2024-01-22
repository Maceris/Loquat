// pbrt is Copyright(c) 1998-2020 Matt Pharr, Wenzel Jakob, and Greg Humphreys.
// The pbrt source code is licensed under the Apache License, Version 2.0.
// SPDX: Apache-2.0

// This file has been modified from the original, original notice is above.

#pragma once


#include "main/loquat.h"
#include "pbr/base/shape.h"
#include "pbr/struct/interaction.h"

namespace loquat
{

	struct ShapeIntersection
	{
		SurfaceInteraction interaction;
		Float t_hit;
		std::string to_string() const noexcept;
	};

}