// pbrt is Copyright(c) 1998-2020 Matt Pharr, Wenzel Jakob, and Greg Humphreys.
// The pbrt source code is licensed under the Apache License, Version 2.0.
// SPDX: Apache-2.0

// This file has been modified from the original, original notice is above.

#pragma once

#include "glm/glm.hpp"

namespace loquat
{
#if defined(DOUBLE_PRECISION_FLOAT)
	using Quaternion = glm::dquat;
#else
	using Quaternion = glm::fquat;
#endif

	//TODO(ches) actually have a quaternion?
}