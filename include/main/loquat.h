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
	class AnimatedTransform;
	class BilinearPatchMesh;
	class Interaction;
	class MediumInteraction;
	class Ray;
	class RayDifferential;
	class SurfaceInteraction;
	class Transform;
	class TriangleMesh;

	class RGB;
	class RGBColorSpace;
	class RGBSigmoidPolynomial;
	class RGBIlluminantSpectrum;
	class SampledSpectrum;
	class SampledWavelengths;
	class SpectrumWavelengths;
	class XYZ;
	enum class SpectrumType;

	class BSDF;
	class CameraTransform;
	class Image;
	class ParameterDictionary;
	struct NamedTextures;
	class TextureParameterDictionary;
	struct ImageMetadata;
	class MediumInterface;
	struct PBRTOptions;

	class PiecewiseConstant1D;
	class PiecewiseConstant2D;
	class ProgressReporter;
	class RNG;
	struct FileLoc;
	class Interval;
	template <typename T>
	class Array2D;

	template <typename T>
	struct SOA;
	class ScratchBuffer;
}

/// <summary>
/// Used to block off sections of code that use defined code that
/// will have linker errors.
/// </summary>
#define ENABLE_WIP_CODE 0
