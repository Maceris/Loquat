// pbrt is Copyright(c) 1998-2020 Matt Pharr, Wenzel Jakob, and Greg Humphreys.
// The pbrt source code is licensed under the Apache License, Version 2.0.
// SPDX: Apache-2.0

// This file has been modified from the original, original notice is above.

#pragma once

#include "glm/glm.hpp"

#include "glm/ext/quaternion_common.hpp"
#include "glm/ext/quaternion_double.hpp"
#include "glm/ext/quaternion_double_precision.hpp"
#include "glm/ext/quaternion_float.hpp"
#include "glm/ext/quaternion_float_precision.hpp"
//#include "glm/ext/quaternion_exponential.hpp"
//#include "glm/ext/quaternion_geometric.hpp"
//#include "glm/ext/quaternion_relational.hpp"
#include "glm/ext/quaternion_transform.hpp"
#include "glm/ext/quaternion_trigonometric.hpp"

namespace loquat
{
#if defined(DOUBLE_PRECISION_FLOAT)
	using Quaternion = glm::dquat;
#else
	using Quaternion = glm::fquat;
#endif

	//TODO(ches) actually have a quaternion?
}