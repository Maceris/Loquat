// pbrt is Copyright(c) 1998-2020 Matt Pharr, Wenzel Jakob, and Greg Humphreys.
// The pbrt source code is licensed under the Apache License, Version 2.0.
// SPDX: Apache-2.0

// This file has been modified from the original, original notice is above.

#pragma once

#include <bit>
#include <cmath>
#include <concepts>
#include <limits>
#include <string>

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
    inline double flip_sign(double a, double b)
    {
        return bits_to_float(float_to_bits(a) ^ sign_bit(b));
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
	inline Float add_round_up(Float a, Float b)
    {
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
	inline Float add_round_down(Float a, Float b)
    {
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
	inline Float sub_round_up(Float a, Float b)
    {
        return add_round_up(a, -b);
    }
    LOQUAT_CPU_GPU
	inline Float sub_round_down(Float a, Float b)
    {
        return add_round_down(a, -b);
    }

    LOQUAT_CPU_GPU
	inline Float mul_round_up(Float a, Float b)
    {
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
	inline Float mul_round_down(Float a, Float b)
    {
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
	inline Float div_round_up(Float a, Float b)
    {
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
	inline Float div_round_down(Float a, Float b)
    {
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
	inline Float sqrt_round_up(Float a)
    {
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
	inline Float sqrt_round_down(Float a)
    {
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
	inline Float FMA_round_up(Float a, Float b, Float c)
    {
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
	inline Float FMA_round_down(Float a, Float b, Float c)
    {
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

    static const int HALF_EXPONENT_MASK = 0b0111110000000000;
    static const int HALF_SIGNIFICAND_MASK = 0b1111111111;
    static const int HALF_NEGATIVE_ZERO = 0b1000000000000000;
    static const int HALF_POSITIVE_ZERO = 0;
    static const int HALF_NEGATIVE_INFINITY = 0b1111110000000000;
    static const int HALF_POSITIVE_INFINITY = 0b0111110000000000;

    namespace
    {
        //TODO(ches) support for non-AVX systems, check CPUID
        // https://gist.github.com/rygorous/2156668
        union FP32
        {
            uint32_t u;
            float f;
            struct
            {
                unsigned int Mantissa : 23;
                unsigned int Exponent : 8;
                unsigned int Sign : 1;
            };
        };

        union FP16
        {
            uint16_t u;
            struct
            {
                unsigned int Mantissa : 10;
                unsigned int Exponent : 5;
                unsigned int Sign : 1;
            };
        };
    }

    class Half
    {
    public:
        Half() = default;
        Half(const Half&) = default;
        Half& operator=(const Half&) = default;
        
        LOQUAT_CPU_GPU
        static Half from_bits(uint16_t v)
        {
            return Half(v);
        }

        LOQUAT_CPU_GPU
        explicit Half(float ff)
        {
#ifdef LOQUAT_IS_GPU_CODE
            h = __half_as_ushort(__float2half(ff));
#else
            // Rounding ties to nearest even instead of towards + inf
            FP32 f;
            f.f = ff;
            FP32 f32_infinity = { 25 << 23 };
            FP32 f16_max = { (127 + 16) << 23 };
            FP32 denorm_magic = { ((127 - 15) + (23 - 10) + 1) << 23 };
            unsigned int sign_mask = 0x80000000u;
            FP16 o = { 0 };

            unsigned int sign = f.u & sign_mask;
            f.u ^= sign;

            // NOTE all the integer compares in this function can be safely
            // compiled into signed compares since all operands are below
            // 0x80000000. Important if you want fast straight SSE2 code
            // (since there's no unsigned PCMPGTD).

            if (f.u >= f16_max.u)
            {
                // Result is Inf or NaN (all exponent bits set)
                // NaN->qNaN and Inf->Inf, respectively
                o.u = (f.u > f32_infinity.u) ? 0x7e00 : 0x7c00;
            }
            else
            {
                // (De)normalized number or zero
                if (f.u < (113 << 23))
                {
                    // resulting FP16 is subnormal or zero
                    // use a magic value to align our 10 mantissa bits at the
                    // bottom of the float. as long as FP addition is
                    // round-to-nearest-even this just works.
                    f.f += denorm_magic.f;

                    // and one integer subtract of the bias later, we have our
                    // final float!
                    o.u = f.u - denorm_magic.u;
                }
                else
                {
                    // resulting mantissa is odd
                    unsigned int mant_odd = (f.u >> 13) & 1;

                    // update exponent, rounding bias part 1
                    f.u += (uint32_t(15 - 127) << 23) + 0xfff;
                    // rounding bias part 2
                    f.u += mant_odd;
                    // take the bits!
                    o.u = f.u >> 13;
                }
            }
            o.u |= sign >> 16;
            h = o.u;
#endif
        }

        LOQUAT_CPU_GPU
        explicit Half(double d)
            : Half(float(d))
        {}

        LOQUAT_CPU_GPU
        explicit operator float() const
        {
#ifdef LOQUAT_IS_GPU_CODE
            return __half2float(__ushort_as_half(h));
#else
            FP16 h;
            h.u = this->h;
            static const FP32 magic = { 113 << 23 };
            // Exponent mask after shift
            static const unsigned int shifted_exp = 0x7c00 << 13;

            FP32 o;

            // exponent/mantissa bits
            o.u = (h.u & 0x7fff) << 13;
            // just the exponent
            unsigned int exp = shifted_exp & o.u;
            // exponent adjust
            o.u += (127 - 15) << 23;

            // handle exponent special cases
            // Inf/NaN?
            if (exp == shifted_exp)
            {
                // extra exp adjust
                o.u += (128 - 16) << 23;
            }
            else if (exp == 0)
            {
                // Zero/Denormal?
                // extra exp adjust
                o.u += 1 << 23;
                // renormalize
                o.f -= magic.f;
            }

            // sign bit
            o.u |= (h.u & 0x8000) << 16;
            return o.f;
#endif
        }

        LOQUAT_CPU_GPU
        explicit operator double() const
        {
            return (float)(*this);
        }

        LOQUAT_CPU_GPU
        bool operator==(const Half& v) const
        {
#if defined(LOQUAT_IS_GPU_CODE) && __CUDA_ARCH__ >= 530
            return __ushort_as_half(h) == __ushort_as_half(v.h);
#else
            if (bits() == v.bits())
            {
                return true;
            }
            return (
                (bits() == HALF_NEGATIVE_ZERO && v.bits() == HALF_POSITIVE_ZERO) ||
                (bits() == HALF_POSITIVE_ZERO && v.bits() == HALF_NEGATIVE_ZERO)
                );
#endif
        }

        LOQUAT_CPU_GPU
        bool operator!=(const Half& v) const
        {
            return !(*this == v);
        }

        LOQUAT_CPU_GPU
        Half operator-() const 
        {
            return from_bits(h ^ (1 << 15));
        }

        LOQUAT_CPU_GPU
        uint16_t bits() const
        {
            return h;
        }

        LOQUAT_CPU_GPU
        int sign()
        {
            return (h >> 15) ? -1 : 1;
        }

        LOQUAT_CPU_GPU
        bool is_inf()
        {
            return h == HALF_POSITIVE_INFINITY || h == HALF_NEGATIVE_INFINITY;
        }

        LOQUAT_CPU_GPU
        bool is_NaN()
        {
            return (
                (h & HALF_EXPONENT_MASK) == HALF_EXPONENT_MASK &&
                (h & HALF_SIGNIFICAND_MASK) != 0
                );
        }

        LOQUAT_CPU_GPU
        Half next_up()
        {
            if (is_inf() && sign() == 1)
            {
                return *this;
            }

            Half up = *this;
            if (up.h == HALF_NEGATIVE_ZERO)
            {
                up.h = HALF_POSITIVE_ZERO;
            }
            // Advance _v_ to next higher float
            if (up.sign() >= 0)
            {
                ++up.h;
            }
            else
            {
                --up.h;
            }
            return up;
        }

        LOQUAT_CPU_GPU
        Half next_down()
        {
            if (is_inf() && sign() == -1)
            {
                return *this;
            }

            Half down = *this;
            if (down.h == HALF_POSITIVE_ZERO)
            {
                down.h = HALF_NEGATIVE_ZERO;
            }
            if (down.sign() >= 0)
            {
                --down.h;
            }
            else
            {
                ++down.h;
            }
            return down;
        }

        [[nodiscard]]
        std::string to_string() const noexcept;
        
    private:
        LOQUAT_CPU_GPU
        explicit Half(uint16_t h)
            : h(h)
        {}

        uint16_t h;
    };
}