// pbrt is Copyright(c) 1998-2020 Matt Pharr, Wenzel Jakob, and Greg Humphreys.
// The pbrt source code is licensed under the Apache License, Version 2.0.
// SPDX: Apache-2.0

// This file has been modified from the original, original notice is above.

#pragma once

#include <algorithm>
#include <cmath>
#include <concepts>
#include <cstdint>
#include <cstring>
#include <limits>
#include <numbers>
#include <string>
#include <type_traits>
#include <utility>

#ifdef LOQUAT_HAS_INTRIN_H
#include <intrin.h>
#endif

#include "debug/logger.h"
#include "main/loquat.h"
#include "pbr/util/pstd.h"

namespace loquat
{

#ifdef LOQUAT_IS_GPU_CODE

#define SHADOW_EPSILON 0.0001f
#define PI Float(3.14159265358979323846)
#define INV_PI Float(0.31830988618379067154)
#define INV_2PI Float(0.15915494309189533577)
#define INV_4PI Float(0.07957747154594766788)
#define PI_OVER_2 Float(1.57079632679489661923)
#define PI_OVER_4 Float(0.78539816339744830961)
#define SQRT2 Float(1.41421356237309504880)

#else
    /// <summary>
    /// To avoid incorrect intersections with surfaces due to floating point
    /// errors, this is used to set t_max just under 1 to stop before 
    /// hitting light source surfaces.
    /// </summary>
    constexpr Float SHADOW_EPSILON = 0.0001f;

    constexpr Float PI = 3.14159265358979323846;
    constexpr Float INV_PI = 0.31830988618379067154;
    constexpr Float INV_2PI = 0.15915494309189533577;
    constexpr Float INV_4PI = 0.07957747154594766788;
    constexpr Float PI_OVER_2 = 1.57079632679489661923;
    constexpr Float PI_OVER_4 = 0.78539816339744830961;
    constexpr Float SQRT2 = 1.41421356237309504880;

#endif

    /// <summary>
    /// A concept for checking if a type is either an integral or a
    /// floating point value.
    /// </summary>
    template <typename T>
    concept number = std::integral<T> || std::floating_point<T>;

    LOQUAT_CPU_GPU
    inline uint32_t reverse_bits_32(uint32_t n)
    {
#ifdef LOQUAT_IS_GPU_CODE
        return __brev(n);
#else
        n = (n << 16) | (n >> 16);
        n = ((n & 0x00ff00ff) << 8) | ((n & 0xff00ff00) >> 8);
        n = ((n & 0x0f0f0f0f) << 4) | ((n & 0xf0f0f0f0) >> 4);
        n = ((n & 0x33333333) << 2) | ((n & 0xcccccccc) >> 2);
        n = ((n & 0x55555555) << 1) | ((n & 0xaaaaaaaa) >> 1);
        return n;
#endif
    }

    LOQUAT_CPU_GPU
    inline uint64_t reverse_bits_64(uint64_t n)
    {
#ifdef LOQUAT_IS_GPU_CODE
        return __brevll(n);
#else
        uint64_t n0 = reverse_bits_32((uint32_t)n);
        uint64_t n1 = reverse_bits_32((uint32_t)(n >> 32));
        return (n0 << 32) | n1;
#endif
    }

    // https://fgiesen.wordpress.com/2009/12/13/decoding-morton-codes/
    // updated to 64 bits.
    LOQUAT_CPU_GPU
    inline uint64_t left_shift_2(uint64_t x)
    {
        x &= 0xffffffff;
        x = (x ^ (x << 16)) & 0x0000ffff0000ffff;
        x = (x ^ (x << 8)) & 0x00ff00ff00ff00ff;
        x = (x ^ (x << 4)) & 0x0f0f0f0f0f0f0f0f;
        x = (x ^ (x << 2)) & 0x3333333333333333;
        x = (x ^ (x << 1)) & 0x5555555555555555;
        return x;
    }

    LOQUAT_CPU_GPU
    inline uint64_t encode_morton_2(uint32_t x, uint32_t y)
    {
        return (left_shift_2(y) << 1) | left_shift_2(x);
    }

    LOQUAT_CPU_GPU
    inline uint32_t left_shift_3(uint32_t x)
    {
        LOG_ASSERT(x <= (1u << 10));
        if (x == (1 << 10))
            --x;
        x = (x | (x << 16)) & 0b00000011000000000000000011111111;
        // x = ---- --98 ---- ---- ---- ---- 7654 3210
        x = (x | (x << 8)) & 0b00000011000000001111000000001111;
        // x = ---- --98 ---- ---- 7654 ---- ---- 3210
        x = (x | (x << 4)) & 0b00000011000011000011000011000011;
        // x = ---- --98 ---- 76-- --54 ---- 32-- --10
        x = (x | (x << 2)) & 0b00001001001001001001001001001001;
        // x = ---- 9--8 --7- -6-- 5--4 --3- -2-- 1--0
        return x;
    }

    LOQUAT_CPU_GPU
    inline uint32_t encode_morton_3(float x, float y, float z)
    {
        LOG_ASSERT(x >= 0);
        LOG_ASSERT(y >= 0);
        LOG_ASSERT(z >= 0);
        return (left_shift_3(z) << 2) | (left_shift_3(y) << 1) | left_shift_3(x);
    }

    LOQUAT_CPU_GPU
    inline uint32_t compact_1by1(uint64_t x)
    {
        // TODO(ches): as of Haswell, the PEXT instruction could do all this in a
        // single instruction.
        // x = -f-e -d-c -b-a -9-8 -7-6 -5-4 -3-2 -1-0
        x &= 0x5555555555555555;
        // x = --fe --dc --ba --98 --76 --54 --32 --10
        x = (x ^ (x >> 1)) & 0x3333333333333333;
        // x = ---- fedc ---- ba98 ---- 7654 ---- 3210
        x = (x ^ (x >> 2)) & 0x0f0f0f0f0f0f0f0f;
        // x = ---- ---- fedc ba98 ---- ---- 7654 3210
        x = (x ^ (x >> 4)) & 0x00ff00ff00ff00ff;
        // x = ---- ---- ---- ---- fedc ba98 7654 3210
        x = (x ^ (x >> 8)) & 0x0000ffff0000ffff;
        // ...
        x = (x ^ (x >> 16)) & 0xffffffff;
        return x;
    }

    LOQUAT_CPU_GPU
    inline void decode_morton_2(uint64_t v, uint32_t* x, uint32_t* y)
    {
        *x = compact_1by1(v);
        *y = compact_1by1(v >> 1);
    }

    LOQUAT_CPU_GPU
    inline uint32_t compact_1by2(uint32_t x)
    {
        x &= 0x09249249;                   // x = ---- 9--8 --7- -6-- 5--4 --3- -2-- 1--0
        x = (x ^ (x >> 2)) & 0x030c30c3;   // x = ---- --98 ---- 76-- --54 ---- 32-- --10
        x = (x ^ (x >> 4)) & 0x0300f00f;   // x = ---- --98 ---- ---- 7654 ---- ---- 3210
        x = (x ^ (x >> 8)) & 0xff0000ff;   // x = ---- --98 ---- ---- ---- ---- 7654 3210
        x = (x ^ (x >> 16)) & 0x000003ff;  // x = ---- ---- ---- ---- ---- --98 7654 3210
        return x;
    }
    
    template <typename Float>
    class CompensatedSum
    {
    public:
        
        CompensatedSum() = default;
        LOQUAT_CPU_GPU
        explicit CompensatedSum(Float v)
            : sum(v)
        {}

        LOQUAT_CPU_GPU
        CompensatedSum& operator=(Float v)
        {
            sum = v;
            c = 0;
            return *this;
        }

        LOQUAT_CPU_GPU
        CompensatedSum& operator+=(Float v)
        {
            Float delta = v - c;
            Float newSum = sum + delta;
            c = (newSum - sum) - delta;
            sum = newSum;
            return *this;
        }

        LOQUAT_CPU_GPU
        explicit operator Float() const
        {
            return sum;
        }

        std::string to_string() const;

    private:
        Float sum = 0;
        Float c = 0;
    };

    
    struct CompensatedFloat
    {
    public:
        
        LOQUAT_CPU_GPU
            CompensatedFloat(Float v, Float err = 0)
            : v(v)
            , err(err)
        {}
        LOQUAT_CPU_GPU
        explicit operator float() const
        {
            return v + err;
        }

        LOQUAT_CPU_GPU
        explicit operator double() const
        {
            return double(v) + double(err);
        }

        std::string to_string() const;

        Float v;
        Float err;
    };

    template <int N>
    class SquareMatrix;
    LOQUAT_CPU_GPU
    inline Float sin_x_over_x(Float x);

    
    LOQUAT_CPU_GPU
    inline Float lerp(Float x, Float a, Float b)
    {
        return (1 - x) * a + x * b;
    }

    template <typename T>
    inline LOQUAT_CPU_GPU
    typename std::enable_if_t<std::is_integral_v<T>, T> FMA(T a, T b, T c)
    {
        return a * b + c;
    }

    LOQUAT_CPU_GPU
    inline Float sinc(Float);

    LOQUAT_CPU_GPU
    inline Float windowed_sinc(Float x, Float radius, Float tau)
    {
        if (std::abs(x) > radius)
        {
            return 0;
        }
        return sinc(x) * sinc(x / tau);
    }

    LOQUAT_CPU_GPU
    inline Float sinc(Float x)
    {
        return sin_x_over_x(PI * x);
    }

#ifdef LOQUAT_IS_MSVC
#pragma warning(push)
#pragma warning(disable : 4018)  // signed/unsigned mismatch
#endif

    template <typename T, typename U, typename V>
    LOQUAT_CPU_GPU
    inline constexpr T clamp(T val, U low, V high)
    {
        if (val < low)
        {
            return T(low);
        }
        else if (val > high)
        {
            return T(high);
        }
        else
        {
            return val;
        }
    }

#ifdef LOQUAT_IS_MSVC
#pragma warning(pop)
#endif

    template <typename T>
    LOQUAT_CPU_GPU
    inline T mod(T a, T b)
    {
        T result = a - (a / b) * b;
        return (T)((result < 0) ? result + b : result);
    }

    template <>
    LOQUAT_CPU_GPU
    inline Float mod(Float a, Float b)
    {
        return std::fmod(a, b);
    }

    LOQUAT_CPU_GPU
    inline Float radians(Float deg)
    {
        return (PI / 180) * deg;
    }
    LOQUAT_CPU_GPU
    inline Float degrees(Float rad)
    {
        return (180 / PI) * rad;
    }

    LOQUAT_CPU_GPU
    inline Float smooth_step(Float x, Float a, Float b)
    {
        if (a == b)
        {
            return (x < a) ? 0 : 1;
        }
        LOG_ASSERT(a < b);
        Float t = clamp((x - a) / (b - a), 0, 1);
        return t * t * (3 - 2 * t);
    }

    LOQUAT_CPU_GPU
    inline float safe_square_root(float x)
    {
        LOG_ASSERT(x >= -1e-3f);  // not too negative
        return std::sqrt(std::max(0.f, x));
    }

    LOQUAT_CPU_GPU
    inline double safe_square_root(double x)
    {
        LOG_ASSERT(x >= -1e-3);  // not too negative
        return std::sqrt(std::max(0., x));
    }

    template <typename T>
    LOQUAT_CPU_GPU
    inline constexpr T square(T v)
    {
        return v * v;
    }

    // Would be nice to allow Float to be a template type here, but it is tricky:
    // https://stackoverflow.com/questions/5101516/why-function-template-cannot-be-partially-specialized
    template <int n>
    LOQUAT_CPU_GPU
    inline constexpr float pow(float v)
    {
        if constexpr (n < 0)
            return 1 / pow<-n>(v);
        float n2 = pow<n / 2>(v);
        return n2 * n2 * pow<n & 1>(v);
    }

    template <>
    LOQUAT_CPU_GPU
    inline constexpr float pow<1>(float v)
    {
        return v;
    }

    template <>
    LOQUAT_CPU_GPU
    inline constexpr float pow<0>(float v)
    {
        return 1;
    }

    template <int n>
    LOQUAT_CPU_GPU
    inline constexpr double pow(double v)
    {
        if constexpr (n < 0)
            return 1 / pow<-n>(v);
        double n2 = pow<n / 2>(v);
        return n2 * n2 * pow<n & 1>(v);
    }

    template <>
    LOQUAT_CPU_GPU
    inline constexpr double pow<1>(double v)
    {
        return v;
    }

    template <>
    LOQUAT_CPU_GPU
    inline constexpr double pow<0>(double v)
    {
        return 1;
    }

    template <typename Float, typename C>
    LOQUAT_CPU_GPU
    inline constexpr Float evaluate_polynomial(Float t, C c)
    {
        return c;
    }

    template <typename Float, typename C, typename... Args>
    LOQUAT_CPU_GPU
    inline constexpr Float evaluate_polynomial(Float t, C c, Args... cRemaining)
    {
        return FMA(t, evaluate_polynomial(t, cRemaining...), c);
    }

    // http://www.plunk.org/~hatch/rightway.html
    LOQUAT_CPU_GPU
    inline Float sin_x_over_x(Float x)
    {
        if (1 - x * x == 1)
        {
            return 1;
        }
        return std::sin(x) / x;
    }

    LOQUAT_CPU_GPU
    inline float safe_asin(float x)
    {
        LOG_ASSERT(x >= -1.0001 && x <= 1.0001);
        return std::asin(clamp(x, -1, 1));
    }

    LOQUAT_CPU_GPU
    inline float safe_acos(float x)
    {
        LOG_ASSERT(x >= -1.0001 && x <= 1.0001);
        return std::acos(clamp(x, -1, 1));
    }

    LOQUAT_CPU_GPU
    inline double safe_asin(double x)
    {
        LOG_ASSERT(x >= -1.0001 && x <= 1.0001);
        return std::asin(clamp(x, -1, 1));
    }

    LOQUAT_CPU_GPU
    inline double safe_acos(double x)
    {
        LOG_ASSERT(x >= -1.0001 && x <= 1.0001);
        return std::acos(clamp(x, -1, 1));
    }

    LOQUAT_CPU_GPU
    inline Float log2(Float x)
    {
        const Float invLog2 = 1.442695040888963387004650940071;
        return std::log(x) * invLog2;
    }

    LOQUAT_CPU_GPU inline int log2_int(float v)
    {
        LOG_ASSERT(v > 0);
        if (v < 1)
            return -log2_int(1 / v);
        // https://graphics.stanford.edu/~seander/bithacks.html#IntegerLog
        // (With an additional check of the significant to get round-to-nearest
        // rather than round down.)
        // midsignif = significand(std::pow(2., 1.5))
        // i.e. grab the significand of a value halfway between two exponents,
        // in log space.
        const uint32_t midsignif = 0b00000000001101010000010011110011;
        return exponent(v) + ((significand(v) >= midsignif) ? 1 : 0);
    }

    LOQUAT_CPU_GPU
    inline int log2_int(double v)
    {
        LOG_ASSERT(v > 0);
        if (v < 1)
        {
            return -log2_int(1 / v);
        }
        // https://graphics.stanford.edu/~seander/bithacks.html#IntegerLog
        // (With an additional check of the significant to get round-to-nearest
        // rather than round down.)
        // midsignif = significand(std::pow(2., 1.5))
        // i.e. grab the significand of a value halfway between two exponents,
        // in log space.
        const uint64_t midsignif = 0b110101000001001111001100110011111110011101111001101;
        return exponent(v) + ((significand(v) >= midsignif) ? 1 : 0);
    }

    LOQUAT_CPU_GPU
    inline int log2_int(uint32_t v)
    {
#ifdef LOQUAT_IS_GPU_CODE
        return 31 - __clz(v);
#elif defined(LOQUAT_HAS_INTRIN_H)
        unsigned long lz = 0;
        if (_BitScanReverse(&lz, v))
            return lz;
        return 0;
#else
        return 31 - __builtin_clz(v);
#endif
    }

    LOQUAT_CPU_GPU
    inline int log2_int(int32_t v)
    {
        return log2_int((uint32_t)v);
    }

    LOQUAT_CPU_GPU
    inline int log2_int(uint64_t v)
    {
#ifdef LOQUAT_IS_GPU_CODE
        return 64 - __clzll(v);
#elif defined(LOQUAT_HAS_INTRIN_H)
        unsigned long lz = 0;
#if defined(_WIN64)
        _BitScanReverse64(&lz, v);
#else
        if (_BitScanReverse(&lz, v >> 32))
            lz += 32;
        else
            _BitScanReverse(&lz, v & 0xffffffff);
#endif  // _WIN64
        return lz;
#else   // LOQUAT_HAS_INTRIN_H
        return 63 - __builtin_clzll(v);
#endif
    }

    LOQUAT_CPU_GPU
    inline int log2_int(int64_t v)
    {
        return log2_int(static_cast<uint64_t>(v));
    }

    template <typename T>
    LOQUAT_CPU_GPU
    inline int log4_int(T v)
    {
        return log2_int(v) / 2;
    }

    // https://stackoverflow.com/a/10792321
    LOQUAT_CPU_GPU
    inline float fast_exp(float x)
    {
#ifdef LOQUAT_IS_GPU_CODE
        return __expf(x);
#else
        // Compute $x'$ such that $\roman{e}^x = 2^{x'}$
        float xp = x * 1.442695041f;

        // Find integer and fractional components of $x'$
        float fxp = pstd::floor(xp), f = xp - fxp;
        int i = (int)fxp;

        // Evaluate polynomial approximation of $2^f$
        float twoToF = evaluate_polynomial(f, 1.f, 0.695556856f, 0.226173572f, 0.0781455737f);

        // Scale $2^f$ by $2^i$ and return final result
        int exp = exponent(twoToF) + i;
        if (exp < -126)
        {
            return 0;
        }
        if (exp > 127)
        {
            return FLOAT_INFINITY;
        }
        uint32_t bits = float_to_bits(twoToF);
        bits &= 0b10000000011111111111111111111111u;
        bits |= (exp + 127) << 23;
        return bits_to_float(bits);
#endif
    }

    LOQUAT_CPU_GPU
    inline Float gaussian(Float x, Float mu = 0, Float sigma = 1)
    {
        return 1 / std::sqrt(2 * PI * sigma * sigma) *
            fast_exp(-square(x - mu) / (2 * sigma * sigma));
    }

    LOQUAT_CPU_GPU
    inline Float gaussian_integral(Float x0, Float x1, Float mu = 0,
        Float sigma = 1)
    {
        LOG_ASSERT(sigma > 0);
        Float sigmaRoot2 = sigma * Float(1.414213562373095);
        return 0.5f * (std::erf((mu - x0) / sigmaRoot2) - std::erf((mu - x1) / sigmaRoot2));
    }

    LOQUAT_CPU_GPU
    inline Float logistic(Float x, Float s)
    {
        x = std::abs(x);
        return std::exp(-x / s) / (s * square(1 + std::exp(-x / s)));
    }

    LOQUAT_CPU_GPU
    inline Float logistic_CDF(Float x, Float s)
    {
        return 1 / (1 + std::exp(-x / s));
    }

    LOQUAT_CPU_GPU
    inline Float trimmed_logistic(Float x, Float s, Float a, Float b)
    {
        LOG_ASSERT(a < b);
        return logistic(x, s) / (logistic_CDF(b, s) - logistic_CDF(a, s));
    }

    LOQUAT_CPU_GPU
    inline Float error_function_inverse(Float a);
    LOQUAT_CPU_GPU
    inline Float i0(Float x);
    LOQUAT_CPU_GPU
    inline Float log_i0(Float x);

    template <typename Predicate>
    LOQUAT_CPU_GPU
    inline size_t find_interval(size_t sz, const Predicate& pred)
    {
        using ssize_t = std::make_signed_t<size_t>;
        ssize_t size = (ssize_t)sz - 2, first = 1;
        while (size > 0) {
            // Evaluate predicate at midpoint and update _first_ and _size_
            size_t half = (size_t)size >> 1, middle = first + half;
            bool predResult = pred(middle);
            first = predResult ? middle + 1 : first;
            size = predResult ? size - (half + 1) : half;
        }
        return (size_t)clamp((ssize_t)first - 1, 0, sz - 2);
    }

    template <typename T>
    LOQUAT_CPU_GPU
    inline constexpr bool is_power_of_2(T v)
    {
        return v && !(v & (v - 1));
    }

    template <typename T>
    LOQUAT_CPU_GPU
    inline bool is_power_of_4(T v)
    {
        return v == 1 << (2 * log4_int(v));
    }

    LOQUAT_CPU_GPU
    inline constexpr int32_t round_up_pow_2(int32_t v)
    {
        v--;
        v |= v >> 1;
        v |= v >> 2;
        v |= v >> 4;
        v |= v >> 8;
        v |= v >> 16;
        return v + 1;
    }

    LOQUAT_CPU_GPU
    inline constexpr int64_t round_up_pow_2(int64_t v)
    {
        v--;
        v |= v >> 1;
        v |= v >> 2;
        v |= v >> 4;
        v |= v >> 8;
        v |= v >> 16;
        v |= v >> 32;
        return v + 1;
    }

    template <typename T>
    LOQUAT_CPU_GPU
    inline T round_up_pow_4(T v)
    {
        return is_power_of_4(v) ? v : (1 << (2 * (1 + log4_int(v))));
    }

    LOQUAT_CPU_GPU
    inline CompensatedFloat two_prod(Float a, Float b)
    {
        Float ab = a * b;
        return { ab, FMA(a, b, -ab) };
    }

    LOQUAT_CPU_GPU
    inline CompensatedFloat two_sum(Float a, Float b)
    {
        Float s = a + b, delta = s - a;
        return { s, (a - (s - delta)) + (b - delta) };
    }

    template <typename Ta, typename Tb, typename Tc, typename Td>
    LOQUAT_CPU_GPU
    inline auto difference_of_products(Ta a, Tb b, Tc c, Td d)
    {
        auto cd = c * d;
        auto differenceOfProducts = FMA(a, b, -cd);
        auto error = FMA(-c, d, cd);
        return differenceOfProducts + error;
    }

    template <typename Ta, typename Tb, typename Tc, typename Td>
    LOQUAT_CPU_GPU
    inline auto sum_of_products(Ta a, Tb b, Tc c, Td d)
    {
        auto cd = c * d;
        auto sumOfProducts = FMA(a, b, cd);
        auto error = FMA(c, d, -cd);
        return sumOfProducts + error;
    }

    namespace internal
    {
        
        template <typename Float>
        LOQUAT_CPU_GPU
        inline CompensatedFloat inner_product(Float a, Float b)
        {
            return two_prod(a, b);
        }

        // Accurate dot products with FMA: Graillat et al.,
        // https://www-pequan.lip6.fr/~graillat/papers/posterRNC7.pdf
        //
        // Accurate summation, dot product and polynomial evaluation in complex
        // floating point arithmetic, Graillat and Menissier-Morain.
        template <typename Float, typename... T>
        LOQUAT_CPU_GPU
        inline CompensatedFloat inner_product(Float a, Float b, T... terms)
        {
            CompensatedFloat ab = two_prod(a, b);
            CompensatedFloat tp = inner_product(terms...);
            CompensatedFloat sum = two_sum(ab.v, tp.v);
            return { sum.v, ab.err + (tp.err + sum.err) };
        }

    }

    template <typename... T>
    LOQUAT_CPU_GPU
    inline std::enable_if_t<std::conjunction_v<std::is_arithmetic<T>...>, Float>
    inner_product(T... terms)
    {
        CompensatedFloat ip = internal::inner_product(terms...);
        return Float(ip);
    }

    LOQUAT_CPU_GPU
    inline bool quadratic(float a, float b, float c, float* t0, float* t1)
    {
        // Handle case of $a=0$ for quadratic solution
        if (a == 0)
        {
            if (b == 0)
            {
                return false;
            }
            *t0 = *t1 = -c / b;
            return true;
        }

        // Find quadratic discriminant
        float discrim = difference_of_products(b, b, 4 * a, c);
        if (discrim < 0)
        {
            return false;
        }
        float rootDiscrim = std::sqrt(discrim);

        // Compute quadratic _t_ values
        float q = -0.5f * (b + pstd::copysign(rootDiscrim, b));
        *t0 = q / a;
        *t1 = c / q;
        if (*t0 > *t1)
        {
            pstd::swap(*t0, *t1);
        }

        return true;
    }

    LOQUAT_CPU_GPU
    inline bool quadratic(double a, double b, double c, double* t0, double* t1)
    {
        // Find quadratic discriminant
        double discrim = difference_of_products(b, b, 4 * a, c);
        if (discrim < 0)
        {
            return false;
        }
        double rootDiscrim = std::sqrt(discrim);

        if (a == 0)
        {
            *t0 = *t1 = -c / b;
            return true;
        }

        // Compute quadratic _t_ values
        double q = -0.5 * (b + pstd::copysign(rootDiscrim, b));
        *t0 = q / a;
        *t1 = c / q;
        if (*t0 > *t1)
        {
            pstd::swap(*t0, *t1);
        }
        return true;
    }

    template <typename Func>
    LOQUAT_CPU_GPU
    inline Float newton_bisection(Float x0, Float x1, Func f, Float xEps = 1e-6f,
        Float fEps = 1e-6f)
    {
        // Check function endpoints for roots
        LOG_ASSERT(x0 < x1);
        Float fx0 = f(x0).first, fx1 = f(x1).first;
        if (std::abs(fx0) < fEps)
        {
            return x0;
        }
        if (std::abs(fx1) < fEps)
        {
            return x1;
        }
        bool startIsNegative = fx0 < 0;

        // Set initial midpoint using linear approximation of _f_
        Float xMid = x0 + (x1 - x0) * -fx0 / (fx1 - fx0);

        while (true)
        {
            // Fall back to bisection if _xMid_ is out of bounds
            if (!(x0 < xMid && xMid < x1))
            {
                xMid = (x0 + x1) / 2;
            }

            // Evaluate function and narrow bracket range _[x0, x1]_
            std::pair<Float, Float> fxMid = f(xMid);
            LOG_ASSERT(!is_NaN(fxMid.first));
            if (startIsNegative == (fxMid.first < 0))
            {
                x0 = xMid;
            }
            else
            {
                x1 = xMid;
            }

            // Stop the iteration if converged
            if ((x1 - x0) < xEps || std::abs(fxMid.first) < fEps)
            {
                return xMid;
            }

            // Perform a Newton step
            xMid -= fxMid.first / fxMid.second;
        }
    }

    template <int N>
    pstd::optional<SquareMatrix<N>> linear_least_squares(const Float A[][N],
        const Float B[][N], int rows);

    template <int N>
    pstd::optional<SquareMatrix<N>> linear_least_squares(const Float A[][N],
        const Float B[][N], int rows)
    {
        SquareMatrix<N> AtA = SquareMatrix<N>::zero();
        SquareMatrix<N> AtB = SquareMatrix<N>::zero();

        for (int i = 0; i < N; ++i)
            for (int j = 0; j < N; ++j)
                for (int r = 0; r < rows; ++r) {
                    AtA[i][j] += A[r][i] * A[r][j];
                    AtB[i][j] += A[r][i] * B[r][j];
                }

        auto AtAi = inverse(AtA);
        if (!AtAi)
            return {};
        return transpose(*AtAi * AtB);
    }

    int next_prime(int x);
    
    LOQUAT_CPU_GPU
    inline int permutation_element(uint32_t i, uint32_t n, uint32_t seed);

    LOQUAT_CPU_GPU
    inline int permutation_element(uint32_t i, uint32_t l, uint32_t p)
    {
        uint32_t w = l - 1;
        w |= w >> 1;
        w |= w >> 2;
        w |= w >> 4;
        w |= w >> 8;
        w |= w >> 16;
        do {
            i ^= p;
            i *= 0xe170893d;
            i ^= p >> 16;
            i ^= (i & w) >> 4;
            i ^= p >> 8;
            i *= 0x0929eb3f;
            i ^= p >> 23;
            i ^= (i & w) >> 1;
            i *= 1 | p >> 27;
            i *= 0x6935fa69;
            i ^= (i & w) >> 11;
            i *= 0x74dcb303;
            i ^= (i & w) >> 2;
            i *= 0x9e501cc3;
            i ^= (i & w) >> 2;
            i *= 0xc860a3df;
            i &= w;
            i ^= i >> 5;
        } while (i >= l);
        return (i + p) % l;
    }

    LOQUAT_CPU_GPU
    inline Float error_function_inverse(Float a)
    {
#ifdef LOQUAT_IS_GPU_CODE
        return erfinv(a);
#else
        // https://stackoverflow.com/a/49743348
        float p;
        float t = std::log(std::max(FMA(a, -a, 1.0f), std::numeric_limits<Float>::min()));
        LOG_ASSERT(!is_NaN(t) && !is_inf(t));
        if (std::abs(t) > 6.125f) {          // maximum ulp error = 2.35793
            p = 3.03697567e-10f;             //  0x1.4deb44p-32
            p = FMA(p, t, 2.93243101e-8f);   //  0x1.f7c9aep-26
            p = FMA(p, t, 1.22150334e-6f);   //  0x1.47e512p-20
            p = FMA(p, t, 2.84108955e-5f);   //  0x1.dca7dep-16
            p = FMA(p, t, 3.93552968e-4f);   //  0x1.9cab92p-12
            p = FMA(p, t, 3.02698812e-3f);   //  0x1.8cc0dep-9
            p = FMA(p, t, 4.83185798e-3f);   //  0x1.3ca920p-8
            p = FMA(p, t, -2.64646143e-1f);  // -0x1.0eff66p-2
            p = FMA(p, t, 8.40016484e-1f);   //  0x1.ae16a4p-1
        }
        else {                             // maximum ulp error = 2.35456
            p = 5.43877832e-9f;              //  0x1.75c000p-28
            p = FMA(p, t, 1.43286059e-7f);   //  0x1.33b458p-23
            p = FMA(p, t, 1.22775396e-6f);   //  0x1.49929cp-20
            p = FMA(p, t, 1.12962631e-7f);   //  0x1.e52bbap-24
            p = FMA(p, t, -5.61531961e-5f);  // -0x1.d70c12p-15
            p = FMA(p, t, -1.47697705e-4f);  // -0x1.35be9ap-13
            p = FMA(p, t, 2.31468701e-3f);   //  0x1.2f6402p-9
            p = FMA(p, t, 1.15392562e-2f);   //  0x1.7a1e4cp-7
            p = FMA(p, t, -2.32015476e-1f);  // -0x1.db2aeep-3
            p = FMA(p, t, 8.86226892e-1f);   //  0x1.c5bf88p-1
        }
        return a * p;
#endif  // LOQUAT_IS_GPU_CODE
    }

    LOQUAT_CPU_GPU
    inline Float i0(Float x)
    {
        Float val = 0;
        Float x2i = 1;
        int64_t ifact = 1;
        int i4 = 1;
        // i0(x) \approx Sum_i x^(2i) / (4^i (i!)^2)
        for (int i = 0; i < 10; ++i)
        {
            if (i > 1)
            {
                ifact *= i;
            }
            val += x2i / (i4 * square(ifact));
            x2i *= x * x;
            i4 *= 4;
        }
        return val;
    }

    LOQUAT_CPU_GPU
    inline Float log_i0(Float x)
    {
        if (x > 12)
        {
            return x + 0.5f * (-std::log(2 * PI) + std::log(1 / x) + 1 / (8 * x));
        }
        else
        {
            return std::log(i0(x));
        }
    }

    class Interval
    {
    public:
        
        Interval() = default;
        LOQUAT_CPU_GPU
        explicit Interval(Float v)
            : low(v)
            , high(v)
        {}
        LOQUAT_CPU_GPU
        constexpr Interval(Float low, Float high)
            : low(std::min(low, high))
            , high(std::max(low, high))
        {}

        LOQUAT_CPU_GPU
        static Interval from_value_and_error(Float v, Float err)
        {
            Interval i;
            if (err == 0)
            {
                i.low = i.high = v;
            }
            else
            {
                i.low = sub_round_down(v, err);
                i.high = add_round_up(v, err);
            }
            return i;
        }

        LOQUAT_CPU_GPU
        Interval& operator=(Float v)
        {
            low = high = v;
            return *this;
        }

        LOQUAT_CPU_GPU
        Float upper_bound() const
        {
            return high;
        }

        LOQUAT_CPU_GPU
        Float lower_bound() const
        {
            return low;
        }

        LOQUAT_CPU_GPU
        Float midpoint() const
        {
            return (low + high) / 2;
        }

        LOQUAT_CPU_GPU
        Float width() const
        {
            return high - low;
        }

        LOQUAT_CPU_GPU
        Float operator[](int i) const
        {
            LOG_ASSERT(i == 0 || i == 1);
            return (i == 0) ? low : high;
        }

        LOQUAT_CPU_GPU
        explicit operator Float() const
        {
            return midpoint();
        }

        LOQUAT_CPU_GPU
        bool exactly(Float v) const
        {
            return low == v && high == v;
        }

        LOQUAT_CPU_GPU
        bool operator==(Float v) const
        {
            return exactly(v);
        }

        LOQUAT_CPU_GPU
        Interval operator-() const
        {
            return { -high, -low };
        }

        LOQUAT_CPU_GPU
        Interval operator+(Interval i) const
        {
            return { add_round_down(low, i.low), add_round_up(high, i.high) };
        }

        LOQUAT_CPU_GPU
        Interval operator-(Interval i) const
        {
            return { sub_round_down(low, i.high), sub_round_up(high, i.low) };
        }

        LOQUAT_CPU_GPU
        Interval operator*(Interval i) const
        {
            Float lp[4] = {
                mul_round_down(low, i.low),
                mul_round_down(high, i.low),
                mul_round_down(low, i.high),
                mul_round_down(high, i.high)
            };
            Float hp[4] = {
                mul_round_up(low, i.low),
                mul_round_up(high, i.low),
                mul_round_up(low, i.high),
                mul_round_up(high, i.high)
            };
            return {
                std::min({lp[0], lp[1], lp[2], lp[3]}),
                std::max({hp[0], hp[1], hp[2], hp[3]}) 
            };
        }

        LOQUAT_CPU_GPU
        Interval operator/(Interval i) const;

        LOQUAT_CPU_GPU
        bool operator==(Interval i) const
        {
            return low == i.low && high == i.high;
        }

        LOQUAT_CPU_GPU
        bool operator!=(Float f) const
        {
            return f < low || f > high;
        }

        std::string to_string() const;

        LOQUAT_CPU_GPU
        Interval& operator+=(Interval i)
        {
            *this = Interval(*this + i);
            return *this;
        }

        LOQUAT_CPU_GPU 
        Interval& operator-=(Interval i)
        {
            *this = Interval(*this - i);
            return *this;
        }
        LOQUAT_CPU_GPU
        Interval& operator*=(Interval i)
        {
            *this = Interval(*this * i);
            return *this;
        }

        LOQUAT_CPU_GPU
        Interval& operator/=(Interval i)
        {
            *this = Interval(*this / i);
            return *this;
        }
        LOQUAT_CPU_GPU
        Interval& operator+=(Float f)
        {
            return *this += Interval(f);
        }

        LOQUAT_CPU_GPU
        Interval& operator-=(Float f)
        {
            return *this -= Interval(f);
        }

        LOQUAT_CPU_GPU
        Interval& operator*=(Float f)
        {
            if (f > 0)
            {
                *this = Interval(mul_round_down(f, low), mul_round_up(f, high));
            }
            else
            {
                *this = Interval(mul_round_down(f, high), mul_round_up(f, low));
            }
            return *this;
        }
        LOQUAT_CPU_GPU
        Interval& operator/=(Float f)
        {
            if (f > 0)
            {
                *this = Interval(div_round_down(low, f), div_round_up(high, f));
            }
            else
            {
                *this = Interval(div_round_down(high, f), div_round_up(low, f));
            }
            return *this;
        }

#ifndef LOQUAT_IS_GPU_CODE
        static const Interval PI;
#endif

    private:
        friend struct SOA<Interval>;
        
        Float low;
        Float high;
    };
    
    LOQUAT_CPU_GPU
    inline bool in_range(Float v, Interval i)
    {
        return v >= i.lower_bound() && v <= i.upper_bound();
    }

    LOQUAT_CPU_GPU
    inline bool in_range(Interval a, Interval b)
    {
        return a.lower_bound() <= b.upper_bound() 
            && a.upper_bound() >= b.lower_bound();
    }

    LOQUAT_CPU_GPU
    inline Interval Interval::operator/(Interval i) const
    {
        if (in_range(0, i))
        {
            // The interval we're dividing by straddles zero, so just
            // return an interval of everything.
            return Interval(-FLOAT_INFINITY, FLOAT_INFINITY);
        }

        Float lowQuot[4] = {
            div_round_down(low, i.low),
            div_round_down(high, i.low),
            div_round_down(low, i.high),
            div_round_down(high, i.high)
        };
        Float highQuot[4] = {
            div_round_up(low, i.low),
            div_round_up(high, i.low),
            div_round_up(low, i.high),
            div_round_up(high, i.high)
        };
        return {
            std::min({lowQuot[0], lowQuot[1], lowQuot[2], lowQuot[3]}),
            std::max({highQuot[0], highQuot[1], highQuot[2], highQuot[3]})
        };
    }

    LOQUAT_CPU_GPU
    inline Interval square(Interval i)
    {
        Float alow = std::abs(i.lower_bound()), ahigh = std::abs(i.upper_bound());
        if (alow > ahigh)
        {
            pstd::swap(alow, ahigh);
        }
        if (in_range(0, i))
        {
            return Interval(0, mul_round_up(ahigh, ahigh));
        }
        return Interval(mul_round_down(alow, alow), mul_round_up(ahigh, ahigh));
    }

    LOQUAT_CPU_GPU
    inline Interval mul_pow_2(Float s, Interval i);
    LOQUAT_CPU_GPU
    inline Interval mul_pow_2(Interval i, Float s);

    LOQUAT_CPU_GPU
    inline Interval operator+(Float f, Interval i)
    {
        return Interval(f) + i;
    }

    LOQUAT_CPU_GPU
    inline Interval operator-(Float f, Interval i)
    {
        return Interval(f) - i;
    }

    LOQUAT_CPU_GPU
    inline Interval operator*(Float f, Interval i)
    {
        if (f > 0)
        {
            return Interval(mul_round_down(f, i.lower_bound()), mul_round_up(f, i.upper_bound()));
        }
        else
        {
            return Interval(mul_round_down(f, i.upper_bound()), mul_round_up(f, i.lower_bound()));
        }
    }

    LOQUAT_CPU_GPU
    inline Interval operator/(Float f, Interval i)
    {
        if (in_range(0, i))
        {
            // The interval we're dividing by straddles zero, so just
            // return an interval of everything.
            return Interval(-FLOAT_INFINITY, FLOAT_INFINITY);
        }

        if (f > 0)
        {
            return Interval(div_round_down(f, i.upper_bound()), div_round_up(f, i.lower_bound()));
        }
        else
        {
            return Interval(div_round_down(f, i.lower_bound()), div_round_up(f, i.upper_bound()));
        }
    }

    LOQUAT_CPU_GPU
    inline Interval operator+(Interval i, Float f)
    {
        return i + Interval(f);
    }

    LOQUAT_CPU_GPU
    inline Interval operator-(Interval i, Float f)
    {
        return i - Interval(f);
    }

    LOQUAT_CPU_GPU
    inline Interval operator*(Interval i, Float f)
    {
        if (f > 0)
        {
            return Interval(mul_round_down(f, i.lower_bound()), mul_round_up(f, i.upper_bound()));
        }
        else
        {
            return Interval(mul_round_down(f, i.upper_bound()), mul_round_up(f, i.lower_bound()));
        }
    }

    LOQUAT_CPU_GPU
    inline Interval operator/(Interval i, Float f)
    {
        if (f == 0)
        {
            return Interval(-FLOAT_INFINITY, FLOAT_INFINITY);
        }

        if (f > 0)
        {
            return Interval(div_round_down(i.lower_bound(), f), div_round_up(i.upper_bound(), f));
        }
        else
        {
            return Interval(div_round_down(i.upper_bound(), f), div_round_up(i.lower_bound(), f));
        }
    }

    LOQUAT_CPU_GPU
    inline Float floor(Interval i)
    {
        return pstd::floor(i.lower_bound());
    }

    LOQUAT_CPU_GPU
    inline Float ceil(Interval i)
    {
        return pstd::ceil(i.upper_bound());
    }

    LOQUAT_CPU_GPU
    inline Float min(Interval a, Interval b)
    {
        return std::min(a.lower_bound(), b.lower_bound());
    }

    LOQUAT_CPU_GPU
    inline Float max(Interval a, Interval b)
    {
        return std::max(a.upper_bound(), b.upper_bound());
    }

    LOQUAT_CPU_GPU
    inline Interval sqrt(Interval i)
    {
        return { sqrt_round_down(i.lower_bound()), sqrt_round_up(i.upper_bound()) };
    }

    LOQUAT_CPU_GPU
    inline Interval FMA(Interval a, Interval b, Interval c)
    {
        Float low = std::min({
            FMA_round_down(a.lower_bound(), b.lower_bound(), c.lower_bound()),
            FMA_round_down(a.upper_bound(), b.lower_bound(), c.lower_bound()),
            FMA_round_down(a.lower_bound(), b.upper_bound(), c.lower_bound()),
            FMA_round_down(a.upper_bound(), b.upper_bound(), c.lower_bound()) 
            });
        Float high = std::max({ 
            FMA_round_up(a.lower_bound(), b.lower_bound(), c.upper_bound()),
            FMA_round_up(a.upper_bound(), b.lower_bound(), c.upper_bound()),
            FMA_round_up(a.lower_bound(), b.upper_bound(), c.upper_bound()),
            FMA_round_up(a.upper_bound(), b.upper_bound(), c.upper_bound())
            });
        return Interval(low, high);
    }

    LOQUAT_CPU_GPU
    inline Interval difference_of_products(Interval a, Interval b, Interval c,
        Interval d)
    {
        Float ab[4] = {
            a.lower_bound() * b.lower_bound(),
            a.upper_bound() * b.lower_bound(),
            a.lower_bound() * b.upper_bound(),
            a.upper_bound() * b.upper_bound() 
        };
        Float abLow = std::min({ ab[0], ab[1], ab[2], ab[3] });
        Float abHigh = std::max({ ab[0], ab[1], ab[2], ab[3] });
        int abLowIndex = abLow == ab[0] ? 0 : (abLow == ab[1] ? 1 : (abLow == ab[2] ? 2 : 3));
        int abHighIndex =
            abHigh == ab[0] ? 0 : (abHigh == ab[1] ? 1 : (abHigh == ab[2] ? 2 : 3));

        Float cd[4] = {
            c.lower_bound() * d.lower_bound(),
            c.upper_bound() * d.lower_bound(),
            c.lower_bound() * d.upper_bound(),
            c.upper_bound() * d.upper_bound()
        };
        Float cdLow = std::min({ cd[0], cd[1], cd[2], cd[3] });
        Float cdHigh = std::max({ cd[0], cd[1], cd[2], cd[3] });
        int cdLowIndex = cdLow == cd[0] ? 0 : (cdLow == cd[1] ? 1 : (cdLow == cd[2] ? 2 : 3));
        int cdHighIndex =
            cdHigh == cd[0] ? 0 : (cdHigh == cd[1] ? 1 : (cdHigh == cd[2] ? 2 : 3));

        // Invert cd Indices since it's subtracted...
        Float low = difference_of_products(a[abLowIndex & 1], b[abLowIndex >> 1],
            c[cdHighIndex & 1], d[cdHighIndex >> 1]);
        Float high = difference_of_products(a[abHighIndex & 1], b[abHighIndex >> 1],
            c[cdLowIndex & 1], d[cdLowIndex >> 1]);
        LOG_ASSERT(low <= high);

        return { next_float_down(next_float_down(low)), next_float_up(next_float_up(high)) };
    }

    LOQUAT_CPU_GPU
    inline Interval sum_of_products(Interval a, Interval b, Interval c,
        Interval d)
    {
        return difference_of_products(a, b, -c, d);
    }

    LOQUAT_CPU_GPU
    inline Interval mul_pow_2(Float s, Interval i)
    {
        return mul_pow_2(i, s);
    }

    LOQUAT_CPU_GPU
    inline Interval mul_pow_2(Interval i, Float s)
    {
        Float as = std::abs(s);
        if (as < 1)
        {
            LOG_ASSERT(1 / as == 1ull << log2_int(1 / as));
        }
        else
        {
            LOG_ASSERT(as == 1ull << log2_int(as));
        }

        // Multiplication by powers of 2 is exaact
        return Interval(std::min(i.lower_bound() * s, i.upper_bound() * s),
            std::max(i.lower_bound() * s, i.upper_bound() * s));
    }

    LOQUAT_CPU_GPU
    inline Interval abs(Interval i)
    {
        if (i.lower_bound() >= 0)
        {
            // The entire interval is greater than zero, so we're all set.
            return i;
        }
        else if (i.upper_bound() <= 0)
        {
            // The entire interval is less than zero.
            return Interval(-i.upper_bound(), -i.lower_bound());
        }
        else
        {
            // The interval straddles zero.
            return Interval(0, std::max(-i.lower_bound(), i.upper_bound()));
        }
    }

    LOQUAT_CPU_GPU
    inline Interval acos(Interval i)
    {
        Float low = std::acos(std::min<Float>(1, i.upper_bound()));
        Float high = std::acos(std::max<Float>(-1, i.lower_bound()));

        return Interval(std::max<Float>(0, next_float_down(low)), next_float_up(high));
    }

    LOQUAT_CPU_GPU
    inline Interval sin(Interval i)
    {
        LOG_ASSERT(i.lower_bound() >= -1e-16);
        LOG_ASSERT(i.upper_bound() <= 2.0001 * PI);
        Float low = std::sin(std::max<Float>(0, i.lower_bound()));
        Float high = std::sin(i.upper_bound());
        if (low > high)
        {
            pstd::swap(low, high);
        }
        low = std::max<Float>(-1, next_float_down(low));
        high = std::min<Float>(1, next_float_up(high));
        if (in_range(PI / 2, i))
        {
            high = 1;
        }
        if (in_range((3.f / 2.f) * PI, i))
        {
            low = -1;
        }

        return Interval(low, high);
    }

    LOQUAT_CPU_GPU
    inline Interval cos(Interval i)
    {
        LOG_ASSERT(i.lower_bound() >= -1e-16);
        LOG_ASSERT(i.upper_bound() <= 2.0001 * PI);
        Float low = std::cos(std::max<Float>(0, i.lower_bound()));
        Float high = std::cos(i.upper_bound());
        if (low > high)
        {
            pstd::swap(low, high);
        }
        low = std::max<Float>(-1, next_float_down(low));
        high = std::min<Float>(1, next_float_up(high));
        if (in_range(PI, i))
        {
            low = -1;
        }

        return Interval(low, high);
    }

    LOQUAT_CPU_GPU
    inline bool quadratic(Interval a, Interval b, Interval c, Interval* t0,
        Interval* t1)
    {
        // Find quadratic discriminant
        Interval discrim = difference_of_products(b, b, mul_pow_2(4, a), c);
        if (discrim.lower_bound() < 0)
            return false;
        Interval floatRootDiscrim = sqrt(discrim);

        // Compute quadratic _t_ values
        Interval q;
        if ((Float)b < 0)
        {
            q = mul_pow_2(-.5, b - floatRootDiscrim);
        }
        else
        {
            q = mul_pow_2(-.5, b + floatRootDiscrim);
        }
        *t0 = q / a;
        *t1 = c / q;
        if (t0->lower_bound() > t1->lower_bound())
        {
            pstd::swap(*t0, *t1);
        }
        return true;
    }

    LOQUAT_CPU_GPU
    inline Interval sum_squares(Interval i)
    {
        return square(i);
    }

    template <typename... Args>
    LOQUAT_CPU_GPU
    inline Interval sum_squares(Interval i, Args... args)
    {
        Interval ss = FMA(i, i, sum_squares(args...));
        return Interval(std::max<Float>(0, ss.lower_bound()), ss.upper_bound());
    }

    LOQUAT_CPU_GPU
    Vec3f equal_area_square_to_sphere(Point2f p);
    LOQUAT_CPU_GPU
    Point2f equal_area_sphere_to_square(Vec3f v);
    LOQUAT_CPU_GPU
    Point2f wrap_equal_area_square(Point2f p);

    LOQUAT_CPU_GPU
    Float catmull_rom(pstd::span<const Float> nodes, pstd::span<const Float> values, Float x);
    LOQUAT_CPU_GPU
    bool catmull_rom_weights(pstd::span<const Float> nodes, Float x, int* offset,
        pstd::span<Float> weights);
    LOQUAT_CPU_GPU
    Float integrate_catmull_rom(pstd::span<const Float> nodes, pstd::span<const Float> values,
        pstd::span<Float> cdf);
    LOQUAT_CPU_GPU
    Float invert_catmull_rom(pstd::span<const Float> x, pstd::span<const Float> values,
        Float u);

    namespace
    {
        template <int N>
        LOQUAT_CPU_GPU
        inline void init(Float m[N][N], int i, int j)
        {}

        template <int N, typename... Args>
        LOQUAT_CPU_GPU
        inline void init(Float m[N][N], int i, int j, Float v, Args... args)
        {
            m[i][j] = v;
            if (++j == N)
            {
                ++i;
                j = 0;
            }
            init<N>(m, i, j, args...);
        }

        template <int N>
        LOQUAT_CPU_GPU
        inline void init_diag(Float m[N][N], int i)
        {}

        template <int N, typename... Args>
        LOQUAT_CPU_GPU
        inline void init_diag(Float m[N][N], int i, Float v, Args... args)
        {
            m[i][i] = v;
            init_diag<N>(m, i + 1, args...);
        }
    }
    
    template <int N>
    class SquareMatrix
    {
    public:
        
        LOQUAT_CPU_GPU
        static SquareMatrix zero()
        {
            SquareMatrix m;
            for (int i = 0; i < N; ++i)
            {
                for (int j = 0; j < N; ++j)
                {
                    m.m[i][j] = 0;
                }
            }
            return m;
        }

        LOQUAT_CPU_GPU
        static constexpr SquareMatrix NaN_matrix()
        {
            SquareMatrix m;
            for (int i = 0; i < N; ++i)
            {
                for (int j = 0; j < N; ++j)
                {
                    m.m[i][j] = NaN;
                }
            }
            return m;
        }

        LOQUAT_CPU_GPU
        constexpr SquareMatrix()
        {
            for (int i = 0; i < N; ++i)
            {
                for (int j = 0; j < N; ++j)
                {
                    m[i][j] = (i == j) ? 1 : 0;
                }
            }
        }

        LOQUAT_CPU_GPU
        SquareMatrix(const Float mat[N][N])
        {
            for (int i = 0; i < N; ++i)
            {
                for (int j = 0; j < N; ++j)
                {
                    m[i][j] = mat[i][j];
                }
            }
        }

        LOQUAT_CPU_GPU
        SquareMatrix(pstd::span<const Float> t);

        template <typename... Args>
        LOQUAT_CPU_GPU
        constexpr SquareMatrix(Float v, Args... args)
        {
            static_assert(1 + sizeof...(Args) == N * N,
                "Incorrect number of values provided to SquareMatrix constructor");
            init<N>(m, 0, 0, v, args...);
        }

        template <typename... Args>
        LOQUAT_CPU_GPU
        static SquareMatrix diag(Float v, Args... args)
        {
            static_assert(1 + sizeof...(Args) == N,
                "Incorrect number of values provided to SquareMatrix::diag");
            SquareMatrix m;
            init_diag<N>(m.m, 0, v, args...);
            return m;
        }

        LOQUAT_CPU_GPU
        SquareMatrix operator+(const SquareMatrix& m) const
        {
            SquareMatrix r = *this;
            for (int i = 0; i < N; ++i)
            {
                for (int j = 0; j < N; ++j)
                {
                    r.m[i][j] += m.m[i][j];
                }
            }
            return r;
        }

        LOQUAT_CPU_GPU
        SquareMatrix operator*(Float s) const
        {
            SquareMatrix r = *this;
            for (int i = 0; i < N; ++i)
            {
                for (int j = 0; j < N; ++j)
                {
                    r.m[i][j] *= s;
                }
            }
            return r;
        }

        LOQUAT_CPU_GPU
        SquareMatrix operator/(Float s) const
        {
            LOG_ASSERT(s != 0);
            SquareMatrix r = *this;
            for (int i = 0; i < N; ++i)
            {
                for (int j = 0; j < N; ++j)
                {
                    r.m[i][j] /= s;
                }
            }
            return r;
        }

        LOQUAT_CPU_GPU
        bool operator==(const SquareMatrix<N>& m2) const
        {
            for (int i = 0; i < N; ++i)
            {
                for (int j = 0; j < N; ++j)
                {
                    if (m[i][j] != m2.m[i][j])
                    {
                        return false;
                    }
                }
            }
            return true;
        }

        LOQUAT_CPU_GPU
        bool operator!=(const SquareMatrix<N>& m2) const
        {
            for (int i = 0; i < N; ++i)
            {
                for (int j = 0; j < N; ++j)
                {
                    if (m[i][j] != m2.m[i][j])
                    {
                        return true;
                    }
                }
            }
            return false;
        }

        LOQUAT_CPU_GPU
        bool operator<(const SquareMatrix<N>& m2) const
        {
            for (int i = 0; i < N; ++i)
            {
                for (int j = 0; j < N; ++j)
                {
                    if (m[i][j] < m2.m[i][j])
                    {
                        return true;
                    }
                    if (m[i][j] > m2.m[i][j])
                    {
                        return false;
                    }
                }
            }
            return false;
        }

        LOQUAT_CPU_GPU
        bool is_identity() const;

        std::string to_string() const;

        LOQUAT_CPU_GPU
        pstd::span<const Float> operator[](int i) const
        {
            return m[i];
        }
        LOQUAT_CPU_GPU
        pstd::span<Float> operator[](int i)
        {
            return pstd::span<Float>(m[i]);
        }

    private:
        Float m[N][N];
    };
    
    template <int N>
    LOQUAT_CPU_GPU
    inline bool SquareMatrix<N>::is_identity() const
    {
        for (int i = 0; i < N; ++i)
        {
            for (int j = 0; j < N; ++j) {
                if (i == j)
                {
                    if (m[i][j] != 1)
                    {
                        return false;
                    }
                }
                else if (m[i][j] != 0)
                {
                    return false;
                }
            }
        }
        return true;
    }

    template <int N>
    LOQUAT_CPU_GPU
    inline SquareMatrix<N> operator*(Float s, const SquareMatrix<N>& m)
    {
        return m * s;
    }

    template <typename Tresult, int N, typename T>
    LOQUAT_CPU_GPU
    inline Tresult mul(const SquareMatrix<N>& m, const T& v)
    {
        Tresult result;
        for (int i = 0; i < N; ++i)
        {
            result[i] = 0;
            for (int j = 0; j < N; ++j)
            {
                result[i] += m[i][j] * v[j];
            }
        }
        return result;
    }

    template <int N>
    LOQUAT_CPU_GPU
    Float determinant(const SquareMatrix<N>& m);

    template <>
    LOQUAT_CPU_GPU inline Float determinant(const SquareMatrix<3>& m)
    {
        Float minor12 = difference_of_products(m[1][1], m[2][2], m[1][2], m[2][1]);
        Float minor02 = difference_of_products(m[1][0], m[2][2], m[1][2], m[2][0]);
        Float minor01 = difference_of_products(m[1][0], m[2][1], m[1][1], m[2][0]);
        return FMA(m[0][2], minor01,
            difference_of_products(m[0][0], minor12, m[0][1], minor02));
    }

    template <int N>
    LOQUAT_CPU_GPU
    inline SquareMatrix<N> transpose(const SquareMatrix<N>& m);
    template <int N>
    LOQUAT_CPU_GPU
    pstd::optional<SquareMatrix<N>> inverse(const SquareMatrix<N>&);

    template <int N>
    LOQUAT_CPU_GPU
    SquareMatrix<N> invert_or_exit(const SquareMatrix<N>& m)
    {
        pstd::optional<SquareMatrix<N>> inv = inverse(m);
        LOG_ASSERT(inv.has_value());
        return *inv;
    }

    template <int N>
    LOQUAT_CPU_GPU
    inline SquareMatrix<N> transpose(const SquareMatrix<N>& m)
    {
        SquareMatrix<N> r;
        for (int i = 0; i < N; ++i)
        {
            for (int j = 0; j < N; ++j)
            {
                r[i][j] = m[j][i];
            }
        }
        return r;
    }

    template <>
    LOQUAT_CPU_GPU
    inline pstd::optional<SquareMatrix<3>> inverse(const SquareMatrix<3>& m)
    {
        Float det = determinant(m);
        if (det == 0)
        {
            return {};
        }
        Float invDet = 1 / det;

        SquareMatrix<3> r;

        r[0][0] = invDet * difference_of_products(m[1][1], m[2][2], m[1][2], m[2][1]);
        r[1][0] = invDet * difference_of_products(m[1][2], m[2][0], m[1][0], m[2][2]);
        r[2][0] = invDet * difference_of_products(m[1][0], m[2][1], m[1][1], m[2][0]);
        r[0][1] = invDet * difference_of_products(m[0][2], m[2][1], m[0][1], m[2][2]);
        r[1][1] = invDet * difference_of_products(m[0][0], m[2][2], m[0][2], m[2][0]);
        r[2][1] = invDet * difference_of_products(m[0][1], m[2][0], m[0][0], m[2][1]);
        r[0][2] = invDet * difference_of_products(m[0][1], m[1][2], m[0][2], m[1][1]);
        r[1][2] = invDet * difference_of_products(m[0][2], m[1][0], m[0][0], m[1][2]);
        r[2][2] = invDet * difference_of_products(m[0][0], m[1][1], m[0][1], m[1][0]);

        return r;
    }

    template <int N, typename T>
    LOQUAT_CPU_GPU
    inline T operator*(const SquareMatrix<N>& m, const T& v)
    {
        return mul<T>(m, v);
    }

    template <>
    LOQUAT_CPU_GPU
    inline SquareMatrix<4> operator*(const SquareMatrix<4>& m1,
        const SquareMatrix<4>& m2)
    {
        SquareMatrix<4> r;
        for (int i = 0; i < 4; ++i)
        {
            for (int j = 0; j < 4; ++j)
            {
                r[i][j] = inner_product(
                    m1[i][0], m2[0][j],
                    m1[i][1], m2[1][j],
                    m1[i][2], m2[2][j],
                    m1[i][3], m2[3][j]);
            }
        }
        return r;
    }

    template <>
    LOQUAT_CPU_GPU
    inline SquareMatrix<3> operator*(const SquareMatrix<3>& m1,
        const SquareMatrix<3>& m2)
    {
        SquareMatrix<3> r;
        for (int i = 0; i < 3; ++i)
        {
            for (int j = 0; j < 3; ++j)
            {
                r[i][j] =
                    inner_product(
                        m1[i][0], m2[0][j],
                        m1[i][1], m2[1][j],
                        m1[i][2], m2[2][j]);
            }
            return r;
        }
    }

    template <int N>
    LOQUAT_CPU_GPU
    inline SquareMatrix<N> operator*(const SquareMatrix<N>& m1,
        const SquareMatrix<N>& m2)
    {
        SquareMatrix<N> r;
        for (int i = 0; i < N; ++i)
        {
            for (int j = 0; j < N; ++j)
            {
                r[i][j] = 0;
                for (int k = 0; k < N; ++k)
                {
                    r[i][j] = FMA(m1[i][k], m2[k][j], r[i][j]);
                }
            }
        }
        return r;
    }

    template <int N>
    LOQUAT_CPU_GPU
    inline SquareMatrix<N>::SquareMatrix(pstd::span<const Float> t)
    {
        LOG_ASSERT(N * N == t.size());
        for (int i = 0; i < N * N; ++i)
        {
            m[i / N][i % N] = t[i];
        }
    }

    template <int N>
    LOQUAT_CPU_GPU
    SquareMatrix<N> operator*(const SquareMatrix<N>& m1,
        const SquareMatrix<N>& m2);

    template <>
    LOQUAT_CPU_GPU
    inline Float determinant(const SquareMatrix<1>& m)
    {
        return m[0][0];
    }

    template <>
    LOQUAT_CPU_GPU
    inline Float determinant(const SquareMatrix<2>& m)
    {
        return difference_of_products(m[0][0], m[1][1], m[0][1], m[1][0]);
    }

    template <>
    LOQUAT_CPU_GPU
    inline Float determinant(const SquareMatrix<4>& m)
    {
        Float s0 = difference_of_products(m[0][0], m[1][1], m[1][0], m[0][1]);
        Float s1 = difference_of_products(m[0][0], m[1][2], m[1][0], m[0][2]);
        Float s2 = difference_of_products(m[0][0], m[1][3], m[1][0], m[0][3]);

        Float s3 = difference_of_products(m[0][1], m[1][2], m[1][1], m[0][2]);
        Float s4 = difference_of_products(m[0][1], m[1][3], m[1][1], m[0][3]);
        Float s5 = difference_of_products(m[0][2], m[1][3], m[1][2], m[0][3]);

        Float c0 = difference_of_products(m[2][0], m[3][1], m[3][0], m[2][1]);
        Float c1 = difference_of_products(m[2][0], m[3][2], m[3][0], m[2][2]);
        Float c2 = difference_of_products(m[2][0], m[3][3], m[3][0], m[2][3]);

        Float c3 = difference_of_products(m[2][1], m[3][2], m[3][1], m[2][2]);
        Float c4 = difference_of_products(m[2][1], m[3][3], m[3][1], m[2][3]);
        Float c5 = difference_of_products(m[2][2], m[3][3], m[3][2], m[2][3]);

        return (difference_of_products(s0, c5, s1, c4) + difference_of_products(s2, c3, -s3, c2) +
            difference_of_products(s5, c0, s4, c1));
    }

    template <int N>
    LOQUAT_CPU_GPU
    inline Float determinant(const SquareMatrix<N>& m)
    {
        SquareMatrix<N - 1> sub;
        Float det = 0;
        // Inefficient, but we don't currently use N>4 anyway..
        for (int i = 0; i < N; ++i)
        {
            // Sub-matrix without row 0 and column i
            for (int j = 0; j < N - 1; ++j)
            {
                for (int k = 0; k < N - 1; ++k)
                {
                    sub[j][k] = m[j + 1][k < i ? k : k + 1];
                }
            }

            Float sign = (i & 1) ? -1 : 1;
            det += sign * m[0][i] * determinant(sub);
        }
        return det;
    }

    template <>
    LOQUAT_CPU_GPU
    inline pstd::optional<SquareMatrix<4>> inverse(const SquareMatrix<4>& m)
    {
        // Via: https://github.com/google/ion/blob/master/ion/math/matrixutils.cc,
        // (c) Google, Apache license.

        // For 4x4 do not compute the adjugate as the transpose of the cofactor
        // matrix, because this results in extra work. Several calculations can be
        // shared across the sub-determinants.
        //
        // This approach is explained in David Eberly's Geometric Tools book,
        // excerpted here:
        //   http://www.geometrictools.com/Documentation/LaplaceExpansionTheorem.pdf
        Float s0 = difference_of_products(m[0][0], m[1][1], m[1][0], m[0][1]);
        Float s1 = difference_of_products(m[0][0], m[1][2], m[1][0], m[0][2]);
        Float s2 = difference_of_products(m[0][0], m[1][3], m[1][0], m[0][3]);

        Float s3 = difference_of_products(m[0][1], m[1][2], m[1][1], m[0][2]);
        Float s4 = difference_of_products(m[0][1], m[1][3], m[1][1], m[0][3]);
        Float s5 = difference_of_products(m[0][2], m[1][3], m[1][2], m[0][3]);

        Float c0 = difference_of_products(m[2][0], m[3][1], m[3][0], m[2][1]);
        Float c1 = difference_of_products(m[2][0], m[3][2], m[3][0], m[2][2]);
        Float c2 = difference_of_products(m[2][0], m[3][3], m[3][0], m[2][3]);

        Float c3 = difference_of_products(m[2][1], m[3][2], m[3][1], m[2][2]);
        Float c4 = difference_of_products(m[2][1], m[3][3], m[3][1], m[2][3]);
        Float c5 = difference_of_products(m[2][2], m[3][3], m[3][2], m[2][3]);

        Float determinant = inner_product(s0, c5, -s1, c4, s2, c3, s3, c2, s5, c0, -s4, c1);
        if (determinant == 0)
            return {};
        Float s = 1 / determinant;

        Float inv[4][4] = { {s * inner_product(m[1][1], c5, m[1][3], c3, -m[1][2], c4),
                            s * inner_product(-m[0][1], c5, m[0][2], c4, -m[0][3], c3),
                            s * inner_product(m[3][1], s5, m[3][3], s3, -m[3][2], s4),
                            s * inner_product(-m[2][1], s5, m[2][2], s4, -m[2][3], s3)},

                           {s * inner_product(-m[1][0], c5, m[1][2], c2, -m[1][3], c1),
                            s * inner_product(m[0][0], c5, m[0][3], c1, -m[0][2], c2),
                            s * inner_product(-m[3][0], s5, m[3][2], s2, -m[3][3], s1),
                            s * inner_product(m[2][0], s5, m[2][3], s1, -m[2][2], s2)},

                           {s * inner_product(m[1][0], c4, m[1][3], c0, -m[1][1], c2),
                            s * inner_product(-m[0][0], c4, m[0][1], c2, -m[0][3], c0),
                            s * inner_product(m[3][0], s4, m[3][3], s0, -m[3][1], s2),
                            s * inner_product(-m[2][0], s4, m[2][1], s2, -m[2][3], s0)},

                           {s * inner_product(-m[1][0], c3, m[1][1], c1, -m[1][2], c0),
                            s * inner_product(m[0][0], c3, m[0][2], c0, -m[0][1], c1),
                            s * inner_product(-m[3][0], s3, m[3][1], s1, -m[3][2], s0),
                            s * inner_product(m[2][0], s3, m[2][2], s0, -m[2][1], s1)} };

        return SquareMatrix<4>(inv);
    }

    extern template class SquareMatrix<2>;
    extern template class SquareMatrix<3>;
    extern template class SquareMatrix<4>;

    /// <summary>
    /// A 3x3 matrix of Floats.
    /// </summary>
    using Mat3 = SquareMatrix<3>;

    /// <summary>
    /// A 4x4 matrix of Floats.
    /// </summary>
    using Mat4 = SquareMatrix<4>;

    /// <summary>
    /// A Mat4 full of NaNs.
    /// </summary>
    constexpr Mat4 Mat4_NaN = SquareMatrix<4>::NaN_matrix();
}
