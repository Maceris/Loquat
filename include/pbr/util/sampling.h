// pbrt is Copyright(c) 1998-2020 Matt Pharr, Wenzel Jakob, and Greg Humphreys.
// The pbrt source code is licensed under the Apache License, Version 2.0.
// SPDX: Apache-2.0

// This file has been modified from the original, original notice is above.

#pragma once

#include "main/loquat.h"

namespace loquat
{
	//TODO(ches) complete this

	class PiecewiseConstant1D
	{
	public:

	private:

	};

	class PiecewiseConstant2D
	{
	public:

	private:
		AABB2f domain;
		std::vector<PiecewiseConstant1D> conditional_densities;
		PiecewiseConstant1D marginal_density;
	};
}