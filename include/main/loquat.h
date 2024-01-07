#pragma once

#include <concepts>
#include <cstdint>
#include <cstddef>
#include <memory>

#include "debug/logger.h"

// NOTE(ches) the math includes are in a somewhat specific order

#include "glm/glm.hpp"


#include "pbr/math/float.h"
#include "pbr/math/vec.h"
#include "pbr/math/interval.h"
#include "pbr/math/point.h"
#include "pbr/math/aabb.h"
#include "pbr/base/medium.h"
#include "pbr/math/ray.h"

#include "main/global_state.h"
#include "resource/resource_cache.h"
