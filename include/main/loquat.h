#pragma once

#include <concepts>
#include <cstdint>
#include <cstddef>
#include <memory>
#include <type_traits>

#include "main/memory_utils.h"

#include "debug/logger.h"

// NOTE(ches) the math includes are in a somewhat specific order

#include "glm/glm.hpp"

#include "pbr/math/float.h"
#include "pbr/math/math.h"
#include "pbr/math/vec.h"
#include "pbr/math/matrix.h"
#include "pbr/math/quaternion.h"
#include "pbr/math/interval.h"
#include "pbr/math/point.h"
#include "pbr/math/aabb.h"

#include "main/global_state.h"
#include "resource/resource_cache.h"

namespace loquat
{
	class Interaction;
	class MediumInteraction;
	class SurfaceInteraction;
	class Ray;
	class RayDifferential;
	class Transform;

	class Image;
	class ParameterDictionary;
	class TextureParameterDictionary;

	class RGB;
	class RGBColorSpace;
	class RGBSigmoidPolynomial;
	class RGBIlluminantSpectrum;
	class SampledSpectrum;
	class SampledWavelengths;
	class SpectrumWavelengths;
	class XYZ;
	enum class SpectrumType;


	template <typename T>
	struct SOA;
}
