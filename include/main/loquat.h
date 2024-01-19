#pragma once

#include <concepts>
#include <cstdint>
#include <cstddef>
#include <memory>
#include <type_traits>

#include "memory/memory_utils.h"

#include "debug/logger.h"

// NOTE(ches) the math includes are in a somewhat specific order

#include "glm/glm.hpp"

#include "pbr/math/float.h"
#include "pbr/math/math.h"
#include "pbr/math/vec.h"
#include "pbr/math/interval.h"
#include "pbr/math/quaternion.h"
#include "pbr/math/point.h"
#include "pbr/math/aabb.h"
#include "pbr/base/medium.h"
#include "pbr/math/ray.h"
#include "pbr/math/matrix.h"
#include "pbr/math/frame.h"
#include "pbr/struct/interaction.h"
#include "pbr/math/transform.h"

#include "main/global_state.h"
#include "resource/resource_cache.h"
