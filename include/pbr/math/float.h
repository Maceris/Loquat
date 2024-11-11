// pbrt is Copyright(c) 1998-2020 Matt Pharr, Wenzel Jakob, and Greg Humphreys.
// The pbrt source code is licensed under the Apache License, Version 2.0.
// SPDX: Apache-2.0

// This file has been modified from the original, original notice is above.

#pragma once

#include <bit>
#include <cmath>
#include <concepts>
#include <limits>

#if defined(LOQUAT_BUILD_GPU_RENDERER) && defined(LOQUAT_IS_GPU_CODE)
#include <cuda_fp16.h>
#endif

namespace loquat
{
#if defined(DOUBLE_PRECISION_FLOAT)
    /// <summary>
    /// Floating point number.
    /// </summary>
    using Float = double;
#else
    /// <summary>
    /// Floating point number.
    /// </summary>
    using Float = float;
#endif

#if defined(DOUBLE_PRECISION_FLOAT)
    /// <summary>
    /// An unsigned integer used to access bits for a floating point number.
    /// </summary>
    using FloatBits = uint64_t;
#else
    /// <summary>
    /// An unsigned integer used to access bits for a floating point number.
    /// </summary>
    using FloatBits = uint32_t;
#endif

#if defined(DOUBLE_PRECISION_FLOAT)
    /// <summary>
    /// The floating point type to use for GLM.
    /// </summary>
    using FloatGLM = glm::f64;
#else
    /// <summary>
    /// The floating point type to use for GLM.
    /// </summary>
    using FloatGLM = glm::f32;
#endif

    static_assert(sizeof(Float) == sizeof(FloatBits), "Floats are an unexpected size on this device");

#if LOQUAT_IS_GPU_CODE

    #define ONE_MINUS_EPSILON_DOUBLE 0x1.fffffffffffffp-1
    #define ONE_MINUS_EPSILON_FLOAT float(0x1.fffffep-1)

    #ifdef DOUBLE_PRECISION_FLOAT
        #define ONE_MINUS_EPSILON ONE_MINUS_EPSILON_DOUBLE
    #else
        #define ONE_MINUS_EPSILON ONE_MINUS_EPSILON_FLOAT
    #endif

    #define FLOAT_INFINITY std::numeric_limits<Float>::infinity()
    #define MACHINE_EPSILON std::numeric_limits<Float>::epsilon() * 0.5f

#else
    /// <summary>
    /// As close to infinity as we can represent.
    /// </summary>
    static constexpr Float FLOAT_INFINITY = std::numeric_limits<Float>::infinity();

    /// <summary>
    /// A very small number.
    /// </summary>
    static constexpr Float MACHINE_EPSILON = std::numeric_limits<Float>::epsilon() * 0.5f;

    static constexpr double ONE_MINUS_EPSILON_DOUBLE = 0x1.fffffffffffffp-1;
    static constexpr float ONE_MINUS_EPSILON_FLOAT = 0x1.fffffep-1;

    #if defined(DOUBLE_PRECISION_FLOAT)
    static constexpr Float ONE_MINUS_EPSILON = ONE_MINUS_EPSILON_DOUBLE;
    #else
    static constexpr Float ONE_MINUS_EPSILON = ONE_MINUS_EPSILON_FLOAT;
    #endif

#endif // LOQUAT_IS_GPU_CODE
    static constexpr Float NaN = std::numeric_limits<Float>::has_signaling_NaN
        ? std::numeric_limits<Float>::signaling_NaN()
        : std::numeric_limits<Float>::quiet_NaN();

    template <std::floating_point T>
    LOQUAT_CPU_GPU
    inline T is_NaN(T v)
    {
#ifdef LOQUAT_IS_GPU_CODE
        return isnan(v);
#else
       return std::isnan(v);
#endif
    }

    template <std::integral T>
    LOQUAT_CPU_GPU
    inline T is_NaN(T v)
    {
        return false;
    }

    template <std::floating_point T>
    LOQUAT_CPU_GPU
    inline T is_inf(T v)
    {
#ifdef LOQUAT_IS_GPU_CODE
        return isinf(v);
#else
        return std::isinf(v);
#endif
    }

    template <std::integral T>
    LOQUAT_CPU_GPU
    inline T is_inf(T v)
    {
        return false;
    }

    template <std::floating_point T>
    LOQUAT_CPU_GPU
    inline T is_finite(T v)
    {
#ifdef LOQUAT_IS_GPU_CODE
        return isfinite(v);
#else
        return std::isfinite(v);
#endif
    }

    template <std::integral T>
    LOQUAT_CPU_GPU
    inline T is_finite(T v)
    {
        return true;
    }

    template <std::floating_point T>
    LOQUAT_CPU_GPU
    inline T FMA(T a, T b, T c)
    {
        return std::fma(a, b, c);
    }

    LOQUAT_CPU_GPU
    inline float bits_to_float(uint32_t i)
    {
#ifdef LOQUAT_IS_GPU_CODE
        return __uint_as_float(i);
#else
        return std::bit_cast<float>(i);
#endif
    }

    LOQUAT_CPU_GPU
    inline double bits_to_float(uint64_t i)
    {
#ifdef LOQUAT_IS_GPU_CODE
        return __longlong_as_double(i);
#else
        return std::bit_cast<double>(i);
#endif
    }

    LOQUAT_CPU_GPU
    inline uint32_t float_to_bits(float f)
    {
#ifdef LOQUAT_IS_GPU_CODE
        return __float_as_uint(f);
#else
        return std::bit_cast<uint32_t>(f);
#endif
    }

    LOQUAT_CPU_GPU
    inline uint64_t float_to_bits(double f)
    {
#ifdef LOQUAT_IS_GPU_CODE
        return __double_as_longlong(f);
#else
        return std::bit_cast<uint64_t>(f);
#endif
    }

    LOQUAT_CPU_GPU
    inline int exponent(float f)
    {
        return (float_to_bits(f) >> 23) - 127;
    }

    LOQUAT_CPU_GPU
    inline int exponent(double d)
    {
        return (float_to_bits(d) >> 52) - 1023;
    }

    LOQUAT_CPU_GPU
    inline int significand(float f)
    {
        return float_to_bits(f) & ((1 << 23) - 1);
    }

    LOQUAT_CPU_GPU
    inline uint64_t significand(double d)
    {
        return float_to_bits(d) & ((1ull << 52) - 1);
    }

    LOQUAT_CPU_GPU
    inline uint32_t sign_bit(float f)
    {
        return float_to_bits(f) & 0x80000000;
    }

    LOQUAT_CPU_GPU
    inline uint64_t sign_bit(double d)
    {
        return float_to_bits(d) & 0x8000000000000000;
    }

    LOQUAT_CPU_GPU
    inline float next_float_up(float v)
    {
        if (is_inf(v) && v > 0.0f)
        {
            return v;
        }

        if (v == -0.0f)
        {
            v = 0.0f;
        }

        uint32_t bit_value = float_to_bits(v);
        if (v >= 0)
        {
            ++bit_value;
        }
        else
        {
            --bit_value;
        }
        
        return bits_to_float(bit_value);
    }

    LOQUAT_CPU_GPU
    inline double next_float_up(double v)
    {
        if (is_inf(v) && v > 0.0f)
        {
            return v;
        }

        if (v == -0.0f)
        {
            v = 0.0f;
        }

        uint64_t bit_value = float_to_bits(v);
        if (v >= 0)
        {
            ++bit_value;
        }
        else
        {
            --bit_value;
        }

        return bits_to_float(bit_value);
    }

    LOQUAT_CPU_GPU
    inline float next_float_down(float v)
    {
        if (is_inf(v) && v < 0.0f)
        {
            return v;
        }
        if (v == 0.0f)
        {
            v = -0.0f;
        }

        uint32_t bit_value = float_to_bits(v);
        if (v > 0)
        {
            --bit_value;
        }
        else
        {
            ++bit_value;
        }

        return bits_to_float(bit_value);
    }

    LOQUAT_CPU_GPU
    inline double next_float_down(double v)
    {
        if (is_inf(v) && v < 0.0f)
        {
            return v;
        }
        if (v == 0.0f)
        {
            v = -0.0f;
        }

        uint64_t bit_value = float_to_bits(v);
        if (v > 0)
        {
            --bit_value;
        }
        else
        {
            ++bit_value;
        }

        return bits_to_float(bit_value);
    }

    inline constexpr Float gamma(int n)
    {
        return (n * MACHINE_EPSILON) / (1 - n * MACHINE_EPSILON);
    }


    LOQUAT_CPU_GPU
	inline Float add_round_up(Float a, Float b) {
#ifdef LOQUAT_IS_GPU_CODE
#ifdef DOUBLE_PRECISION_FLOAT
        return __dadd_ru(a, b);
#else
        return __fadd_ru(a, b);
#endif
#else  // GPU
        return next_float_up(a + b);
#endif
    }
    LOQUAT_CPU_GPU
	inline Float add_round_down(Float a, Float b) {
#ifdef LOQUAT_IS_GPU_CODE
#ifdef DOUBLE_PRECISION_FLOAT
        return __dadd_rd(a, b);
#else
        return __fadd_rd(a, b);
#endif
#else  // GPU
        return next_float_down(a + b);
#endif
    }

    LOQUAT_CPU_GPU
	inline Float sub_round_up(Float a, Float b) {
        return add_round_up(a, -b);
    }
    LOQUAT_CPU_GPU
	inline Float sub_round_down(Float a, Float b) {
        return add_round_down(a, -b);
    }

    LOQUAT_CPU_GPU
	inline Float mul_round_up(Float a, Float b) {
#ifdef LOQUAT_IS_GPU_CODE
#ifdef DOUBLE_PRECISION_FLOAT
        return __dmul_ru(a, b);
#else
        return __fmul_ru(a, b);
#endif
#else  // GPU
        return next_float_up(a * b);
#endif
    }

    LOQUAT_CPU_GPU
	inline Float mul_round_down(Float a, Float b) {
#ifdef LOQUAT_IS_GPU_CODE
#ifdef DOUBLE_PRECISION_FLOAT
        return __dmul_rd(a, b);
#else
        return __fmul_rd(a, b);
#endif
#else  // GPU
        return next_float_down(a * b);
#endif
    }

    LOQUAT_CPU_GPU
	inline Float div_round_up(Float a, Float b) {
#ifdef LOQUAT_IS_GPU_CODE
#ifdef DOUBLE_PRECISION_FLOAT
        return __ddiv_ru(a, b);
#else
        return __fdiv_ru(a, b);
#endif
#else  // GPU
        return next_float_up(a / b);
#endif
    }

    LOQUAT_CPU_GPU
	inline Float div_round_down(Float a, Float b) {
#ifdef LOQUAT_IS_GPU_CODE
#ifdef DOUBLE_PRECISION_FLOAT
        return __ddiv_rd(a, b);
#else
        return __fdiv_rd(a, b);
#endif
#else  // GPU
        return next_float_down(a / b);
#endif
    }

    LOQUAT_CPU_GPU
	inline Float sqrt_round_up(Float a) {
#ifdef LOQUAT_IS_GPU_CODE
#ifdef DOUBLE_PRECISION_FLOAT
        return __dsqrt_ru(a);
#else
        return __fsqrt_ru(a);
#endif
#else  // GPU
        return next_float_up(std::sqrt(a));
#endif
    }

    LOQUAT_CPU_GPU
	inline Float sqrt_round_down(Float a) {
#ifdef LOQUAT_IS_GPU_CODE
#ifdef DOUBLE_PRECISION_FLOAT
        return __dsqrt_rd(a);
#else
        return __fsqrt_rd(a);
#endif
#else  // GPU
        return std::max<Float>(0, next_float_down(std::sqrt(a)));
#endif
    }

    LOQUAT_CPU_GPU
	inline Float FMA_round_up(Float a, Float b, Float c) {
#ifdef LOQUAT_IS_GPU_CODE
#ifdef DOUBLE_PRECISION_FLOAT
        return __fma_ru(a, b, c);  //TODO(ches) fix this
#else
        return __fma_ru(a, b, c);
#endif
#else  // GPU
        return next_float_up(FMA(a, b, c));
#endif
    }

    LOQUAT_CPU_GPU
	inline Float FMA_round_down(Float a, Float b, Float c) {
#ifdef LOQUAT_IS_GPU_CODE
#ifdef DOUBLE_PRECISION_FLOAT
        return __fma_rd(a, b, c);  //TODO(ches) fix this
#else
        return __fma_rd(a, b, c);
#endif
#else  // GPU
        return next_float_down(FMA(a, b, c));
#endif
    }

    //TODO(ches) finish this
}