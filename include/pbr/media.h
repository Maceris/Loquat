// pbrt is Copyright(c) 1998-2020 Matt Pharr, Wenzel Jakob, and Greg Humphreys.
// The pbrt source code is licensed under the Apache License, Version 2.0.
// SPDX: Apache-2.0

// This file has been modified from the original, original notice is above.

#pragma once

//TODO(ches) fill this out
#include <algorithm>
#include <limits>
#include <memory>
#include <vector>

#include "main/loquat.h"

#include "pbr/textures.h"
#include "pbr/base/medium.h"
#include "pbr/math/transform.h"
#include "pbr/struct/interaction.h"
#include "pbr/struct/parameter_dictionary.h"
#include "pbr/util/color_space.h"
#include "pbr/util/memory.h"
#include "pbr/util/parallel.h"
#include "pbr/util/pstd.h"
#include "pbr/util/scattering.h"
#include "pbr/util/spectrum.h"

#include "nanovdb/NanoVDB.h"
#include "nanovdb/util/GridHandle.h"
#include "nanovdb/util/SampleFromVoxels.h"
#if defined(LOQUAT_BUILD_GPU_RENDERER) && defined(__NVCC__)
#include "nanovdb/util/CudaDeviceBuffer.h"
#endif

