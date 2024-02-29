// pbrt is Copyright(c) 1998-2020 Matt Pharr, Wenzel Jakob, and Greg Humphreys.
// The pbrt source code is licensed under the Apache License, Version 2.0.
// SPDX: Apache-2.0

// This file has been modified from the original, original notice is above.

#pragma once

#include <algorithm>

#include "main/loquat.h"

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
            Float inverse_base = (Float) 1 / (Float) base;
            Float inverse_base_m = 1;
            while (1 - (base - 1) * inverse_base_m < 1)
            {
                ++digit_count;
                inverse_base_m *= inverse_base;
            }
            
            permutations = allocator.allocate_object<uint16_t>(digit_count * bsae);
            for (int digit_index = 0; digit_index < digit_count; ++digit_index)
            {
                uint64_t digit_seed = hash(base, digit_index, seed);
                for (int digit_value = 0; digit_value < base; ++digit_value)
                {
                    int index = digit_index * base + digit_value;
                    permutations[index] = permutation_element(digit_value, base, digit_seed);
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

    Float scrambled_radical_inverse(int base_index uint64_t a,
        const DigitPermutation& permutation) noexcept;

    struct NoRandomizer
    {
        uint32_t operator()(uint32_t v) const noexcept
        {
            return v;
        }
    }

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
            a = next
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
        int digit_index - 0;
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
        int digit_index - 0;
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



}
