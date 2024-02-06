// pbrt is Copyright(c) 1998-2020 Matt Pharr, Wenzel Jakob, and Greg Humphreys.
// The pbrt source code is licensed under the Apache License, Version 2.0.
// SPDX: Apache-2.0

// This file has been modified from the original, original notice is above.

#pragma once

#include "main/loquat.h"

#include <string.h>
#include <cstdint>
#include <cstring>

namespace loquat
{

    /// <summary>
    /// https://github.com/explosion/murmurhash/blob/master/murmurhash/MurmurHash2.cpp
    /// </summary>
    /// <param name="key">The item to hash.</param>
    /// <param name="len">The length of the key.</param>
    /// <param name="seed">The hash seed.</param>
    /// <returns></returns>
    inline uint64_t murmur_hash_64A(const unsigned char* key, size_t len,
        uint64_t seed) noexcept
    {
        const uint64_t m = 0xc6a4a7935bd1e995ull;
        const int r = 47;

        uint64_t h = seed ^ (len * m);

        const unsigned char* end = key + 8 * (len / 8);

        while (key != end)
        {
            uint64_t k;
            std::memcpy(&k, key, sizeof(uint64_t));
            key += 8;

            k *= m;
            k ^= k >> r;
            k *= m;

            h ^= k;
            h *= m;
        }

        switch (len & 7)
        {
        case 7:
            h ^= uint64_t(key[6]) << 48;
            [[fallthrough]];
        case 6:
            h ^= uint64_t(key[5]) << 40;
            [[fallthrough]];
        case 5:
            h ^= uint64_t(key[4]) << 32;
            [[fallthrough]];
        case 4:
            h ^= uint64_t(key[3]) << 24;
            [[fallthrough]];
        case 3:
            h ^= uint64_t(key[2]) << 16;
            [[fallthrough]];
        case 2:
            h ^= uint64_t(key[1]) << 8;
            [[fallthrough]];
        case 1:
            h ^= uint64_t(key[0]);
            h *= m;
        };

        h ^= h >> r;
        h *= m;
        h ^= h >> r;

        return h;
    }

    /// <summary>
    /// Hashing Inline Functions
    /// http://zimbry.blogspot.ch/2011/09/better-bit-mixing-improving-on.html
    /// </summary>
    /// <param name="v">The bits to mix.</param>
    /// <returns>The bits, after mixing.</returns>
    inline uint64_t mix_bits(uint64_t v) noexcept
    {
        v ^= (v >> 31);
        v *= 0x7fb5d329728ea185;
        v ^= (v >> 27);
        v *= 0x81dadef4bc2dd44d;
        v ^= (v >> 33);
        return v;
    }

    template <typename T>
    inline uint64_t hash_buffer(const T* ptr, size_t size, uint64_t seed = 0)
        noexcept
    {
        return murmur_hash_64A((const unsigned char*)ptr, size, seed);
    }

    template <typename... Args>
    inline uint64_t hash(Args... args) noexcept;

    template <typename... Args>
    inline void hash_recursive_copy(char* buf, Args...) noexcept;

    template <>
    inline void hash_recursive_copy(char* buf) noexcept {}

    template <typename T, typename... Args>
    inline void hash_recursive_copy(char* buf, T v, Args... args) noexcept
    {
        memcpy(buf, &v, sizeof(T));
        hash_recursive_copy(buf + sizeof(T), args...);
    }

    template <typename... Args>
    inline uint64_t hash(Args... args) noexcept
    {
        constexpr size_t sz = (sizeof(Args) + ... + 0);
        constexpr size_t n = (sz + 7) / 8;
        uint64_t buf[n];
        hash_recursive_copy((char*)buf, args...);
        return murmur_hash_64A((const unsigned char*)buf, sz, 0);
    }

    template <typename... Args>
    inline Float hash_float(Args... args) noexcept
    {
        return uint32_t(hash(args...)) * 0x1p-32f;
    }

}