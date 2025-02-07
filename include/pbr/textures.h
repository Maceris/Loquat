// pbrt is Copyright(c) 1998-2020 Matt Pharr, Wenzel Jakob, and Greg Humphreys.
// The pbrt source code is licensed under the Apache License, Version 2.0.
// SPDX: Apache-2.0

// This file has been modified from the original, original notice is above.

#pragma once

#include <initializer_list>
#include <map>
#include <mutex>
#include <string>

#include "main/loquat.h"

#include "pbr/base/texture.h"
#include "pbr/math/math.h"
#include "pbr/math/transform.h"
#include "pbr/math/vector_math.h"
#include "pbr/struct/interaction.h"
#include "pbr/struct/parameter_dictionary.h"
#include "pbr/util/color_space.h"
#include "pbr/util/mipmap.h"
#include "pbr/util/noise.h"
#include "pbr/util/spectrum.h"
#include "pbr/util/tagged_pointer.h"

namespace loquat
{
    struct TextureEvalContext
    {
        TextureEvalContext() = default;

        LOQUAT_CPU_GPU
        TextureEvalContext(const Interaction& intr)
            : p(intr.p())
            , uv(intr.uv)
        {}

        LOQUAT_CPU_GPU
        TextureEvalContext(const SurfaceInteraction& si)
            : p(si.p())
            , dpdx(si.dpdx)
            , dpdy(si.dpdy)
            , normal(si.normal)
            , uv(si.uv)
            , dudx(si.dudx)
            , dudy(si.dudy)
            , dvdx(si.dvdx)
            , dvdy(si.dvdy)
            , face_index(si.face_index)
        {}

        LOQUAT_CPU_GPU
        TextureEvalContext(Point3f p, Vec3f dpdx, Vec3f dpdy, Normal3f normal, Point2f uv,
            Float dudx, Float dudy, Float dvdx, Float dvdy, int face_index)
            : p(p)
            , dpdx(dpdx)
            , dpdy(dpdy)
            , normal(normal)
            , uv(uv)
            , dudx(dudx)
            , dudy(dudy)
            , dvdx(dvdx)
            , dvdy(dvdy)
            , face_index(face_index)
        {}

        std::string to_string() const;

        Point3f p;
        Vec3f dpdx;
        Vec3f dpdy;
        Normal3f normal;
        Point2f uv;
        Float dudx = 0;
        Float dudy = 0;
        Float dvdx = 0;
        Float dvdy = 0;
        int face_index = 0;
    };

    struct TexCoord2D
    {
        Point2f st;
        Float dsdx;
        Float dsdy;
        Float dtdx;
        Float dtdy;
        std::string to_string() const;
    };

    struct TexCoord3D
    {
        Point3f p;
        Vec3f dpdx;
        Vec3f dpdy;
        std::string to_string() const;
    };

    class UVMapping
    {
    public:
        UVMapping(Float su = 1, Float sv = 1, Float du = 0, Float dv = 0)
            : su(su)
            , sv(sv)
            , du(du)
            , dv(dv)
        {}

        std::string to_string() const;

        LOQUAT_CPU_GPU
            TexCoord2D map(TextureEvalContext ctx) const
        {
            // Compute texture differentials for 2D $(u,v)$ mapping
            Float dsdx = su * ctx.dudx;
            Float dsdy = su * ctx.dudy;
            Float dtdx = sv * ctx.dvdx;
            Float dtdy = sv * ctx.dvdy;

            Point2f st(su * ctx.uv[0] + du, sv * ctx.uv[1] + dv);
            return TexCoord2D{ st, dsdx, dsdy, dtdx, dtdy };
        }

    private:
        Float su;
        Float sv;
        Float du;
        Float dv;
    };

    class SphericalMapping
    {
    public:
        SphericalMapping(const Transform& texture_from_render)
            : texture_from_render(texture_from_render)
        {}

        std::string to_string() const;

        LOQUAT_CPU_GPU
        TexCoord2D map(TextureEvalContext ctx) const
        {
            Point3f pt = texture_from_render(ctx.p);
            // Compute $\partial\,s/\partial\,\pt{}$ and $\partial\,t/\partial\,\pt{}$ for
            // spherical mapping
            Float x2y2 = square(pt.x) + square(pt.y);
            Float sqrtx2y2 = std::sqrt(x2y2);
            Vec3f dsdp = Vec3f(-pt.y, pt.x, 0) / (2 * PI * x2y2);
            Vec3f dtdp =
                1 / (PI * (x2y2 + square(pt.z))) *
                Vec3f(pt.x * pt.z / sqrtx2y2, pt.y * pt.z / sqrtx2y2, -sqrtx2y2);

            // Compute texture coordinate differentials for spherical mapping
            Vec3f dpdx = texture_from_render(ctx.dpdx);
            Vec3f dpdy = texture_from_render(ctx.dpdy);
            Float dsdx = dot(dsdp, dpdx), dsdy = dot(dsdp, dpdy);
            Float dtdx = dot(dtdp, dpdx), dtdy = dot(dtdp, dpdy);

            // Return $(s,t)$ texture coordinates and differentials based on spherical mapping
            Vec3f vec = normalize(pt - Point3f(0, 0, 0));
            Point2f st(spherical_theta(vec) * INV_PI, spherical_phi(vec) * INV_2PI);
            return TexCoord2D{ st, dsdx, dsdy, dtdx, dtdy };
        }

    private:
        Transform texture_from_render;
    };

    class CylindricalMapping
    {
    public:
        CylindricalMapping(const Transform& texture_from_render)
            : texture_from_render(texture_from_render)
        {}
        std::string to_string() const;

        LOQUAT_CPU_GPU
        TexCoord2D map(TextureEvalContext ctx) const
        {
            Point3f pt = texture_from_render(ctx.p);
            // Compute texture coordinate differentials for cylinder $(u,v)$ mapping
            Float x2y2 = square(pt.x) + square(pt.y);
            
            Vec3f dsdp = Vec3f(-pt.y, pt.x, 0) / (2 * PI * x2y2);
            Vec3f dtdp = Vec3f(0, 0, 1);
            Vec3f dpdx = texture_from_render(ctx.dpdx);
            Vec3f dpdy = texture_from_render(ctx.dpdy);
            
            Float dsdx = dot(dsdp, dpdx);
            Float dsdy = dot(dsdp, dpdy);
            Float dtdx = dot(dtdp, dpdx);
            Float dtdy = dot(dtdp, dpdy);

            Point2f st((PI + std::atan2(pt.y, pt.x)) * INV_2PI, pt.z);
            return TexCoord2D{ st, dsdx, dsdy, dtdx, dtdy };
        }

    private:
        Transform texture_from_render;
    };

    class PlanarMapping {
    public:
        PlanarMapping(const Transform& texture_from_render, Vec3f vs, Vec3f vt,
            Float ds, Float dt)
            : texture_from_render(texture_from_render)
            , vs(vs)
            , vt(vt)
            , ds(ds)
            , dt(dt)
        {}

        LOQUAT_CPU_GPU
        TexCoord2D map(TextureEvalContext ctx) const
        {
            Vec3f vec(texture_from_render(ctx.p));
            // Initialize partial derivatives of planar mapping $(s,t)$ coordinates
            Vec3f dpdx = texture_from_render(ctx.dpdx);
            Vec3f dpdy = texture_from_render(ctx.dpdy);
            Float dsdx = dot(vs, dpdx);
            Float dsdy = dot(vs, dpdy);
            Float dtdx = dot(vt, dpdx);
            Float dtdy = dot(vt, dpdy);

            Point2f st(ds + dot(vec, vs), dt + dot(vec, vt));
            return TexCoord2D{ st, dsdx, dsdy, dtdx, dtdy };
        }

        std::string to_string() const;

    private:
        Transform texture_from_render;
        Vec3f vs;
        Vec3f vt;
        Float ds;
        Float dt;
    };

    class TextureMapping2D : public TaggedPointer<UVMapping, SphericalMapping,
        CylindricalMapping, PlanarMapping>
    {
    public:
        using TaggedPointer::TaggedPointer;
        LOQUAT_CPU_GPU
        TextureMapping2D(
            TaggedPointer<UVMapping, SphericalMapping, CylindricalMapping, PlanarMapping> tp)
            : TaggedPointer(tp)
        {}

        static TextureMapping2D create(const ParameterDictionary& parameters,
            const Transform& render_from_texture, const FileLoc* loc,
            Allocator alloc);

        LOQUAT_CPU_GPU inline TexCoord2D map(TextureEvalContext ctx) const;
    };

    inline TexCoord2D TextureMapping2D::map(TextureEvalContext ctx) const
    {
        auto map = [&](auto ptr) { return ptr->map(ctx); };
        return dispatch(map);
    }

    class PointTransformMapping 
    {
    public:
        PointTransformMapping(const Transform& texture_from_render)
            : texture_from_render(texture_from_render)
        {}

        std::string to_string() const;

        LOQUAT_CPU_GPU
        TexCoord3D map(TextureEvalContext ctx) const
        {
            return TexCoord3D{
                texture_from_render(ctx.p),
                texture_from_render(ctx.dpdx),
                texture_from_render(ctx.dpdy)
            };
        }

    private:
        Transform texture_from_render;
    };

    class TextureMapping3D : public TaggedPointer<PointTransformMapping>
    {
    public:
        using TaggedPointer::TaggedPointer;
        LOQUAT_CPU_GPU
        TextureMapping3D(TaggedPointer<PointTransformMapping> tp)
            : TaggedPointer(tp)
        {}

        static TextureMapping3D create(const ParameterDictionary& parameters,
            const Transform& render_from_texture, const FileLoc* loc,
            Allocator alloc);

        LOQUAT_CPU_GPU
        TexCoord3D map(TextureEvalContext ctx) const;
    };

    inline TexCoord3D TextureMapping3D::map(TextureEvalContext ctx) const
    {
        auto map = [&](auto ptr) { return ptr->map(ctx); };
        return dispatch(map);
    }

    class FloatConstantTexture {
    public:
        FloatConstantTexture(Float value)
            : value(value)
        {}
        LOQUAT_CPU_GPU
        Float evaluate(TextureEvalContext ctx) const
        {
            return value;
        }

        static FloatConstantTexture* create(const Transform& render_from_texture,
            const TextureParameterDictionary& parameters,
            const FileLoc* loc, Allocator alloc);

        std::string to_string() const;

    private:
        Float value;
    };

    class SpectrumConstantTexture {
    public:
        SpectrumConstantTexture(Spectrum value)
            : value(value)
        {}

        LOQUAT_CPU_GPU
        SampledSpectrum evaluate(TextureEvalContext ctx,
            SampledWavelengths lambda) const
        {
            return value.sample(lambda);
        }

        static SpectrumConstantTexture* create(const Transform& render_from_texture,
            const TextureParameterDictionary& parameters,
            SpectrumType spectrum_type, const FileLoc* loc, Allocator alloc);

        std::string to_string() const;

    private:
        Spectrum value;
    };

    class FloatBilerpTexture
    {
    public:
        FloatBilerpTexture(TextureMapping2D mapping, Float v00, Float v01, Float v10,
            Float v11)
            : mapping(mapping)
            , v00(v00)
            , v01(v01)
            , v10(v10)
            , v11(v11)
        {}

        LOQUAT_CPU_GPU
        Float evaluate(TextureEvalContext ctx) const
        {
            TexCoord2D c = mapping.map(ctx);
            return (1 - c.st[0]) * (1 - c.st[1]) * v00 + c.st[0] * (1 - c.st[1]) * v10 
                + (1 - c.st[0]) * c.st[1] * v01 + c.st[0] * c.st[1] * v11;
        }

        static FloatBilerpTexture* create(const Transform& render_from_texture,
            const TextureParameterDictionary& parameters,
            const FileLoc* loc, Allocator alloc);

        std::string to_string() const;

    private:
        TextureMapping2D mapping;
        Float v00;
        Float v01;
        Float v10;
        Float v11;
    };

    class SpectrumBilerpTexture
    {
    public:
        SpectrumBilerpTexture(TextureMapping2D mapping, Spectrum v00, Spectrum v01,
            Spectrum v10, Spectrum v11)
            : mapping(mapping)
            , v00(v00)
            , v01(v01)
            , v10(v10)
            , v11(v11)
        {}

        LOQUAT_CPU_GPU
        SampledSpectrum evaluate(TextureEvalContext ctx,
            SampledWavelengths lambda) const
        {
            TexCoord2D c = mapping.map(ctx);
            return bilerp(
                { c.st[0], c.st[1] },
                { v00.sample(lambda), v10.sample(lambda)
                , v01.sample(lambda), v11.sample(lambda)
                }
            );
        }

        static SpectrumBilerpTexture* create(const Transform& render_from_texture,
            const TextureParameterDictionary& parameters,
            SpectrumType spectrum_type, const FileLoc* loc,
            Allocator alloc);

        std::string to_string() const;

    private:
        TextureMapping2D mapping;
        Spectrum v00;
        Spectrum v01;
        Spectrum v10;
        Spectrum v11;
    };

    LOQUAT_CPU_GPU
    Float checkerboard(TextureEvalContext ctx, TextureMapping2D map2D,
        TextureMapping3D map3D);

    class FloatCheckerboardTexture
    {
    public:
        FloatCheckerboardTexture(TextureMapping2D map2D, TextureMapping3D map3D,
            FloatTexture tex1, FloatTexture tex2)
            : map2D(map2D)
            , map3D(map3D)
            , tex{ tex1, tex2 }
        {}

        LOQUAT_CPU_GPU
        Float evaluate(TextureEvalContext ctx) const
        {
            Float w = checkerboard(ctx, map2D, map3D);
            Float t0 = 0, t1 = 0;
            if (w != 1)
            {
                t0 = tex[0].evaluate(ctx);
            }
            if (w != 0)
            {
                t1 = tex[1].evaluate(ctx);
            }
            return (1 - w) * t0 + w * t1;
        }

        static FloatCheckerboardTexture* create(
            const Transform& render_from_texture,
            const TextureParameterDictionary& parameters,
            const FileLoc* loc, Allocator alloc);

        std::string to_string() const;

    private:
        TextureMapping2D map2D;
        TextureMapping3D map3D;
        FloatTexture tex[2];
    };

    class SpectrumCheckerboardTexture
    {
    public:
        SpectrumCheckerboardTexture(TextureMapping2D map2D, TextureMapping3D map3D,
            SpectrumTexture tex1, SpectrumTexture tex2)
            : map2D(map2D)
            , map3D(map3D)
            , tex{ tex1, tex2 }
        {}

        static SpectrumCheckerboardTexture* create(
            const Transform& render_from_texture, const TextureParameterDictionary& parameters,
            SpectrumType spectrum_type, const FileLoc* loc, Allocator alloc);

        std::string to_string() const;

        LOQUAT_CPU_GPU
        SampledSpectrum evaluate(TextureEvalContext ctx, SampledWavelengths lambda) const
        {
            Float w = checkerboard(ctx, map2D, map3D);
            SampledSpectrum t0, t1;
            if (w != 1)
            {
                t0 = tex[0].evaluate(ctx, lambda);
            }
            if (w != 0)
            {
                t1 = tex[1].evaluate(ctx, lambda);
            }
            return (1 - w) * t0 + w * t1;
        }

    private:
        TextureMapping2D map2D;
        TextureMapping3D map3D;
        SpectrumTexture tex[2];
    };

    LOQUAT_CPU_GPU
    bool inside_polka_dot(Point2f st);

    class FloatDotsTexture {
    public:
        FloatDotsTexture(TextureMapping2D mapping, FloatTexture outside_dot,
            FloatTexture inside_dot)
            : mapping(mapping)
            , outside_dot(outside_dot)
            , inside_dot(inside_dot)
        {}

        LOQUAT_CPU_GPU
        Float evaluate(TextureEvalContext ctx) const
        {
            TexCoord2D c = mapping.map(ctx);
            return inside_polka_dot(c.st) 
                ? inside_dot.evaluate(ctx) 
                : outside_dot.evaluate(ctx);
        }

        static FloatDotsTexture* create(const Transform& render_from_texture,
            const TextureParameterDictionary& parameters,
            const FileLoc* loc, Allocator alloc);

        std::string to_string() const;

    private:
        TextureMapping2D mapping;
        FloatTexture outside_dot;
        FloatTexture inside_dot;
    };

    class SpectrumDotsTexture
    {
    public:
        SpectrumDotsTexture(TextureMapping2D mapping, SpectrumTexture outside_dot,
            SpectrumTexture inside_dot)
            : mapping(mapping)
            , outside_dot(outside_dot)
            , inside_dot(inside_dot)
        {}

        LOQUAT_CPU_GPU
        SampledSpectrum evaluate(TextureEvalContext ctx, SampledWavelengths lambda) const
        {
            TexCoord2D c = mapping.map(ctx);
            return inside_polka_dot(c.st) ? inside_dot.evaluate(ctx, lambda)
                : outside_dot.evaluate(ctx, lambda);
        }

        static SpectrumDotsTexture* create(const Transform& render_from_texture,
            const TextureParameterDictionary& parameters,
            SpectrumType spectrum_type, const FileLoc* loc,
            Allocator alloc);

        std::string to_string() const;

    private:
        TextureMapping2D mapping;
        SpectrumTexture outside_dot;
        SpectrumTexture inside_dot;
    };

    class FBMTexture
    {
    public:
        FBMTexture(TextureMapping3D mapping, int octaves, Float omega)
            : mapping(mapping)
            , omega(omega)
            , octaves(octaves)
        {}

        LOQUAT_CPU_GPU
        Float evaluate(TextureEvalContext ctx) const
        {
            TexCoord3D c = mapping.map(ctx);
            return fbm(c.p, c.dpdx, c.dpdy, omega, octaves);
        }

        static FBMTexture* create(const Transform& render_from_texture,
            const TextureParameterDictionary& parameters,
            const FileLoc* loc, Allocator alloc);

        std::string to_string() const;

    private:
        TextureMapping3D mapping;
        Float omega;
        int octaves;
    };

    struct TexInfo
    {
        TexInfo(const std::string& f, MIPMapFilterOptions filter_options,
            WrapMode wm, ColorEncoding encoding)
            : filename(f)
            , filter_options(filter_options)
            , wrap_mode(wm)
            , encoding(encoding)
        {}

        bool operator<(const TexInfo& t) const
        {
            return std::tie(filename, filter_options, encoding, wrap_mode) <
                std::tie(t.filename, t.filter_options, t.encoding, t.wrap_mode);
        }

        std::string to_string() const;

        std::string filename;
        MIPMapFilterOptions filter_options;
        WrapMode wrap_mode;
        ColorEncoding encoding;
    };

    class ImageTextureBase {
    public:
        ImageTextureBase(TextureMapping2D mapping, std::string filename,
            MIPMapFilterOptions filter_options, WrapMode wrap_mode, Float scale,
            bool invert, ColorEncoding encoding, Allocator alloc)
            : mapping(mapping)
            , filename(filename)
            , scale(scale)
            , invert(invert)
        {
            // Get _MIPMap_ from texture cache if present
            TexInfo texInfo(filename, filter_options, wrap_mode, encoding);
            std::unique_lock<std::mutex> lock(texture_cache_mutex);
            if (auto iter = texture_cache.find(texInfo); iter != texture_cache.end())
            {
                mipmap = iter->second;
                return;
            }
            lock.unlock();

            // create _MIPMap_ for _filename_ and add to texture cache
            mipmap = MIPMap::create_from_file(filename, filter_options,
                wrap_mode, encoding, alloc);
            lock.lock();
            // This is actually ok, but if it hits, it means we've wastefully
            // loaded this texture. (Note that in that case, should just return
            // the one that's already in there and not replace it.)
            LOG_ASSERT(texture_cache.find(texInfo) == texture_cache.end());
            texture_cache[texInfo] = mipmap;
        }

        static void clear_cache() { texture_cache.clear(); }

        void multiply_scale(Float s) { scale *= s; }

    protected:
        TextureMapping2D mapping;
        std::string filename;
        Float scale;
        bool invert;
        MIPMap* mipmap;

    private:
        static std::mutex texture_cache_mutex;
        static std::map<TexInfo, MIPMap*> texture_cache;
    };

    class FloatImageTexture : public ImageTextureBase
    {
    public:
        FloatImageTexture(TextureMapping2D m, const std::string& filename,
            MIPMapFilterOptions filter_options, WrapMode wm, Float scale,
            bool invert, ColorEncoding encoding, Allocator alloc)
            : ImageTextureBase(m, filename, filter_options, wm, scale, invert, encoding,
                alloc) {}
        LOQUAT_CPU_GPU
        Float evaluate(TextureEvalContext ctx) const
        {
#ifdef LOQUAT_IS_GPU_CODE
            assert(!"Should not be called in GPU code");
            return 0;
#else
            TexCoord2D c = mapping.map(ctx);
            // Texture coordinates are (0,0) in the lower left corner, but
            // image coordinates are (0,0) in the upper left.
            c.st[1] = 1 - c.st[1];
            Float v = scale * mipmap->Filter<Float>(c.st, { c.dsdx, c.dtdx }, { c.dsdy, c.dtdy });
            return invert ? std::max<Float>(0, 1 - v) : v;
#endif
        }

        static FloatImageTexture* create(const Transform& render_from_texture,
            const TextureParameterDictionary& parameters,
            const FileLoc* loc, Allocator alloc);

        std::string to_string() const;
    };

    class SpectrumImageTexture : public ImageTextureBase
    {
    public:
        SpectrumImageTexture(TextureMapping2D mapping, std::string filename,
            MIPMapFilterOptions filter_options, WrapMode wrap_mode,
            Float scale, bool invert, ColorEncoding encoding,
            SpectrumType spectrum_type, Allocator alloc)
            : ImageTextureBase(mapping, filename, filter_options, wrap_mode,
                scale, invert, encoding, alloc)
            , spectrum_type(spectrum_type)
        {}

        LOQUAT_CPU_GPU
        SampledSpectrum evaluate(TextureEvalContext ctx, SampledWavelengths lambda) const;

        static SpectrumImageTexture* create(const Transform& render_from_texture,
            const TextureParameterDictionary& parameters,
            SpectrumType spectrum_type, const FileLoc* loc,
            Allocator alloc);

        std::string to_string() const;

    private:
        SpectrumType spectrum_type;
    };

#if defined(LOQUAT_BUILD_GPU_RENDERER) && defined(__NVCC__)
    class GPUSpectrumImageTexture
    {
    public:
        GPUSpectrumImageTexture(std::string filename, TextureMapping2D mapping,
            cudaTextureObject_t tex_obj, Float scale, bool invert,
            bool is_single_channel, const RGBColorSpace* color_space,
            SpectrumType spectrum_type)
            : mapping(mapping)
            , filename(filename)
            , tex_obj(tex_obj)
            , scale(scale)
            , invert(invert)
            , is_single_channel(is_single_channel)
            , color_space(color_space)
            , spectrum_type(spectrum_type)
        {}

        LOQUAT_CPU_GPU
        SampledSpectrum evaluate(TextureEvalContext ctx,
            SampledWavelengths lambda) const
        {
#ifndef LOQUAT_IS_GPU_CODE
            LOG_FATAL("GPUSpectrumImageTexture::evaluate called from CPU");
            return SampledSpectrum(0);
#else
            // flip y coord since image has (0,0) at upper left, texture at lower
            // left
            TexCoord2D c = mapping.map(ctx);
            RGB rgb;
            if (is_single_channel) {
                float tex = scale * tex2DGrad<float>(tex_obj, c.st[0], 1 - c.st[1],
                    make_float2(c.dsdx, c.dsdy),
                    make_float2(c.dtdx, c.dtdy));
                rgb = RGB(tex, tex, tex);
            }
            else {
                float4 tex = tex2DGrad<float4>(tex_obj, c.st[0], 1 - c.st[1],
                    make_float2(c.dsdx, c.dsdy),
                    make_float2(c.dtdx, c.dtdy));
                rgb = scale * RGB(tex.x, tex.y, tex.z);
            }
            if (invert)
                rgb = ClampZero(RGB(1, 1, 1) - rgb);
            if (spectrum_type == SpectrumType::Unbounded)
                return RGBUnboundedSpectrum(*color_space, rgb).sample(lambda);
            else if (spectrum_type == SpectrumType::Albedo) {
                rgb = clamp(rgb, 0, 1);
                return RGBAlbedoSpectrum(*color_space, rgb).sample(lambda);
            }
            else
                return RGBIlluminantSpectrum(*color_space, rgb).sample(lambda);
#endif
        }

        static GPUSpectrumImageTexture* create(const Transform& render_from_texture,
            const TextureParameterDictionary& parameters,
            SpectrumType spectrum_type, const FileLoc* loc,
            Allocator alloc);

        std::string to_string() const;

        void multiply_scale(Float s) { scale *= s; }

        TextureMapping2D mapping;
        std::string filename;
        cudaTextureObject_t tex_obj;
        Float scale;
        bool invert;
        bool is_single_channel;
        const RGBColorSpace* color_space;
        SpectrumType spectrum_type;
    };

    class GPUFloatImageTexture
    {
    public:
        GPUFloatImageTexture(std::string filename, TextureMapping2D mapping,
            cudaTextureObject_t tex_obj, Float scale, bool invert)
            : mapping(mapping)
            , filename(filename)
            , tex_obj(tex_obj)
            , scale(scale)
            , invert(invert)
        {}

        LOQUAT_CPU_GPU
        Float evaluate(TextureEvalContext ctx) const
        {
#ifndef LOQUAT_IS_GPU_CODE
            LOG_FATAL("GPUSpectrumImageTexture::evaluate called from CPU");
            return 0;
#else
            TexCoord2D c = mapping.map(ctx);
            // flip y coord since image has (0,0) at upper left, texture at lower
            // left
            Float v = scale * tex2DGrad<float>(tex_obj, c.st[0], 1 - c.st[1],
                make_float2(c.dsdx, c.dsdy),
                make_float2(c.dtdx, c.dtdy));
            return invert ? std::max<Float>(0, 1 - v) : v;
#endif
        }

        static GPUFloatImageTexture* create(const Transform& render_from_texture,
            const TextureParameterDictionary& parameters,
            const FileLoc* loc, Allocator alloc);

        std::string to_string() const;

        void multiply_scale(Float s) { scale *= s; }

        TextureMapping2D mapping;
        std::string filename;
        cudaTextureObject_t tex_obj;
        Float scale;
        bool invert;
    };

#else  // LOQUAT_BUILD_GPU_RENDERER && __NVCC__

    class GPUSpectrumImageTexture
    {
    public:
        SampledSpectrum evaluate(TextureEvalContext ctx, SampledWavelengths lambda) const
        {
            LOG_FATAL("GPUSpectrumImageTexture::evaluate called from CPU");
            return SampledSpectrum(0);
        }

        static GPUSpectrumImageTexture* create(const Transform& render_from_texture,
            const TextureParameterDictionary& parameters,
            SpectrumType spectrum_type, const FileLoc* loc,
            Allocator alloc)
        {
            LOG_FATAL("GPUSpectrumImageTexture::create called in non-GPU configuration.");
            return nullptr;
        }

        std::string to_string() const { return "GPUSpectrumImageTexture"; }
    };

    class GPUFloatImageTexture
    {
    public:
        Float evaluate(const TextureEvalContext&) const {
            LOG_FATAL("GPUFloatImageTexture::evaluate called from CPU");
            return 0;
        }

        static GPUFloatImageTexture* create(const Transform& render_from_texture,
            const TextureParameterDictionary& parameters,
            const FileLoc* loc, Allocator alloc)
        {
            LOG_FATAL("GPUFloatImageTexture::create called in non-GPU configuration.");
            return nullptr;
        }

        std::string to_string() const { return "GPUFloatImageTexture"; }
    };

#endif  // LOQUAT_BUILD_GPU_RENDERER && __NVCC__

    class MarbleTexture
    {
    public:
        MarbleTexture(TextureMapping3D mapping, int octaves, Float omega,
            Float scale, Float variation)
            : mapping(mapping)
            , octaves(octaves)
            , omega(omega)
            , scale(scale)
            , variation(variation)
        {}

        LOQUAT_CPU_GPU
        SampledSpectrum evaluate(TextureEvalContext ctx, SampledWavelengths lambda) const;

        static MarbleTexture* create(const Transform& render_from_texture,
            const TextureParameterDictionary& parameters,
            const FileLoc* loc, Allocator alloc);

        std::string to_string() const;

    private:
        TextureMapping3D mapping;
        int octaves;
        Float omega;
        Float scale;
        Float variation;
    };

    class FloatMixTexture
    {
    public:
        FloatMixTexture(FloatTexture tex1, FloatTexture tex2, FloatTexture amount)
            : tex1(tex1)
            , tex2(tex2)
            , amount(amount)
        {}

        LOQUAT_CPU_GPU
        Float evaluate(TextureEvalContext ctx) const
        {
            Float amt = amount.evaluate(ctx);
            Float t1 = 0, t2 = 0;
            if (amt != 1)
            {
                t1 = tex1.evaluate(ctx);
            }
            if (amt != 0)
            {
                t2 = tex2.evaluate(ctx);
            }
            return (1 - amt) * t1 + amt * t2;
        }

        static FloatMixTexture* create(const Transform& render_from_texture,
            const TextureParameterDictionary& parameters,
            const FileLoc* loc, Allocator alloc);

        std::string to_string() const;

    private:
        FloatTexture tex1;
        FloatTexture tex2;
        FloatTexture amount;
    };

    class FloatDirectionMixTexture
    {
    public:
        FloatDirectionMixTexture(FloatTexture tex1, FloatTexture tex2, Vec3f dir)
            : tex1(tex1)
            , tex2(tex2)
            , dir(dir)
        {}

        LOQUAT_CPU_GPU
        Float evaluate(TextureEvalContext ctx) const
        {
            Float amt = absolute_dot(ctx.normal, dir);
            Float t1 = 0, t2 = 0;
            if (amt != 0)
            {
                t1 = tex1.evaluate(ctx);
            }
            if (amt != 1)
            {
                t2 = tex2.evaluate(ctx);
            }
            return amt * t1 + (1 - amt) * t2;
        }

        static FloatDirectionMixTexture* create(
            const Transform& render_from_texture,
            const TextureParameterDictionary& parameters,
            const FileLoc* loc, Allocator alloc);

        std::string to_string() const;

    private:
        FloatTexture tex1;
        FloatTexture tex2;
        Vec3f dir;
    };

    class SpectrumMixTexture
    {
    public:
        SpectrumMixTexture(SpectrumTexture tex1, SpectrumTexture tex2,
            FloatTexture amount)
            : tex1(tex1)
            , tex2(tex2)
            , amount(amount)
        {}

        LOQUAT_CPU_GPU
        SampledSpectrum evaluate(TextureEvalContext ctx,
            SampledWavelengths lambda) const
        {
            Float amt = amount.evaluate(ctx);
            SampledSpectrum t1, t2;
            if (amt != 1)
            {
                t1 = tex1.evaluate(ctx, lambda);
            }
            if (amt != 0)
            {
                t2 = tex2.evaluate(ctx, lambda);
            }
            return (1 - amt) * t1 + amt * t2;
        }

        static SpectrumMixTexture* create(const Transform& render_from_texture,
            const TextureParameterDictionary& parameters,
            SpectrumType spectrum_type, const FileLoc* loc,
            Allocator alloc);

        std::string to_string() const;

    private:
        SpectrumTexture tex1;
        SpectrumTexture tex2;
        FloatTexture amount;
    };

    class SpectrumDirectionMixTexture
    {
    public:
        SpectrumDirectionMixTexture(SpectrumTexture tex1, SpectrumTexture tex2,
            Vec3f dir)
            : tex1(tex1)
            , tex2(tex2)
            , dir(dir)
        {}

        LOQUAT_CPU_GPU
        SampledSpectrum evaluate(TextureEvalContext ctx,
            SampledWavelengths lambda) const
        {
            Float amt = absolute_dot(ctx.normal, dir);
            SampledSpectrum t1, t2;
            if (amt != 0)
            {
                t1 = tex1.evaluate(ctx, lambda);
            }
            if (amt != 1)
            {
                t2 = tex2.evaluate(ctx, lambda);
            }
            return amt * t1 + (1 - amt) * t2;
        }

        static SpectrumDirectionMixTexture* create(
            const Transform& render_from_texture,
            const TextureParameterDictionary& parameters,
            SpectrumType spectrum_type, const FileLoc* loc, Allocator alloc);

        std::string to_string() const;

    private:
        SpectrumTexture tex1;
        SpectrumTexture tex2;
        Vec3f dir;
    };

    class PtexTextureBase
    {
    public:
        PtexTextureBase(const std::string& filename, ColorEncoding encoding,
            Float scale);

        static void report_stats();

        int sample_texture(TextureEvalContext ctx, float* result) const;

    protected:
        std::string base_to_string() const;

    private:
        bool valid;
        std::string filename;
        ColorEncoding encoding;
        Float scale;
    };

    class FloatPtexTexture : public PtexTextureBase
    {
    public:
        FloatPtexTexture(const std::string& filename, ColorEncoding encoding,
            Float scale)
            : PtexTextureBase(filename, encoding, scale)
        {}

        LOQUAT_CPU_GPU
        Float evaluate(TextureEvalContext ctx) const;

        static FloatPtexTexture* create(const Transform& render_from_texture,
            const TextureParameterDictionary& parameters,
            const FileLoc* loc, Allocator alloc);
        std::string to_string() const;
    };

    class SpectrumPtexTexture : public PtexTextureBase {
    public:
        SpectrumPtexTexture(const std::string& filename,
            ColorEncoding encoding, Float scale, SpectrumType spectrum_type)
            : PtexTextureBase(filename, encoding, scale)
            , spectrum_type(spectrum_type)
        {}

        LOQUAT_CPU_GPU
        SampledSpectrum evaluate(TextureEvalContext ctx,
            SampledWavelengths lambda) const;

        static SpectrumPtexTexture* create(
            const Transform& render_from_texture,
            const TextureParameterDictionary& parameters,
            SpectrumType spectrum_type, const FileLoc* loc,
            Allocator alloc);

        std::string to_string() const;

    private:
        SpectrumType spectrum_type;
    };

    class GPUFloatPtexTexture
    {
    public:
        GPUFloatPtexTexture(const std::string& filename,
            ColorEncoding encoding, Float scale, Allocator alloc);

        LOQUAT_CPU_GPU
        Float evaluate(TextureEvalContext ctx) const
        {
            LOG_ASSERT(ctx.face_index >= 0 && ctx.face_index < faceValues.size());
            return faceValues[ctx.face_index];
        }

        static GPUFloatPtexTexture* create(
            const Transform& render_from_texture,
            const TextureParameterDictionary& parameters,
            const FileLoc* loc, Allocator alloc);
        std::string to_string() const;

    private:
        pstd::vector<Float> faceValues;
    };

    class GPUSpectrumPtexTexture
    {
    public:
        GPUSpectrumPtexTexture(const std::string& filename,
            ColorEncoding encoding, Float scale, SpectrumType spectrum_type,
            Allocator alloc);

        LOQUAT_CPU_GPU
        SampledSpectrum evaluate(TextureEvalContext ctx, SampledWavelengths lambda) const
        {
            LOG_ASSERT(ctx.face_index >= 0 && ctx.face_index < faceValues.size());

            RGB rgb = faceValues[ctx.face_index];
            const RGBColorSpace* sRGB =
#ifdef LOQUAT_IS_GPU_CODE
                RGBColorSpace_sRGB;
#else
                RGBColorSpace::sRGB;
#endif
            if (spectrum_type == SpectrumType::Unbounded)
            {
                return RGBUnboundedSpectrum(*sRGB, rgb).sample(lambda);
            }
            else if (spectrum_type == SpectrumType::Albedo)
            {
                return RGBAlbedoSpectrum(*sRGB, clamp(rgb, 0, 1)).sample(lambda);
            }
            else
            {
                return RGBIlluminantSpectrum(*sRGB, rgb).sample(lambda);
            }
        }

        static GPUSpectrumPtexTexture* create(const Transform& render_from_texture,
            const TextureParameterDictionary& parameters,
            SpectrumType spectrum_type, const FileLoc* loc,
            Allocator alloc);

        std::string to_string() const;

    private:
        SpectrumType spectrum_type;
        pstd::vector<RGB> faceValues;
    };

    class FloatScaledTexture
    {
    public:
        FloatScaledTexture(FloatTexture tex, FloatTexture scale)
            : tex(tex)
            , scale(scale)
        {}

        static FloatTexture create(const Transform& render_from_texture,
            const TextureParameterDictionary& parameters,
            const FileLoc* loc, Allocator alloc);

        LOQUAT_CPU_GPU
        Float evaluate(TextureEvalContext ctx) const
        {
            Float sc = scale.evaluate(ctx);
            if (sc == 0)
            {
                return 0;
            }
            return tex.evaluate(ctx) * sc;
        }

        std::string to_string() const;

    private:
        FloatTexture tex;
        FloatTexture scale;
    };

    class SpectrumScaledTexture
    {
    public:
        SpectrumScaledTexture(SpectrumTexture tex, FloatTexture scale)
            : tex(tex)
            , scale(scale)
        {}

        LOQUAT_CPU_GPU
            SampledSpectrum evaluate(TextureEvalContext ctx,
                SampledWavelengths lambda) const
        {
            Float sc = scale.evaluate(ctx);
            if (sc == 0)
            {
                return SampledSpectrum(0.f);
            }
            return tex.evaluate(ctx, lambda) * sc;
        }

        static SpectrumTexture create(const Transform& render_from_texture,
            const TextureParameterDictionary& parameters,
            SpectrumType spectrum_type, const FileLoc* loc,
            Allocator alloc);

        std::string to_string() const;

    private:
        SpectrumTexture tex;
        FloatTexture scale;
    };

    class WindyTexture {
    public:
        WindyTexture(TextureMapping3D mapping)
            : mapping(mapping)
        {}

        LOQUAT_CPU_GPU
        Float evaluate(TextureEvalContext ctx) const
        {
            TexCoord3D c = mapping.map(ctx);
            Float windStrength = fbm(.1f * c.p, .1f * c.dpdx, .1f * c.dpdy, .5, 3);
            Float waveHeight = fbm(c.p, c.dpdx, c.dpdy, .5, 6);
            return std::abs(windStrength) * waveHeight;
        }

        static WindyTexture* create(const Transform& render_from_texture,
            const TextureParameterDictionary& parameters,
            const FileLoc* loc, Allocator alloc);

        std::string to_string() const;

    private:
        TextureMapping3D mapping;
    };

    class WrinkledTexture
    {
    public:
        WrinkledTexture(TextureMapping3D mapping, int octaves, Float omega)
            : mapping(mapping)
            , octaves(octaves)
            , omega(omega)
        {}

        LOQUAT_CPU_GPU
        Float evaluate(TextureEvalContext ctx) const
        {
            TexCoord3D c = mapping.map(ctx);
            return turbulence(c.p, c.dpdx, c.dpdy, omega, octaves);
        }

        static WrinkledTexture* create(const Transform& render_from_texture,
            const TextureParameterDictionary& parameters,
            const FileLoc* loc, Allocator alloc);

        std::string to_string() const;

    private:
        TextureMapping3D mapping;
        int octaves;
        Float omega;
    };

    inline Float FloatTexture::evaluate(TextureEvalContext ctx) const
    {
        auto eval = [&](auto ptr) { return ptr->evaluate(ctx); };
        return dispatch(eval);
    }

    inline SampledSpectrum SpectrumTexture::evaluate(TextureEvalContext ctx,
        SampledWavelengths lambda) const
    {
        auto eval = [&](auto ptr) { return ptr->evaluate(ctx, lambda); };
        return dispatch(eval);
    }

    class UniversalTextureEvaluator
    {
    public:
        LOQUAT_CPU_GPU
        bool can_evaluate(std::initializer_list<FloatTexture>,
            std::initializer_list<SpectrumTexture>) const
        {
            return true;
        }

        LOQUAT_CPU_GPU
        Float operator()(FloatTexture tex, TextureEvalContext ctx);

        LOQUAT_CPU_GPU
        SampledSpectrum operator()(SpectrumTexture tex, TextureEvalContext ctx,
            SampledWavelengths lambda);
    };

    class BasicTextureEvaluator {
    public:
        LOQUAT_CPU_GPU
        bool can_evaluate(std::initializer_list<FloatTexture> ftex,
            std::initializer_list<SpectrumTexture> stex) const
        {
            // Return _false_ if any _FloatTexture_s cannot be evaluated
            for (FloatTexture f : ftex)
            {
                if (f && !f.is<FloatConstantTexture>() && !f.is<FloatImageTexture>() &&
                    !f.is<GPUFloatPtexTexture>() && !f.is<GPUFloatImageTexture>())
                {
                    return false;
                }
            }

            // Return _false_ if any _SpectrumTexture_s cannot be evaluated
            for (SpectrumTexture s : stex)
            {
                if (s && !s.is<SpectrumConstantTexture>() && !s.is<SpectrumImageTexture>() &&
                    !s.is<GPUSpectrumPtexTexture>() && !s.is<GPUSpectrumImageTexture>())
                {
                    return false;
                }
            }

            return true;
        }

        LOQUAT_CPU_GPU
        Float operator()(FloatTexture tex, TextureEvalContext ctx)
        {
            if (tex.is<FloatConstantTexture>())
            {
                return tex.cast<FloatConstantTexture>()->evaluate(ctx);
            }
            else if (tex.is<FloatImageTexture>())
            {
                return tex.cast<FloatImageTexture>()->evaluate(ctx);
            }
            else if (tex.is<GPUFloatImageTexture>())
            {
                return tex.cast<GPUFloatImageTexture>()->evaluate(ctx);
            }
            else if (tex.is<GPUFloatPtexTexture>())
            {
                return tex.cast<GPUFloatPtexTexture>()->evaluate(ctx);
            }
            else
            {
                if (tex)
                {
                    LOG_FATAL(std::format("BasicTextureEvaluator::operator() called with {}", tex));
                }
                return 0.0f;
            }
        }

        LOQUAT_CPU_GPU
        SampledSpectrum operator()(SpectrumTexture tex, TextureEvalContext ctx,
            SampledWavelengths lambda)
        {
            if (tex.is<SpectrumConstantTexture>())
            {
                return tex.cast<SpectrumConstantTexture>()->evaluate(ctx, lambda);
            }
            else if (tex.is<SpectrumImageTexture>())
            {
                return tex.cast<SpectrumImageTexture>()->evaluate(ctx, lambda);
            }
            else if (tex.is<GPUSpectrumImageTexture>())
            {
                return tex.cast<GPUSpectrumImageTexture>()->evaluate(ctx, lambda);
            }
            else if (tex.is<GPUSpectrumPtexTexture>())
            {
                return tex.cast<GPUSpectrumPtexTexture>()->evaluate(ctx, lambda);
            }
            else
            {
                if (tex)
                {
                    LOG_FATAL(std::format("BasicTextureEvaluator::operator() called with {}", tex));
                }
                return SampledSpectrum(0.f);
            }
        }
    };
	
}