// pbrt is Copyright(c) 1998-2020 Matt Pharr, Wenzel Jakob, and Greg Humphreys.
// The pbrt source code is licensed under the Apache License, Version 2.0.
// SPDX: Apache-2.0

// This file has been modified from the original, original notice is above.

#pragma once

#include "main/loquat.h"

namespace loquat
{
	LOQUAT_CPU_GPU
	Float noise(Float x, Float y = .5f, Float z = .5f);
	LOQUAT_CPU_GPU
	Float noise(Point3f p);
	LOQUAT_CPU_GPU
	Vec3f d_noise(Point3f p);
	LOQUAT_CPU_GPU
	Float fbm(Point3f p, Vec3f dpdx, Vec3f dpdy, Float omega, int octaves);
	LOQUAT_CPU_GPU
	Float turbulence(Point3f p, Vec3f dpdx, Vec3f dpdy, Float omega, int octaves);
}