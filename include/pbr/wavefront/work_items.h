// pbrt is Copyright(c) 1998-2020 Matt Pharr, Wenzel Jakob, and Greg Humphreys.
// The pbrt source code is licensed under the Apache License, Version 2.0.
// SPDX: Apache-2.0

// This file has been modified from the original, original notice is above.

#pragma once

//TODO(ches) fill this out

#include "main/loquat.h"

#include "pbr/films.h"
#include "pbr/light_samplers.h"
#include "pbr/materials.h"
#include "pbr/base/sampler.h"
#include "pbr/math/ray.h"
#include "pbr/struct/containers.h"
#include "pbr/struct/soa.h"
#include "pbr/util/pstd.h"
#include "pbr/wavefront/work_queue.h"

namespace loquat
{
    struct RaySamples
    {
        struct {
            Point2f u;
            Float uc;
        } direct;
        struct {
            Float uc, rr;
            Point2f u;
        } indirect;
        bool have_subsurface;
        struct {
            Float uc;
            Point2f u;
        } subsurface;
    };

    template <>
    struct SOA<RaySamples>
    {
      public:
        SOA() = default;

        SOA(int size, Allocator alloc)
        {
            direct = alloc.allocate_object<Float4>(size);
            indirect = alloc.allocate_object<Float4>(size);
            subsurface = alloc.allocate_object<Float4>(size);
            media_dist = alloc.allocate_object<Float>(size);
            media_mode = alloc.allocate_object<Float>(size);
        }

        LOQUAT_CPU_GPU
        RaySamples operator[](int i) const
        {
            RaySamples rs;
            Float4 dir = load_4(direct + i);
            rs.direct.u = Point2f(dir.v[0], dir.v[1]);
            rs.direct.uc = dir.v[2];

            rs.have_subsurface = int(dir.v[3]) & 1;

            Float4 ind = load_4(indirect + i);
            rs.indirect.uc = ind.v[0];
            rs.indirect.rr = ind.v[1];
            rs.indirect.u = Point2f(ind.v[2], ind.v[3]);

            if (rs.have_subsurface) {
                Float4 ss = load_4(subsurface + i);
                rs.subsurface.uc = ss.v[0];
                rs.subsurface.u = Point2f(ss.v[1], ss.v[2]);
            }

            return rs;
        }

        struct GetSetIndirector
        {
            LOQUAT_CPU_GPU
            operator RaySamples() const
            {
                return (*(const SOA*)soa)[index];
            }

            LOQUAT_CPU_GPU
            void operator=(RaySamples rs)
            {
                int flags = rs.have_subsurface ? 1 : 0;
                soa->direct[index] = Float4{
                    rs.direct.u[0],
                    rs.direct.u[1],
                    rs.direct.uc,
                    Float(flags)
                };
                soa->indirect[index] = Float4{
                    rs.indirect.uc,
                    rs.indirect.rr,
                    rs.indirect.u[0],
                    rs.indirect.u[1]
                };
                if (rs.have_subsurface)
                {
                    soa->subsurface[index] = Float4{
                        rs.subsurface.uc,
                        rs.subsurface.u.x,
                        rs.subsurface.u.y,
                        0.f
                    };
                }
            }

            SOA* soa;
            int index;
        };

        LOQUAT_CPU_GPU
        GetSetIndirector operator[](int i)
        {
            return GetSetIndirector{this, i};
        }

      private:
        Float4* LOQUAT_RESTRICT direct;
        Float4* LOQUAT_RESTRICT indirect;
        Float4* LOQUAT_RESTRICT subsurface;
        Float* LOQUAT_RESTRICT media_dist;
        Float* LOQUAT_RESTRICT media_mode;
    };

    struct PixelSampleState
    {
        Point2i p_pixel;
        SampledSpectrum L;
        SampledWavelengths lambda;
        Float filter_weight;
        VisibleSurface visible_surface;
        SampledSpectrum camera_ray_weight;
        RaySamples samples;
    };


}