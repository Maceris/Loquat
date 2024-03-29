// pbrt is Copyright(c) 1998-2020 Matt Pharr, Wenzel Jakob, and Greg Humphreys.
// The pbrt source code is licensed under the Apache License, Version 2.0.
// SPDX: Apache-2.0

// This file has been modified from the original, original notice is above.

#pragma once

#include <algorithm>
#include <memory>
#include <string>

#include "main/loquat.h"
#include "pbr/math/hash.h"
#include "pbr/math/primes.h"
#include "pbr/math/sobol_matrices.h"
#include "pbr/math/vector_math.h"

namespace loquat
{

    class DigitPermutation
    {
    public:
        DigitPermutation() = default;
        DigitPermutation(int base, int32_t seed, Allocator allocator) noexcept
            : base{ base }
        {
            LOG_ASSERT(base < 65535 && "Base too large");
            digit_count = 0;
            Float inverse_base = (Float)1 / (Float) base;
            Float inverse_base_m = 1;
            while (1 - (base - 1) * inverse_base_m < 1)
            {
                ++digit_count;
                inverse_base_m *= inverse_base;
            }

            permutations = allocator.allocate_object<uint16_t>(
                digit_count * base);
            for (int digit_index = 0; digit_index < digit_count; ++digit_index)
            {
                uint64_t digit_seed = hash(base, digit_index, seed);
                for (int digit_value = 0; digit_value < base; ++digit_value)
                {
                    int index = digit_index * base + digit_value;
                    permutations[index] = permutation_element(digit_value,
                        base, digit_seed);
                }
            }
        }
        
        int permute(int digit_index, int digit_value) const noexcept
        {
            LOG_ASSERT(digit_index < digit_count && "Invalid digit index");
            LOG_ASSERT(digit_value < base && "Invalid digit value");
            return permutations[digit_index * base + digit_value];
        }
        
        [[nodiscard]]
        std::string to_string() const noexcept;
        
    private:
        int base;
        int digit_count;
        uint16_t* permutations;
    };

    inline uint64_t sobol_interval_to_index(int32_t log2_scale,
        uint64_t sample_index, Point2i point) noexcept;

    inline Float blue_noise_sample(Point2i point, int instance);

    Float scrambled_radical_inverse(int base_index, uint64_t a,
        const DigitPermutation& permutation) noexcept;

    struct NoRandomizer
    {
        uint32_t operator()(uint32_t v) const noexcept
        {
            return v;
        }
    };

    template <typename R>
    concept randomizer = requires(R r, uint32_t i)
    {
        { r(i) } -> std::convertible_to<uint32_t>;
    };

    inline Float radical_inverse(int base_index, uint64_t a) noexcept
    {
        unsigned int base = primes[base_index];
        
        uint64_t limit = ~0ull / base - base;
        Float inverse_base = (Float) 1 / (Float) base;
        Float inverse_base_m = 1;
        uint64_t reversed_digits = 0;
        while (a && reversed_digits < limit)
        {
            uint64_t next = a / base;
            uint64_t digit = a - next * base;
            reversed_digits = reversed_digits * base + digit;
            inverse_base_m *= inverse_base;
            a = next;
        }
        return std::min(reversed_digits * inverse_base_m, ONE_MINUS_EPSILON);
    }

    inline uint64_t inverse_radical_inverse(uint64_t inverse, int base,
        int digit_count) noexcept
    {
        uint64_t index = 0;
        for (int i = 0; i < digit_count; ++i)
        {
            uint64_t digit = inverse % base;
            inverse /= base;
            index = index * base + digit;
        }
        return index;
    }

    inline Float scrambled_radical_inverse(int base_index, uint64_t a,
        const DigitPermutation& permutation) noexcept
    {
        unsigned int base = primes[base_index];
        
        uint64_t limit = ~0ull / base - base;
        Float inverse_base = (Float) 1 / (Float) base;
        Float inverse_base_m = 1;
        uint64_t reversed_digits = 0;
        int digit_index = 0;
        while (1 - (base - 1) * inverse_base_m < 1 && reversed_digits < limit)
        {
            uint64_t next = a / base;
            int digit_value = a - next * base;
            reversed_digits = reversed_digits * base
                + permutation.permute(digit_index, digit_value);
            inverse_base_m *= inverse_base;
            ++digit_index;
            a = next;
        }
        return std::min(inverse_base_m * reversed_digits, ONE_MINUS_EPSILON);
    }

    inline Float owen_scrambled_radical_inverse(int base_index, uint64_t a,
        uint32_t hash) noexcept
    {
        unsigned int base = primes[base_index];
        
        uint64_t limit = ~0ull / base - base;
        Float inverse_base = (Float) 1 / (Float) base;
        Float inverse_base_m = 1;
        uint64_t reversed_digits = 0;
        int digit_index = 0;
        while (1 - inverse_base_m < 1 && reversed_digits < limit)
        {
            uint64_t next = a / base;
            int digit_value = a - next * base;
            uint32_t digit_hash = mix_bits(hash ^ reversed_digits);
            digit_value = permutation_element(digit_value, base, digit_hash);
            reversed_digits = reversed_digits * base + digit_value;
            inverse_base_m *= inverse_base;
            ++digit_index;
            a = next;
        }
        return std::min(inverse_base_m * reversed_digits, ONE_MINUS_EPSILON);
    }

    inline uint32_t multiply_generator(std::span<const uint32_t> C,
        uint32_t a) noexcept
    {
        uint32_t result = a;
        for (int i = 0 ; a != 0; ++i, a >>= 1)
        {
            if (a & 1)
            {
                result ^= C[i];
            }
        }
        return result;
    }

    template <randomizer R>
    inline Float sobol_sample(int64_t a, int dimension, R randomizer) noexcept
    {
        LOG_ASSERT(dimension < SOBOL_DIMENSIONS);
        LOG_ASSERT(a >= 0 && a < (1ull << SOBOL_MATRIX_SIZE));

        uint32_t v = 0;
        for (int i = dimension * SOBOL_MATRIX_SIZE; a != 0; a >>= 1, ++i)
        {
            if (a & 1)
            {
                v ^= sobol_matrices_32[i];
            }
        }
        v = randomizer(v);
        return std::min(v * 0x1p-32f, ONE_MINUS_EPSILON_FLOAT);
    }

    inline Float blue_noise_sample(Point2i point, int instance) noexcept
    {
        auto hash_permutation = [&](uint64_t index) -> int
            {
                return uint32_t(
                    mix_bits(index ^ (0x55555555 * instance)) >> 24
                ) % 24;
            };
        
        const int base_4_digit_count = 8;
        point.x &= 255;
        point.y &= 255;
        uint64_t morton_index = encode_morton_2(point.x, point.y);

        static const uint8_t permutations[24][4] = {
            {0, 1, 2, 3}, {0, 1, 3, 2}, {0, 2, 1, 3}, {0, 2, 3, 1},
            {0, 3, 2, 1}, {0, 3, 1, 2}, {1, 0, 2, 3}, {1, 0, 3, 2},
            {1, 2, 0, 3}, {1, 2, 3, 0}, {1, 3, 2, 0}, {1, 3, 0, 2},
            {2, 1, 0, 3}, {2, 1, 3, 0}, {2, 0, 1, 3}, {2, 0, 3, 1},
            {2, 3, 0, 1}, {2, 3, 1, 0}, {3, 1, 2, 0}, {3, 1, 0, 2},
            {3, 2, 1, 0}, {3, 2, 0, 1}, {3, 0, 2, 1}, {3, 0, 1, 2}
        };

        uint32_t sample_index = 0;

        for (int i = base_4_digit_count - 1; i >= 0; --i)
        {
            int digit_shift = 2 * i;
            int digit = (morton_index >> digit_shift) & 3;
            int p = hash_permutation(morton_index >> (digit_shift + 2));
            digit = permutations[p][digit];
            sample_index |= digit << digit_shift;
        }

        return reverse_bits_32(sample_index) * 0x1p-32f;
    }

    struct BinaryPermuteScrambler
    {
        BinaryPermuteScrambler(uint32_t permutation) noexcept
            : permutation{ permutation }
        {}
        uint32_t operator()(uint32_t v) const noexcept
        {
            return permutation ^ v;
        }
        uint32_t permutation;
    };

    struct FastOwenScrambler
    {
        FastOwenScrambler(uint32_t seed) noexcept
            : seed{ seed }
        {}

        uint32_t operator()(uint32_t v) const noexcept
        {
            v = reverse_bits_32(v);
            v ^= v * 0x3d20adea;
            v += seed;
            v *= (seed >> 16) | 1;
            v ^= v * 0x05526c56;
            v ^= v * 0x53a22864;
            return reverse_bits_32(v);
        }

        uint32_t seed;
    };

    struct OwenScrambler
    {
        OwenScrambler(uint32_t seed) noexcept
            : seed{ seed }
        {}

        uint32_t operator()(uint32_t v) const noexcept
        {
            if (seed & 1)
            {
                v ^= 1u << 31;
            }
            for (int b = 1; b < 32; ++b)
            {
                uint32_t mask = (~0u) << (32 - b);
                if ((uint32_t)mix_bits((v & mask) ^ seed) & (1u << b))
                {
                    v ^= 1u << (31 - b);
                }
            }
            return v;
        }

        uint32_t seed;
    };

    enum class RandomizeStrategy
    {
        None,
        PermuteDigits,
        FastOwen,
        Owen
    };

    [[nodiscard]]
    std::string to_string(RandomizeStrategy r) noexcept;

    inline uint64_t sobol_interval_to_index(uint32_t m, uint64_t frame,
        Point2i point) noexcept
    {
        if (m == 0)
        {
            return frame;
        }

        const uint32_t m2 = m << 1;
        uint64_t index = uint64_t(frame) << m2;

        uint64_t delta = 0;
        for (int c = 0; frame; frame >>= 1, ++c)
        {
            //NOTE(ches) For flipped column m + c + 1
            if (frame & 1)
            {
                delta ^= transformed_sobol_matrices[m - 1][c];
            }
        }

        uint64_t b = (((uint64_t)((uint32_t)point.x) << m) 
            | ((uint32_t)point.y)) ^ delta;

        for (int c = 0; b; b >>= 1, ++c)
        {
            //NOTE(ches) for column 2 * m - c
            if (b & 1)
            {
                index ^= inverse_transformed_sobol_matrices[m - 1][c];
            }
        }
        return index;
    }

}
