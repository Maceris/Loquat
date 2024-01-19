// pbrt is Copyright(c) 1998-2020 Matt Pharr, Wenzel Jakob, and Greg Humphreys.
// The pbrt source code is licensed under the Apache License, Version 2.0.
// SPDX: Apache-2.0

// This file has been modified from the original, original notice is above.

#pragma once

#include <bit>
#include <cmath>
#include <concepts>
#include <limits>

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

    static constexpr Float NaN = std::numeric_limits<Float>::has_signaling_NaN
        ? std::numeric_limits<Float>::signaling_NaN()
        : std::numeric_limits<Float>::quiet_NaN();

    template <std::floating_point T>
    inline T is_NaN(T v)
    {
        return std::isnan(v);
    }

    template <std::integral T>
    inline T is_NaN(T v)
    {
        return false;
    }

    template <std::floating_point T>
    inline T is_inf(T v)
    {
        return std::isinf(v);
    }

    template <std::integral T>
    inline T is_inf(T v)
    {
        return false;
    }

    template <std::floating_point T>
    inline T is_finite(T v)
    {
        return std::isfinite(v);
    }

    template <std::integral T>
    inline T is_finite(T v)
    {
        return true;
    }

    template <std::floating_point T>
    inline T FMA(T a, T b, T c)
    {
        return std::fma(a, b, c);
    }

    inline Float bits_to_float(FloatBits i)
    {
        return std::bit_cast<Float>(i);
    }

    inline FloatBits float_to_bits(Float f)
    {
        return std::bit_cast<FloatBits>(f);
    }


    inline int exponent_float(float f)
    {
        return (std::bit_cast<uint32_t>(f) >> 23) - 127;
    }

    inline int exponent_double(double d)
    {
        return (std::bit_cast<uint64_t>(d) >> 52) - 1023;
    }

    inline int exponent(Float f)
    {
        if constexpr (std::is_same_v<Float, float>)
        {
            return exponent_float(f);
        }
        else
        {
            return exponent_double(f);
        }
    }

    inline int significand_float(float f)
    {
        return std::bit_cast<uint32_t>(f) & ((1 << 23) - 1);
    }

    inline uint64_t significand_double(double d)
    {
        return std::bit_cast<uint64_t>(d) & ((1LL << 52) - 1);
    }

    inline FloatBits significand(Float f)
    {
        if constexpr (std::is_same_v<Float, float>)
        {
            return std::bit_cast<FloatBits>(f)& ((1 << 23) - 1);
        }
        else
        {
            return std::bit_cast<FloatBits>(f) & ((1LL << 52) - 1);
        }
    }

    inline int sign_bit_float(float f)
    {
        return (std::bit_cast<uint32_t>(f) >> 31) & 0x1;
    }

    inline int sign_bit_double(double d)
    {
        return (std::bit_cast<uint64_t>(d) >> 63) & 0x1;
    }

    inline int sign_bit(Float f)
    {
        if constexpr (std::is_same_v<Float, float>)
        {
            return sign_bit_float(f);
        }
        else
        {
            return sign_bit_double(f);
        }
    }

    inline Float next_float_up(Float v)
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

    inline Float next_float_down(Float v)
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

    inline constexpr Float gamma(int n)
    {
        return (n * MACHINE_EPSILON) / (1 - n * MACHINE_EPSILON);
    }

    inline Float add_round_up(Float a, Float b)
    {
        return next_float_up(a + b);
    }

    inline Float add_round_down(Float a, Float b)
    {
        return next_float_down(a + b);
    }

    inline Float sub_round_up(Float a, Float b)
    {
        return next_float_up(a - b);
    }

    inline Float sub_round_down(Float a, Float b)
    {
        return next_float_down(a - b);
    }

    inline Float mul_round_up(Float a, Float b)
    {
        return next_float_up(a * b);
    }

    inline Float mul_round_down(Float a, Float b)
    {
        return next_float_down(a * b);
    }

    inline Float div_round_up(Float a, Float b)
    {
        return next_float_up(a / b);
    }

    inline Float div_round_down(Float a, Float b)
    {
        return next_float_down(a / b);
    }

    inline Float sqrt_round_up(Float a)
    {
        return next_float_up(std::sqrt(a));
    }

    inline Float sqrt_round_down(Float a)
    {
        return next_float_down(std::sqrt(a));
    }

    inline Float FMA_round_up(Float a, Float b, Float c)
    {
        return next_float_up(FMA(a, b, c));
    }

    inline Float FMA_round_down(Float a, Float b, Float c)
    {
        return next_float_down(FMA(a, b, c));
    }
}