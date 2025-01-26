// pbrt is Copyright(c) 1998-2020 Matt Pharr, Wenzel Jakob, and Greg Humphreys.
// The pbrt source code is licensed under the Apache License, Version 2.0.
// SPDX: Apache-2.0

// PhysLight code contributed by Anders Langlands and Luca Fascione
// Copyright (c) 2020, Weta Digital, Ltd.
// SPDX-License-Identifier: Apache-2.0

// This file has been modified from the original, original notice is above.

#pragma once

//TODO(ches) fill this out

#include <functional>
#include <memory>
#include <string>

#include "main/loquat.h"

#include "debug/logger.h"
#include "pbr/base/light.h"
#include "pbr/base/medium.h"
#include "pbr/base/texture.h"
#include "pbr/shapes.h"
#include "pbr/textures.h"
#include "pbr/struct/image.h"
#include "pbr/struct/interaction.h"
#include "pbr/util/pstd.h"
#include "pbr/util/sampling.h"
#include "pbr/util/spectrum.h"
#include "pbr/math/transform.h"
#include "pbr/math/vector_math.h"

namespace loquat
{
    std::string to_string(LightType type);

    LOQUAT_CPU_GPU
    inline bool is_delta_light(LightType type)
    {
        return (type == LightType::DeltaPosition || type == LightType::DeltaDirection);
    }

    struct LightIncidentSample
    {
        LightIncidentSample() = default;

        LOQUAT_CPU_GPU
        LightIncidentSample(const SampledSpectrum& radiance,
            Vec3f incoming, Float pdf,
            const Interaction& p_light)
            : radiance(radiance)
            , incoming(incoming)
            , pdf(pdf)
            , p_light(p_light)
        {}

        std::string to_string() const;

        SampledSpectrum radiance;
        Vec3f incoming;
        Float pdf;
        Interaction p_light;
    };

    struct LightLeSample
    {
        LightLeSample() = default;
        LOQUAT_CPU_GPU
        LightLeSample(const SampledSpectrum& radiance,
            const Ray& ray, Float pdf_pos, Float pdf_dir)
            : radiance(radiance)
            , ray(ray)
            , pdf_pos(pdf_pos)
            , pdf_dir(pdf_dir)
        {}
        LOQUAT_CPU_GPU
        LightLeSample(const SampledSpectrum& radiance, const Ray& ray,
            const Interaction& intr, Float pdf_pos, Float pdf_dir)
            : radiance(radiance)
            , ray(ray)
            , intr(intr)
            , pdf_pos(pdf_pos)
            , pdf_dir(pdf_dir)
        {
            LOG_ASSERT(this->intr->normal != Normal3f(0, 0, 0));
        }
        std::string to_string() const;

        LOQUAT_CPU_GPU
        Float abs_cos_theta(Vec3f direction) const
        {
            return intr ? absolute_dot(direction, intr->normal) : 1;
        }

        SampledSpectrum radiance;
        Ray ray;
        pstd::optional<Interaction> intr;
        Float pdf_pos = 0;
        Float pdf_dir = 0;
    };

    class LightSampleContext
    {
    public:
        LightSampleContext() = default;
        LOQUAT_CPU_GPU
            LightSampleContext(const SurfaceInteraction& si)
            : pi(si.point)
            , normal(si.normal)
            , shading_normal(si.shading.normal)
        {}

        LOQUAT_CPU_GPU
        LightSampleContext(const Interaction& intr)
            : pi(intr.point)
        {}

        LOQUAT_CPU_GPU
        LightSampleContext(Point3fi pi, Normal3f normal, Normal3f shading_normal)
            : pi(pi)
            , normal(normal)
            , shading_normal(shading_normal)
        {}

        LOQUAT_CPU_GPU
        Point3f p() const { return Point3f(pi); }

        Point3fi pi;
        Normal3f normal;
        Normal3f shading_normal;
    };

    class LightBounds
    {
    public:
        LightBounds() = default;
        LightBounds(const AABB3f& b, Vec3f direction, Float phi,
            Float cos_theta_o, Float cos_theta_e, bool two_sided);

        LOQUAT_CPU_GPU
        Point3f centroid() const
        {
            return (bounds.min + bounds.max) / 2.0f;
        }

        LOQUAT_CPU_GPU
        Float importance(Point3f p, Normal3f normal) const;

        std::string to_string() const;

        AABB3f bounds;
        Float phi = 0;
        Vec3f direction;
        Float cos_theta_o;
        Float cos_theta_e;
        bool two_sided;
    };

    inline LightBounds::LightBounds(const AABB3f& b, Vec3f direction, Float phi,
        Float cos_theta_o, Float cos_theta_e, bool two_sided)
        : bounds(b)
        , direction(normalize(direction))
        , phi(phi)
        , cos_theta_o(cos_theta_o)
        , cos_theta_e(cos_theta_e)
        , two_sided(two_sided)
    {}

    inline LightBounds bounds_union(const LightBounds& a, const LightBounds& b)
    {
        // If one _LightBounds_ has zero power, return the other
        if (a.phi == 0)
        {
            return b;
        }
        if (b.phi == 0)
        {
            return a;
        }

        // Find average direction and updated angles for _LightBounds_
        DirectionCone cone =
            bounds_union(
                DirectionCone(a.direction, a.cos_theta_o), 
                DirectionCone(b.direction, b.cos_theta_o)
            );
        Float cos_theta_o = cone.cos_theta;
        Float cos_theta_e = std::min(a.cos_theta_e, b.cos_theta_e);

        // Return final _LightBounds_ union
        return LightBounds(bounds_union(a.bounds, b.bounds), cone.direction, 
            a.phi + b.phi, cos_theta_o, cos_theta_e, a.two_sided | b.two_sided);
    }

    class LightBase
    {
    public:
        LightBase(LightType type, const Transform& render_from_light,
            const MediumInterface& medium_interface);

        LOQUAT_CPU_GPU
        LightType get_type() const { return type; }

        LOQUAT_CPU_GPU
        SampledSpectrum radiance_reflected_back(Point3f p, Normal3f normal,
            Point2f uv, Vec3f direction, const SampledWavelengths& lambda) const
        {
            return SampledSpectrum(0.0f);
        }

        LOQUAT_CPU_GPU
        SampledSpectrum infinite_light_contribution(const Ray&,
            const SampledWavelengths&) const
        {
            return SampledSpectrum(0.0f);
        }

    protected:
        static const DenselySampledSpectrum* lookup_spectrum(Spectrum s);

        std::string base_to_string() const;

        LightType type;
        Transform render_from_light;
        MediumInterface medium_interface;
        static InternCache<DenselySampledSpectrum>* spectrum_cache;
    };

    class PointLight : public LightBase
    {
    public:
        PointLight(Transform render_from_light,
            MediumInterface medium_interface, Spectrum intensity,
            Float scale)
            : LightBase(LightType::DeltaPosition, render_from_light,
                medium_interface)
            , intensity(lookup_spectrum(intensity))
            , scale(scale) {}

        static PointLight* create(const Transform& render_from_light, Medium medium,
            const ParameterDictionary& parameters,
            const RGBColorSpace* color_space, const FileLoc* loc,
            Allocator alloc);

        SampledSpectrum total_emitted_power(SampledWavelengths lambda) const;

        void preprocess(const AABB3f& scene_bounds) {}

        LOQUAT_CPU_GPU
        pstd::optional<LightLeSample> sample_emissive(Point2f sample1, Point2f sample2,
            SampledWavelengths& lambda, Float time) const;
        LOQUAT_CPU_GPU
        void get_PDFs(const Ray&, Float* pdf_pos, Float* pdf_dir) const;

        LOQUAT_CPU_GPU
        void get_PDFs(const Interaction&, Vec3f direction, Float* pdf_pos,
            Float* pdf_dir) const
        {
            LOG_FATAL("Shouldn't be called for non-area lights");
        }

        pstd::optional<LightBounds> bounds() const;

        std::string to_string() const;

        LOQUAT_CPU_GPU
        pstd::optional<LightIncidentSample> sample_light_incoming(
            LightSampleContext context, Point2f sample,
            SampledWavelengths lambda,
            bool allow_incomplete_PDF) const
        {
            Point3f p = render_from_light(Point3f(0, 0, 0));
            Vec3f incoming = normalize(p - context.p());
            SampledSpectrum Li = scale * intensity->sample(lambda) 
                / distance_squared(p, context.p());
            return LightIncidentSample(Li, incoming, 1, 
                Interaction(p, &medium_interface));
        }

        LOQUAT_CPU_GPU
        Float sample_PDF(LightSampleContext, Vec3f, 
            bool allow_incomplete_PDF) const
        {
            return 0;
        }

    private:
        const DenselySampledSpectrum* intensity;
        Float scale;
    };

    class DistantLight : public LightBase
    {
    public:
        DistantLight(const Transform& render_from_light, Spectrum emitted,
            Float scale)
            : LightBase(LightType::DeltaDirection, render_from_light, {}),
            emitted(lookup_spectrum(emitted)),
            scale(scale) {}

        static DistantLight* create(const Transform& render_from_light,
            const ParameterDictionary& parameters,
            const RGBColorSpace* color_space, const FileLoc* loc,
            Allocator alloc);

        SampledSpectrum total_emitted_power(SampledWavelengths lambda) const;

        LOQUAT_CPU_GPU
        Float sample_PDF(LightSampleContext, Vec3f,
            bool allow_incomplete_PDF) const
        {
            return 0;
        }

        LOQUAT_CPU_GPU
        pstd::optional<LightLeSample> sample_emissive(Point2f sample1,
            Point2f sample2,  SampledWavelengths& lambda, Float time) const;

        LOQUAT_CPU_GPU
        void get_PDFs(const Ray&, Float* pdf_pos, Float* pdf_dir) const;

        LOQUAT_CPU_GPU
        void get_PDFs(const Interaction&, Vec3f direction, Float* pdf_pos,
            Float* pdf_dir) const
        {
            LOG_FATAL("Shouldn't be called for non-area lights");
        }

        pstd::optional<LightBounds> bounds() const { return {}; }

        std::string to_string() const;

        void preprocess(const AABB3f& scene_bounds) {
            scene_bounds.bounding_sphere(&scene_center, &scene_radius);
        }

        LOQUAT_CPU_GPU
        pstd::optional<LightIncidentSample> sample_light_incoming(LightSampleContext context, Point2f sample,
            SampledWavelengths lambda,
            bool allow_incomplete_PDF) const
        {
            Vec3f incoming = normalize(render_from_light(Vec3f(0, 0, 1)));
            Point3f pOutside = context.p() + incoming * (2 * scene_radius);
            return LightIncidentSample(scale * emitted->sample(lambda), 
                incoming, 1, Interaction(pOutside, nullptr));
        }

    private:
        const DenselySampledSpectrum* emitted;
        Float scale;
        Point3f scene_center;
        Float scene_radius;
    };

    class ProjectionLight : public LightBase
    {
    public:
        ProjectionLight(Transform render_from_light, MediumInterface medium,
            Image image, const RGBColorSpace* color_space, Float scale,
            Float fov, Allocator alloc);

        static ProjectionLight* create(const Transform& render_from_light,
            Medium medium,
            const ParameterDictionary& parameters,
            const FileLoc* loc, Allocator alloc);

        void preprocess(const AABB3f& scene_bounds)
        {}

        LOQUAT_CPU_GPU
        pstd::optional<LightIncidentSample> sample_light_incoming(
            LightSampleContext context, Point2f sample,
            SampledWavelengths lambda,
            bool allow_incomplete_PDF) const;

        LOQUAT_CPU_GPU
        SampledSpectrum intensity(Vec3f direction,
            const SampledWavelengths& lambda) const;

        SampledSpectrum total_emitted_power(SampledWavelengths lambda) const;

        LOQUAT_CPU_GPU
        Float sample_PDF(LightSampleContext, Vec3f,
            bool allow_incomplete_PDF) const;

        LOQUAT_CPU_GPU
        pstd::optional<LightLeSample> sample_emissive(Point2f sample1,
            Point2f sample2,
            SampledWavelengths& lambda, Float time) const;
        LOQUAT_CPU_GPU
        void get_PDFs(const Ray&, Float* pdf_pos, Float* pdf_dir) const;

        LOQUAT_CPU_GPU
        void get_PDFs(const Interaction&, Vec3f direction, Float* pdf_pos,
            Float* pdf_dir) const
        {
            LOG_FATAL("Shouldn't be called for non-area lights");
        }

        pstd::optional<LightBounds> bounds() const;

        std::string to_string() const;

    private:
        Image image;
        const RGBColorSpace* image_color_space;
        Float scale;
        AABB2f screen_bounds;
        Float hither = 1e-3f;
        Transform screen_from_light;
        Transform light_from_screen;
        Float A;
        PiecewiseConstant2D distrib;
    };

    class GoniometricLight : public LightBase
    {
    public:
        GoniometricLight(const Transform& render_from_light,
            const MediumInterface& medium_interface, Spectrum intensity,
            Float scale,
            Image image, Allocator alloc);

        static GoniometricLight* create(const Transform& render_from_light,
            Medium medium,
            const ParameterDictionary& parameters,
            const RGBColorSpace* color_space, const FileLoc* loc,
            Allocator alloc);

        void preprocess(const AABB3f& scene_bounds) {}

        LOQUAT_CPU_GPU
        pstd::optional<LightIncidentSample> sample_light_incoming(
            LightSampleContext context, Point2f sample,
            SampledWavelengths lambda,
            bool allow_incomplete_PDF) const;

        SampledSpectrum total_emitted_power(SampledWavelengths lambda) const;

        LOQUAT_CPU_GPU
            Float sample_PDF(LightSampleContext, Vec3f,
                bool allow_incomplete_PDF) const;

        LOQUAT_CPU_GPU
        pstd::optional<LightLeSample> sample_emissive(Point2f sample1,
            Point2f sample2, SampledWavelengths& lambda, Float time) const;
        LOQUAT_CPU_GPU
        void get_PDFs(const Ray&, Float* pdf_pos, Float* pdf_dir) const;

        LOQUAT_CPU_GPU
        void get_PDFs(const Interaction&, Vec3f direction, Float* pdf_pos,
            Float* pdf_dir) const
        {
            LOG_FATAL("Shouldn't be called for non-area lights");
        }

        pstd::optional<LightBounds> bounds() const;

        std::string to_string() const;

        LOQUAT_CPU_GPU
        SampledSpectrum intensity(Vec3f direction,
            const SampledWavelengths& lambda) const
        {
            Point2f uv = equal_area_sphere_to_square(direction);
            return scale * emitted->sample(lambda) 
                * image.lookup_nearest_channel(uv, 0);
        }

    private:
        const DenselySampledSpectrum* emitted;
        Float scale;
        Image image;
        PiecewiseConstant2D distrib;
    };

    class DiffuseAreaLight : public LightBase
    {
    public:
        DiffuseAreaLight(const Transform& render_from_light,
            const MediumInterface& medium_interface,
            Spectrum infinite_light_contribution, Float scale,
            const Shape shape, FloatTexture alpha, Image image,
            const RGBColorSpace* image_color_space, bool two_sided);

        static DiffuseAreaLight* create(const Transform& render_from_light,
            Medium medium, const ParameterDictionary& parameters,
            const RGBColorSpace* color_space, const FileLoc* loc,
            Allocator alloc, const Shape shape,
            FloatTexture alpha);

        void preprocess(const AABB3f& scene_bounds)
        {}

        SampledSpectrum total_emitted_power(SampledWavelengths lambda) const;

        LOQUAT_CPU_GPU
        pstd::optional<LightLeSample> sample_emissive(Point2f sample1,
            Point2f sample2,
            SampledWavelengths& lambda, Float time) const;
        LOQUAT_CPU_GPU
        void get_PDFs(const Interaction&, Vec3f direction, Float* pdf_pos,
            Float* pdf_dir) const;

        pstd::optional<LightBounds> bounds() const;

        LOQUAT_CPU_GPU
        void get_PDFs(const Ray&, Float* pdf_pos, Float* pdf_dir) const
        {
            LOG_FATAL("Shouldn't be called for area lights");
        }

        std::string to_string() const;

        LOQUAT_CPU_GPU
        SampledSpectrum radiance_reflected_back(Point3f p, Normal3f normal,
            Point2f uv, Vec3f direction, const SampledWavelengths& lambda) const
        {
            // Check for zero emitted radiance from point on area light
            if (!two_sided && dot(normal, direction) < 0)
            {
                return SampledSpectrum(0.0f);
            }
            if (AlphaMasked(Interaction(p, uv)))
            {
                return SampledSpectrum(0.0f);
            }

            if (image)
            {
                // Return _DiffuseAreaLight_ emission using image
                RGB rgb;
                uv[1] = 1 - uv[1];
                for (int c = 0; c < 3; ++c)
                {
                    rgb[c] = image.bilerp_channel(uv, c);
                }
                RGBIlluminantSpectrum spec(*image_color_space, clamp_zero(rgb));
                return scale * spec.sample(lambda);

            }
            else
            {
                return scale * emitted->sample(lambda);
            }
        }

        LOQUAT_CPU_GPU
        pstd::optional<LightIncidentSample> sample_light_incoming(
            LightSampleContext context, Point2f sample,
            SampledWavelengths lambda,
            bool allow_incomplete_PDF) const;

        LOQUAT_CPU_GPU
        Float sample_PDF(LightSampleContext context, Vec3f incoming,
            bool allow_incomplete_PDF) const;

    private:
        Shape shape;
        FloatTexture alpha;
        Float area;
        bool two_sided;
        const DenselySampledSpectrum* emitted;
        Float scale;
        Image image;
        const RGBColorSpace* image_color_space;

        // DiffuseAreaLight Private Methods
        LOQUAT_CPU_GPU
        bool AlphaMasked(const Interaction& intr) const
        {
            if (!alpha)
            {
                return false;
            }
#ifdef LOQUAT_IS_GPU_CODE
            Float a = BasicTextureEvaluator()(alpha, intr);
#else
            Float a = UniversalTextureEvaluator()(alpha, intr);
#endif  // LOQUAT_IS_GPU_CODE
            if (a >= 1)
            {
                return false;
            }
            if (a <= 0)
            {
                return true;
            }
            return hash_float(intr.p()) > a;
        }
    };

    class UniformInfiniteLight : public LightBase
    {
    public:
        UniformInfiniteLight(const Transform& render_from_light,
            Spectrum emitted, Float scale);

        void preprocess(const AABB3f& scene_bounds)
        {
            scene_bounds.bounding_sphere(&scene_center, &scene_radius);
        }

        SampledSpectrum total_emitted_power(SampledWavelengths lambda) const;

        LOQUAT_CPU_GPU
            SampledSpectrum infinite_light_contribution(const Ray& ray,
                const SampledWavelengths& lambda) const;
        LOQUAT_CPU_GPU
            pstd::optional<LightIncidentSample> sample_light_incoming(
                LightSampleContext context, Point2f sample,
                SampledWavelengths lambda,
                bool allow_incomplete_PDF) const;
        LOQUAT_CPU_GPU
            Float sample_PDF(LightSampleContext, Vec3f,
                bool allow_incomplete_PDF) const;

        LOQUAT_CPU_GPU
            pstd::optional<LightLeSample> sample_emissive(Point2f sample1,
                Point2f sample2, SampledWavelengths& lambda, Float time) const;
        LOQUAT_CPU_GPU
            void get_PDFs(const Ray&, Float* pdf_pos, Float* pdf_dir) const;

        LOQUAT_CPU_GPU
            void get_PDFs(const Interaction&, Vec3f direction, Float* pdf_pos,
                Float* pdf_dir) const
        {
            LOG_FATAL("Shouldn't be called for non-area lights");
        }

        pstd::optional<LightBounds> bounds() const
        {
            return {};
        }

        std::string to_string() const;

    private:
        const DenselySampledSpectrum* emitted;
        Float scale;
        Point3f scene_center;
        Float scene_radius;
    };

    class ImageInfiniteLight : public LightBase
    {
    public:
        ImageInfiniteLight(Transform render_from_light, Image image,
            const RGBColorSpace* image_color_space, Float scale,
            std::string filename, Allocator alloc);

        void preprocess(const AABB3f& scene_bounds)
        {
            scene_bounds.bounding_sphere(&scene_center, &scene_radius);
        }

        SampledSpectrum total_emitted_power(SampledWavelengths lambda) const;

        LOQUAT_CPU_GPU
        Float sample_PDF(LightSampleContext, Vec3f,
            bool allow_incomplete_PDF) const;

        LOQUAT_CPU_GPU
        pstd::optional<LightLeSample> sample_emissive(Point2f sample1,
            Point2f sample2,
            SampledWavelengths& lambda, Float time) const;
        LOQUAT_CPU_GPU
        void get_PDFs(const Ray&, Float* pdf_pos, Float* pdf_dir) const;

        LOQUAT_CPU_GPU
        void get_PDFs(const Interaction&, Vec3f direction, Float* pdf_pos,
            Float* pdf_dir) const
        {
            LOG_FATAL("Shouldn't be called for non-area lights");
        }

        std::string to_string() const;

        LOQUAT_CPU_GPU
        SampledSpectrum infinite_light_contribution(const Ray& ray,
            const SampledWavelengths& lambda) const
        {
            Vec3f wLight = normalize(render_from_light.apply_inverse(ray.direction));
            Point2f uv = equal_area_sphere_to_square(wLight);
            return ImageLe(uv, lambda);
        }

        LOQUAT_CPU_GPU
        pstd::optional<LightIncidentSample> sample_light_incoming(
            LightSampleContext context, Point2f sample,
            SampledWavelengths lambda, bool allow_incomplete_PDF) const
        {
            // Find $(sample,v)$ sample coordinates in infinite light texture
            Float mapPDF = 0;
            Point2f uv;
            if (allow_incomplete_PDF)
                uv = compensatedDistribution.sample(sample, &mapPDF);
            else
                uv = distribution.sample(sample, &mapPDF);
            if (mapPDF == 0)
                return {};

            // Convert infinite light sample point to direction
            Vec3f wLight = equal_area_square_to_sphere(uv);
            Vec3f incoming = render_from_light(wLight);

            // Compute PDF for sampled infinite light direction
            Float pdf = mapPDF / (4 * PI);

            // Return radiance value for infinite light direction
            return LightIncidentSample(
                ImageLe(uv, lambda), incoming, pdf,
                Interaction(context.p() + incoming * (2 * scene_radius), 
                    &medium_interface));
        }

        pstd::optional<LightBounds> bounds() const
        {
            return {};
        }

    private:
        LOQUAT_CPU_GPU
        SampledSpectrum ImageLe(Point2f uv,
            const SampledWavelengths& lambda) const
        {
            RGB rgb;
            for (int c = 0; c < 3; ++c)
            {
                rgb[c] = image.lookup_nearest_channel(uv, c, 
                    WrapMode::OctahedralSphere);
            }
            RGBIlluminantSpectrum spec(*image_color_space, clamp_zero(rgb));
            return scale * spec.sample(lambda);
        }

        Image image;
        const RGBColorSpace* image_color_space;
        Float scale;
        Point3f scene_center;
        Float scene_radius;
        PiecewiseConstant2D distribution;
        PiecewiseConstant2D compensatedDistribution;
    };

    class PortalImageInfiniteLight : public LightBase
    {
    public:
        PortalImageInfiniteLight(const Transform& render_from_light, Image image,
            const RGBColorSpace* image_color_space, Float scale,
            const std::string& filename, std::vector<Point3f> portal,
            Allocator alloc);

        void preprocess(const AABB3f& scene_bounds)
        {
            scene_bounds.bounding_sphere(&scene_center, &scene_radius);
        }

        SampledSpectrum total_emitted_power(SampledWavelengths lambda) const;

        LOQUAT_CPU_GPU
        SampledSpectrum infinite_light_contribution(const Ray& ray,
            const SampledWavelengths& lambda) const;

        LOQUAT_CPU_GPU
        pstd::optional<LightIncidentSample> sample_light_incoming(
            LightSampleContext context, Point2f sample,
            SampledWavelengths lambda,
            bool allow_incomplete_PDF) const;

        LOQUAT_CPU_GPU
        Float sample_PDF(LightSampleContext, Vec3f,
            bool allow_incomplete_PDF) const;

        LOQUAT_CPU_GPU
        pstd::optional<LightLeSample> sample_emissive(Point2f sample1,
            Point2f sample2,
            SampledWavelengths& lambda, Float time) const;
        LOQUAT_CPU_GPU
        void get_PDFs(const Ray&, Float* pdf_pos, Float* pdf_dir) const;

        LOQUAT_CPU_GPU
        void get_PDFs(const Interaction&, Vec3f direction, Float* pdf_pos,
            Float* pdf_dir) const
        {
            LOG_FATAL("Shouldn't be called for non-area lights");
        }

        pstd::optional<LightBounds> bounds() const { return {}; }

        std::string to_string() const;

    private:
        LOQUAT_CPU_GPU
        SampledSpectrum image_lookup(Point2f uv,
            const SampledWavelengths& lambda) const;

        LOQUAT_CPU_GPU
        pstd::optional<Point2f> image_from_render(Vec3f wRender,
            Float* duv_dw = nullptr) const
        {
            Vec3f direction = portal_frame.to_local(wRender);
            if (direction.z <= 0)
            {
                return {};
            }
            // Compute Jacobian determinant of mapping 
            // $\roman{d}(sample,v)/\roman{d}\omega$ if needed
            if (duv_dw)
            {
                *duv_dw = square(PI) 
                    * (1 - square(direction.x)) 
                    * (1 - square(direction.y)) 
                    / direction.z;
            }

            Float alpha = std::atan2(direction.x, direction.z);
            Float beta = std::atan2(direction.y, direction.z);
            LOG_ASSERT(!is_NaN(alpha + beta));
            return Point2f(clamp((alpha + PI / 2) / PI, 0, 1),
                clamp((beta + PI / 2) / PI, 0, 1));
        }

        LOQUAT_CPU_GPU
        Vec3f render_from_image(Point2f uv, Float* duv_dw = nullptr) const
        {
            Float alpha = -PI / 2 + uv[0] * PI, beta = -PI / 2 + uv[1] * PI;
            Float x = std::tan(alpha), y = std::tan(beta);
            LOG_ASSERT(!is_inf(x) && !is_inf(y));
            Vec3f direction = normalize(Vec3f(x, y, 1));
            // Compute Jacobian determinant of mapping $\roman{d}(sample,v)/\roman{d}\omega$ if
            // needed
            if (duv_dw)
            {
                *duv_dw = square(PI) 
                    * (1 - square(direction.x)) 
                    * (1 - square(direction.y)) 
                    / direction.z;
            }

            return portal_frame.from_local(direction);
        }

        LOQUAT_CPU_GPU
        pstd::optional<AABB2f> image_bounds(Point3f p) const
        {
            pstd::optional<Point2f> p0 = image_from_render(normalize(portal[0] - p));
            pstd::optional<Point2f> p1 = image_from_render(normalize(portal[2] - p));
            if (!p0 || !p1)
                return {};
            return AABB2f(*p0, *p1);
        }

        LOQUAT_CPU_GPU
        Float area() const
        {
            return length(portal[1] - portal[0]) * length(portal[3] - portal[0]);
        }

        pstd::array<Point3f, 4> portal;
        Frame portal_frame;
        Image image;
        WindowedPiecewiseConstant2D distribution;
        const RGBColorSpace* image_color_space;
        Float scale;
        Float scene_radius;
        std::string filename;
        Point3f scene_center;
    };

    class SpotLight : public LightBase
    {
    public:
        SpotLight(const Transform& render_from_light, const MediumInterface& m,
            Spectrum intensity, Float scale, Float totalWidth,
            Float falloff_start);

        static SpotLight* create(const Transform& render_from_light,
            Medium medium, const ParameterDictionary& parameters,
            const RGBColorSpace* color_space, const FileLoc* loc,
            Allocator alloc);

        void preprocess(const AABB3f& scene_bounds)
        {}

        LOQUAT_CPU_GPU
        SampledSpectrum intensity(Vec3f direction, SampledWavelengths) const;

        SampledSpectrum total_emitted_power(SampledWavelengths lambda) const;

        LOQUAT_CPU_GPU
        Float sample_PDF(LightSampleContext, Vec3f,
            bool allow_incomplete_PDF) const;

        LOQUAT_CPU_GPU
        pstd::optional<LightLeSample> sample_emissive(Point2f sample1,
            Point2f sample2, SampledWavelengths& lambda, Float time) const;
        LOQUAT_CPU_GPU
        void get_PDFs(const Ray&, Float* pdf_pos, Float* pdf_dir) const;

        LOQUAT_CPU_GPU
        void get_PDFs(const Interaction&, Vec3f direction, Float* pdf_pos,
            Float* pdf_dir) const
        {
            LOG_FATAL("Shouldn't be called for non-area lights");
        }

        pstd::optional<LightBounds> bounds() const;

        std::string to_string() const;

        LOQUAT_CPU_GPU
        pstd::optional<LightIncidentSample> sample_light_incoming(
            LightSampleContext context, Point2f sample,
            SampledWavelengths lambda,
            bool allow_incomplete_PDF) const
        {
            Point3f p = render_from_light(Point3f(0, 0, 0));
            Vec3f incoming = normalize(p - context.p());
            // Compute incident radiance _Li_ for _SpotLight_
            Vec3f wLight = normalize(render_from_light.apply_inverse(-incoming));
            SampledSpectrum Li = intensity(wLight, lambda)
                / distance_squared(p, context.p());

            if (!Li)
            {
                return {};
            }
            return LightIncidentSample(Li, incoming, 1,
                Interaction(p, &medium_interface));
        }

    private:
        const DenselySampledSpectrum* emitted;
        Float scale;
        Float cos_falloff_start;
        Float cos_falloff_end;
    };

    LOQUAT_CPU_GPU
    inline pstd::optional<LightIncidentSample> Light::sample_light_incoming(
        LightSampleContext context, Point2f sample,
        SampledWavelengths lambda,
        bool allow_incomplete_PDF) const
    {
        auto sample = [&](auto ptr) {
            return ptr->sample_light_incoming(context, sample, lambda, allow_incomplete_PDF);
            };
        return dispatch(sample);
    }

    LOQUAT_CPU_GPU
    inline Float Light::sample_PDF(LightSampleContext context, Vec3f incoming,
        bool allow_incomplete_PDF) const
    {
        auto pdf = [&](auto ptr) { return ptr->sample_PDF(context, incoming, allow_incomplete_PDF); };
        return dispatch(pdf);
    }

    LOQUAT_CPU_GPU
    inline SampledSpectrum Light::radiance_reflected_back(Point3f p, Normal3f normal, Point2f uv, Vec3f direction,
        const SampledWavelengths& lambda) const
    {
        LOG_ASSERT(get_type() == LightType::Area);
        auto l = [&](auto ptr) { return ptr->radiance_reflected_back(p, normal, uv, direction, lambda); };
        return dispatch(l);
    }

    LOQUAT_CPU_GPU
    inline SampledSpectrum Light::infinite_light_contribution(const Ray& ray,
        const SampledWavelengths& lambda) const
    {
        auto le = [&](auto ptr) { return ptr->infinite_light_contribution(ray, lambda); };
        return dispatch(le);
    }

    LOQUAT_CPU_GPU
    inline LightType Light::get_type() const
    {
        auto t = [&](auto ptr) { return ptr->get_type(); };
        return dispatch(t);
    }

}

namespace std
{
    template <>
    struct hash<loquat::Light>
    {
        LOQUAT_CPU_GPU
        size_t operator()(loquat::Light light) const noexcept
        {
            return loquat::hash(light.pointer());
        }
    };

}