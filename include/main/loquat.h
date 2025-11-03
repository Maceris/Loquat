#pragma once

#include <concepts>
#include <cstdint>
#include <cstddef>
#include <memory>
#include <type_traits>

#ifdef LOQUAT_IS_WINDOWS
	#ifndef UNICODE
		#define UNICODE
	#endif
	#ifndef _UNICODE
		#define UNICODE
	#endif
#endif

#if defined(__CUDA_ARCH__)
	#define LOQUAT_IS_GPU_CODE
#endif

#if defined(LOQUAT_BUILD_GPU_RENDERER) && defined(__CUDACC__)
	#ifndef LOQUAT_NOINLINE
		#define LOQUAT_NOINLINE __attribute__((noinline))
	#endif
	#define LOQUAT_CPU_GPU __host__ __device__
	#define LOQUAT_GPU __device__
	#if defined(LOQUAT_IS_GPU_CODE)
		#define LOQUAT_CONST __device__ const
	#else
		#define LOQUAT_CONST const
	#endif
#else
	#define LOQUAT_CONST const
	#define LOQUAT_CPU_GPU
	#define LOQUAT_GPU
#endif

#ifdef LOQUAT_IS_WINDOWS
#define LOQUAT_CPU_GPU_LAMBDA(...) [ =, *this ] LOQUAT_CPU_GPU(__VA_ARGS__) mutable
#else
#define LOQUAT_CPU_GPU_LAMBDA(...) [=] LOQUAT_CPU_GPU(__VA_ARGS__)
#endif

#ifdef LOQUAT_BUILD_GPU_RENDERER
#define LOQUAT_L1_CACHE_LINE_SIZE 128
#else
#define LOQUAT_L1_CACHE_LINE_SIZE 64
#endif

#define LOQUAT_ARRAYSIZE(array) (sizeof(::loquat::detail::ArraySizeHelper(array)))

namespace pstd
{
	namespace pmr
	{
		template <typename T>
		class polymorphic_allocator;
	}
}

using Allocator = pstd::pmr::polymorphic_allocator<std::byte>;

/// <summary>
/// Safely delete a pointer to an object, if it's not null, and set it to
/// nullptr.
/// </summary>
/// <typeparam name="T">The type of the pointer to delete.</typeparam>
/// <param name="ptr">The pointer we are deleting.</param>
template<typename T>
constexpr void safe_delete(T* ptr) noexcept
{
	if (ptr)
	{
		delete ptr;
	}
	ptr = nullptr;
}

// NOTE(ches) the math includes are in a somewhat specific order

#include "glm/glm.hpp"
#include "glm/gtc/integer.hpp"
#include "glm/gtx/integer.hpp"

#include "pbr/math/float.h"
#include "pbr/math/vec.h"
#include "pbr/math/quaternion.h"
#include "pbr/math/point.h"

namespace loquat
{
	namespace detail
	{
		template <typename T, uint64_t N>
		auto ArraySizeHelper(const T(&array)[N]) -> char(&)[N];
	}

	class AnimatedTransform;
	class BilinearPatchMesh;
	class Interaction;
	class MediumInteraction;
	class Ray;
	class RayDifferential;
	class SurfaceInteraction;
	struct Transform;
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
	struct LOQUATOptions;

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

	template <template<typename U> typename PointBase, typename T>
		requires is_point<PointBase<T>>
	struct AABB;

	using AABB1f = AABB<Point1, Float>;
	using AABB1i = AABB<Point1, int>;

	using AABB2f = AABB<Point2, Float>;
	using AABB2i = AABB<Point2, int>;

	using AABB3f = AABB<Point3, Float>;
	using AABB3i = AABB<Point3, int>;
}

/// <summary>
/// Used to block off sections of code that use defined code that
/// will have linker errors.
/// </summary>
#define ENABLE_WIP_CODE 1
