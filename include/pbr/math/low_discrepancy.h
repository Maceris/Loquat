// pbrt is Copyright(c) 1998-2020 Matt Pharr, Wenzel Jakob, and Greg Humphreys.
// The pbrt source code is licensed under the Apache License, Version 2.0.
// SPDX: Apache-2.0

// This file has been modified from the original, original notice is above.

#pragma once

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


}
