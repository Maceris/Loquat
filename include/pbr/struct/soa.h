// pbrt is Copyright(c) 1998-2020 Matt Pharr, Wenzel Jakob, and Greg Humphreys.
// The pbrt source code is licensed under the Apache License, Version 2.0.
// SPDX: Apache-2.0

// This file has been modified from the original, original notice is above.

#pragma once

#include "main/loquat.h"

#include "pbr/base/bssrdf.h"
#include "pbr/base/material.h"
#include "pbr/base/medium.h"
#include "pbr/bsdf.h"
#include "pbr/math/math.h"
#include "pbr/math/ray.h"
#include "pbr/math/vector_math.h"
#include "pbr/struct/interaction.h"
#include "pbr/util/pstd.h"
#include "pbr/util/spectrum.h"

namespace loquat
{
	struct alignas(16) Float4
	{
		Float v[4];
	};

    LOQUAT_CPU_GPU
    inline Float4 load_4(const Float4* p)
    {
#if defined(LOQUAT_IS_GPU_CODE) && !defined(LOQUAT_FLOAT_AS_DOUBLE)
        float4 v = *(const float4*)p;
        return { {v.x, v.y, v.z, v.w} };
#else
        return *p;
#endif
    }

    LOQUAT_CPU_GPU
    inline void store_4(Float4* p, Float4 v)
    {
#if defined(LOQUAT_IS_GPU_CODE) && !defined(LOQUAT_FLOAT_AS_DOUBLE)
        *(float4*)p = make_float4(v.v[0], v.v[1], v.v[2], v.v[3]);
#else
        *p = v;
#endif
    }

    template <>
    struct SOA<SampledSpectrum>
    {
        SOA() = default;
        SOA(int size, Allocator alloc)
        {
            if constexpr ((SPECTRUM_SAMPLE_COUNT % 4) == 0)
            {
                allocation_count = n4 * size;
                ptr4 = alloc.allocate_object<Float4>(allocation_count);
            }
            else
            {
                allocation_count = size * SPECTRUM_SAMPLE_COUNT;
                ptr1 = alloc.allocate_object<Float>(allocation_count);
            }
        }

        SOA& operator=(const SOA& s)
        {
            allocation_count = s.allocation_count;
            ptr4 = s.ptr4;
            ptr1 = s.ptr1;
            return *this;
        }

        LOQUAT_CPU_GPU
        SampledSpectrum operator[](int i) const
        {
            SampledSpectrum s;
            if constexpr ((SPECTRUM_SAMPLE_COUNT % 4) == 0)
            {
                int offset = n4 * i;
                LOG_ASSERT(offset < allocation_count);
                for (int i = 0; i < n4; ++i, ++offset)
                {
                    Float4 v4 = load_4(ptr4 + offset);
                    for (int j = 0; j < 4; ++j)
                    {
                        s[4 * i + j] = v4.v[j];
                    }
                }
            }
            else
            {
                int offset = i * SPECTRUM_SAMPLE_COUNT;
                LOG_ASSERT(offset < allocation_count);
                for (int i = 0; i < SPECTRUM_SAMPLE_COUNT; ++i)
                {
                    s[i] = ptr1[offset + i];
                }
            }
            return s;
        }

        struct GetSetIndirector
        {
            LOQUAT_CPU_GPU
            operator SampledSpectrum() const
            {
                // (*(const SOA<SampledSpectrum> *)soa)[index];
                return soa->load(index); 
            }

            LOQUAT_CPU_GPU
            void operator=(const SampledSpectrum& s)
            {
                if constexpr ((SPECTRUM_SAMPLE_COUNT % 4) == 0)
                {
                    int offset = n4 * index;
                    LOG_ASSERT(offset < soa->allocation_count);
                    for (int i = 0; i < n4; ++i, ++offset)
                    {
                        store_4(soa->ptr4 + offset, {
                            s[4 * i],
                            s[4 * i + 1],
                            s[4 * i + 2],
                            s[4 * i + 3]
                        });
                    }
                }
                else
                {
                    int offset = index * SPECTRUM_SAMPLE_COUNT;
                    LOG_ASSERT(offset < soa->allocation_count);
                    for (int i = 0; i < SPECTRUM_SAMPLE_COUNT; ++i)
                    {
                        soa->ptr1[offset + i] = s[i];
                    }
                }
            }

            static constexpr int n4 = (SPECTRUM_SAMPLE_COUNT + 3) / 4;
            SOA<SampledSpectrum>* soa;
            int index;
        };

        LOQUAT_CPU_GPU
        GetSetIndirector operator[](int i)
        {
            return GetSetIndirector{ this, i };
        }

        // TODO(ches) get rid of these
        LOQUAT_CPU_GPU
        SampledSpectrum load(int i) const
        {
            return (*this)[i];
        }

        LOQUAT_CPU_GPU
        void store(int i, const SampledSpectrum& s)
        {
            (*this)[i] = s;
        }

    private:
        static constexpr int n4 = (SPECTRUM_SAMPLE_COUNT + 3) / 4;

        int allocation_count;
        Float4* LOQUAT_RESTRICT ptr4 = nullptr;
        Float* LOQUAT_RESTRICT ptr1 = nullptr;
    };

    template <>
    struct SOA<SampledWavelengths>
    {
        SOA() = default;
        SOA(int size, Allocator alloc)
        {
            if constexpr ((SPECTRUM_SAMPLE_COUNT % 4) == 0)
            {
                allocation_count = n4 * size;
                lambda4 = alloc.allocate_object<Float4>(allocation_count);
                pdf4 = alloc.allocate_object<Float4>(allocation_count);
            }
            else
            {
                allocation_count = size * SPECTRUM_SAMPLE_COUNT;
                lambda1 = alloc.allocate_object<Float>(allocation_count);
                pdf1 = alloc.allocate_object<Float>(allocation_count);
            }
        }

        SOA& operator=(const SOA& s)
        {
            allocation_count = s.allocation_count;
            lambda4 = s.lambda4;
            pdf4 = s.pdf4;
            lambda1 = s.lambda1;
            pdf1 = s.pdf1;
            return *this;
        }

        LOQUAT_CPU_GPU
        SampledWavelengths operator[](int i) const
        {
            SampledWavelengths l;
            if constexpr ((SPECTRUM_SAMPLE_COUNT % 4) == 0)
            {
                int offset = n4 * i;
                for (int i = 0; i < n4; ++i, ++offset)
                {
                    LOG_ASSERT(offset < allocation_count);
                    Float4 l4 = load_4(lambda4 + offset);
                    Float4 p4 = load_4(pdf4 + offset);
                    for (int j = 0; j < 4; ++j)
                    {
                        l.wavelengths[4 * i + j] = l4.v[j];
                        l.pdf[4 * i + j] = p4.v[j];
                    }
                }
            }
            else {
                int offset = SPECTRUM_SAMPLE_COUNT * i;
                for (int i = 0; i < SPECTRUM_SAMPLE_COUNT; ++i)
                {
                    l.wavelengths[i] = lambda1[offset + i];
                }
                for (int i = 0; i < SPECTRUM_SAMPLE_COUNT; ++i)
                {
                    l.pdf[i] = pdf1[offset + i];
                }
            }
            return l;
        }

        struct GetSetIndirector
        {
            LOQUAT_CPU_GPU
            operator SampledWavelengths() const
            {
                // (*(const SOA<SampledWavelengths> *)soa)[index];
                return soa->load(index);
            }

            LOQUAT_CPU_GPU
            void operator=(const SampledWavelengths& s)
            {
                if constexpr ((SPECTRUM_SAMPLE_COUNT % 4) == 0)
                {
                    int offset = n4 * index;
                    for (int i = 0; i < n4; ++i, ++offset)
                    {
                        store_4(soa->lambda4 + offset, {
                            s.wavelengths[4 * i],
                            s.wavelengths[4 * i + 1],
                            s.wavelengths[4 * i + 2],
                            s.wavelengths[4 * i + 3]
                        });
                        store_4(soa->pdf4 + offset, {
                            s.pdf[4 * i],
                            s.pdf[4 * i + 1],
                            s.pdf[4 * i + 2],
                            s.pdf[4 * i + 3]
                        });
                    }
                }
                else
                {
                    int offset = index * SPECTRUM_SAMPLE_COUNT;
                    for (int i = 0; i < SPECTRUM_SAMPLE_COUNT; ++i)
                    {
                        soa->lambda1[offset + i] = s.wavelengths[i];
                    }
                    for (int i = 0; i < SPECTRUM_SAMPLE_COUNT; ++i)
                    {
                        soa->pdf1[offset + i] = s.pdf[i];
                    }
                }
            }

            static constexpr int n4 = (SPECTRUM_SAMPLE_COUNT + 3) / 4;
            SOA<SampledWavelengths>* soa;
            int index;
        };
        LOQUAT_CPU_GPU
        GetSetIndirector operator[](int i)
        {
            return GetSetIndirector{ this, i };
        }

        LOQUAT_CPU_GPU
        SampledWavelengths load(int i) const
        {
            return (*this)[i];
        }

        LOQUAT_CPU_GPU
        void store(int i, const SampledWavelengths& wl)
        {
            (*this)[i] = wl;
        }

    private:
        static constexpr int n4 = (SPECTRUM_SAMPLE_COUNT + 3) / 4;

        int allocation_count;
        Float4* LOQUAT_RESTRICT lambda4 = nullptr;
        Float4* LOQUAT_RESTRICT pdf4 = nullptr;
        Float* LOQUAT_RESTRICT lambda1 = nullptr;
        Float* LOQUAT_RESTRICT pdf1 = nullptr;

    };

    //NOTE(ches) Try running soac if this is giving errors
    #include "pbrt_soa.h"
}
