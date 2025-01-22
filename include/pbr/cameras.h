// pbrt is Copyright(c) 1998-2020 Matt Pharr, Wenzel Jakob, and Greg Humphreys.
// The pbrt source code is licensed under the Apache License, Version 2.0.
// SPDX: Apache-2.0

// This file has been modified from the original, original notice is above.

#pragma once

#include <algorithm>
#include <memory>
#include <string>
#include <vector>

#include "main/loquat.h"

#include "pbr/films.h"
#include "pbr/samplers.h"
#include "pbr/base/camera.h"
#include "pbr/base/film.h"
#include "pbr/math/ray.h"
#include "pbr/struct/interaction.h"
#include "pbr/struct/image.h"
#include "pbr/util/scattering.h"

namespace loquat
{
    class CameraTransform
    {
    public:
        CameraTransform() = default;
        explicit CameraTransform(const AnimatedTransform& world_from_camera);

        LOQUAT_CPU_GPU
        Point3f get_render_from_camera(Point3f p, Float time) const
        {
            return get_render_from_camera(p, time);
        }

        LOQUAT_CPU_GPU
        Point3f camera_from_render(Point3f p, Float time) const
        {
            return render_from_camera.apply_inverse(p, time);
        }

        LOQUAT_CPU_GPU
        Point3f render_from_world(Point3f p) const
        {
            return world_from_render.apply_inverse(p);
        }

        LOQUAT_CPU_GPU
        Transform render_from_world() const
        {
            return inverse(world_from_render);
        }

        LOQUAT_CPU_GPU
        Transform camera_from_render(Float time) const
        {
            return inverse(render_from_camera.interpolate(time));
        }

        LOQUAT_CPU_GPU
            Transform camera_from_world(Float time) const
        {
            return inverse(world_from_render * render_from_camera.interpolate(time));
        }

        LOQUAT_CPU_GPU
        bool camera_from_render_has_scale() const
        {
            return render_from_camera.has_scale();
        }

        LOQUAT_CPU_GPU
        Vec3f get_render_from_camera(Vec3f v, Float time) const
        {
            return render_from_camera(v, time);
        }

        LOQUAT_CPU_GPU
        Normal3f get_render_from_camera(Normal3f n, Float time) const
        {
            return render_from_camera(n, time);
        }

        LOQUAT_CPU_GPU
        Ray get_render_from_camera(const Ray& r) const { return render_from_camera(r); }

        LOQUAT_CPU_GPU
            RayDifferential get_render_from_camera(const RayDifferential& r) const {
            return get_render_from_camera(r);
        }

        LOQUAT_CPU_GPU
        Vec3f camera_from_render(Vec3f v, Float time) const {
            return render_from_camera.apply_inverse(v, time);
        }

        LOQUAT_CPU_GPU
        Normal3f camera_from_render(Normal3f v, Float time) const
        {
            return render_from_camera.apply_inverse(v, time);
        }

        LOQUAT_CPU_GPU
        const AnimatedTransform& get_render_from_camera() const { return render_from_camera; }

        LOQUAT_CPU_GPU
        const Transform& get_world_from_render() const { return world_from_render; }

        std::string to_string() const;

    private:
        AnimatedTransform render_from_camera;
        Transform world_from_render;
    };

    struct CameraWiSample
    {
        CameraWiSample() = default;
        LOQUAT_CPU_GPU
        CameraWiSample(const SampledSpectrum& incoming_spectrum, const Vec3f& incoming,
            Float pdf,
            Point2f p_raster, const Interaction& p_ref, const Interaction& p_lens)
            : incoming_spectrum(incoming_spectrum)
            , incoming(incoming)
            , pdf(pdf)
            , p_raster(p_raster)
            , p_ref(p_ref)
            , p_lens(p_lens)
        {}

        SampledSpectrum incoming_spectrum;
        Vec3f incoming;
        Float pdf;
        Point2f p_raster;
        Interaction p_ref;
        Interaction p_lens;
    };

    struct CameraRay
    {
        Ray ray;
        SampledSpectrum weight = SampledSpectrum(1);
    };

    struct CameraRayDifferential {
        RayDifferential ray;
        SampledSpectrum weight = SampledSpectrum(1);
    };

    struct CameraBaseParameters
    {
        CameraTransform camera_transform;
        Float shutter_open = 0;
        Float shutter_close = 1;
        Film film;
        Medium medium;
        CameraBaseParameters() = default;
        CameraBaseParameters(const CameraTransform& camera_transform,
            Film film, Medium medium, const ParameterDictionary& parameters,
            const FileLoc* loc);
    };

    class CameraBase
    {
    public:
        LOQUAT_CPU_GPU
        Film get_film() const
        {
            return film;
        }
        LOQUAT_CPU_GPU
        const CameraTransform& get_camera_transform() const
        {
            return camera_transform;
        }

        LOQUAT_CPU_GPU
        Float sample_time(Float u) const
        {
            return lerp(u, shutter_open, shutter_close);
        }

        void init_metadata(ImageMetadata* metadata) const;
        std::string to_string() const;

        LOQUAT_CPU_GPU
        void approximate_dp_dxy(Point3f p, Normal3f n, Float time,
            int samples_per_pixel, Vec3f* dpdx, Vec3f* dpdy) const
        {
            // Compute tangent plane equation for ray differential intersections
            Point3f pCamera = camera_from_render(p, time);
            Transform down_z_from_camera =
                rotate_from_to(normalize(Vec3f(pCamera)), Vec3f(0, 0, 1));
            Point3f pDownZ = down_z_from_camera(pCamera);
            Normal3f nDownZ = down_z_from_camera(camera_from_render(n, time));
            Float d = nDownZ.z * pDownZ.z;

            // Find intersection points for approximated camera differential rays
            Ray xRay(Point3f(0, 0, 0) + minPosDifferentialX,
                Vec3f(0, 0, 1) + minDirDifferentialX);
            Float tx = -(dot(nDownZ, Vec3f(xRay.origin)) - d) / dot(nDownZ, xRay.direction);
            Ray yRay(Point3f(0, 0, 0) + minPosDifferentialY,
                Vec3f(0, 0, 1) + minDirDifferentialY);
            Float ty = -(dot(nDownZ, Vec3f(yRay.origin)) - d) / dot(nDownZ, yRay.direction);
            Point3f px = xRay(tx), py = yRay(ty);

            // Estimate $\dpdx$ and $\dpdy$ in tangent plane at intersection point
            Float sppScale =
                get_options().disable_pixel_jitter
                ? 1
                : std::max<Float>(.125, 1 / std::sqrt((Float)samples_per_pixel));
            *dpdx =
                sppScale * render_from_camera(down_z_from_camera.apply_inverse(px - pDownZ), time);
            *dpdy =
                sppScale * render_from_camera(down_z_from_camera.apply_inverse(py - pDownZ), time);
        }

    protected:
        CameraTransform camera_transform;
        Float shutter_open;
        Float shutter_close;
        Film film;
        Medium medium;
        Vec3f minPosDifferentialX;
        Vec3f minPosDifferentialY;
        Vec3f minDirDifferentialX;
        Vec3f minDirDifferentialY;

        CameraBase() = default;
        CameraBase(CameraBaseParameters p);

        LOQUAT_CPU_GPU
        static pstd::optional<CameraRayDifferential> generate_ray_differential(
            Camera camera, CameraSample sample, SampledWavelengths& lambda);

        LOQUAT_CPU_GPU
        Ray render_from_camera(const Ray& r) const
        {
            return camera_transform.get_render_from_camera(r);
        }

        LOQUAT_CPU_GPU
        RayDifferential render_from_camera(const RayDifferential& r) const
        {
            return camera_transform.get_render_from_camera(r);
        }

        LOQUAT_CPU_GPU
        Vec3f render_from_camera(Vec3f v, Float time) const
        {
            return camera_transform.get_render_from_camera(v, time);
        }

        LOQUAT_CPU_GPU
        Normal3f render_from_camera(Normal3f v, Float time) const
        {
            return camera_transform.get_render_from_camera(v, time);
        }

        LOQUAT_CPU_GPU
        Point3f render_from_camera(Point3f p, Float time) const
        {
            return camera_transform.get_render_from_camera(p, time);
        }

        LOQUAT_CPU_GPU
        Vec3f camera_from_render(Vec3f v, Float time) const
        {
            return camera_transform.camera_from_render(v, time);
        }

        LOQUAT_CPU_GPU
        Normal3f camera_from_render(Normal3f v, Float time) const
        {
            return camera_transform.camera_from_render(v, time);
        }

        LOQUAT_CPU_GPU
        Point3f camera_from_render(Point3f p, Float time) const
        {
            return camera_transform.camera_from_render(p, time);
        }

        void find_minimum_differentials(Camera camera);
    };

    class ProjectiveCamera : public CameraBase
    {
    public:
        ProjectiveCamera() = default;
        void init_metadata(ImageMetadata* metadata) const;

        std::string base_to_string() const;

        ProjectiveCamera(CameraBaseParameters base_parameters,
            const Transform& screen_from_camera, AABB2f screen_window,
            Float lens_radius, Float focal_distance)
            : CameraBase(base_parameters)
            , screen_from_camera(screen_from_camera)
            , lens_radius(lens_radius)
            , focal_distance(focal_distance)
        {
            // Compute projective camera transformations
            // Compute projective camera screen transformations
            Transform NDCFromScreen =
                scale(1 / (screen_window.max.x - screen_window.min.x),
                    1 / (screen_window.max.y - screen_window.min.y), 1) *
                translate(Vec3f(-screen_window.min.x, -screen_window.max.y, 0));
            Transform rasterFromNDC =
                scale(film.get_full_resolution().x, -film.get_full_resolution().y, 1);
            raster_from_screen = rasterFromNDC * NDCFromScreen;
            screen_from_raster = inverse(raster_from_screen);

            camera_from_raster = inverse(screen_from_camera) * screen_from_raster;
        }

    protected:
        Transform screen_from_camera;
        Transform camera_from_raster;
        Transform raster_from_screen;
        Transform screen_from_raster;
        Float lens_radius;
        Float focal_distance;
    };

    class OrthographicCamera : public ProjectiveCamera
    {
    public:
        OrthographicCamera(CameraBaseParameters base_parameters,
            AABB2f screen_window, Float lens_radius, Float focalDist)
            : ProjectiveCamera(base_parameters, orthographic(0, 1),
                screen_window, lens_radius, focalDist)
        {
            // Compute differential changes in origin for orthographic camera rays
            dx_camera = camera_from_raster(Vec3f(1, 0, 0));
            dy_camera = camera_from_raster(Vec3f(0, 1, 0));

            // Compute minimum differentials for orthographic camera
            minDirDifferentialX = minDirDifferentialY = Vec3f(0, 0, 0);
            minPosDifferentialX = dx_camera;
            minPosDifferentialY = dy_camera;
        }

        LOQUAT_CPU_GPU
        pstd::optional<CameraRay> generate_ray(CameraSample sample,
            SampledWavelengths& lambda) const;

        LOQUAT_CPU_GPU
        pstd::optional<CameraRayDifferential> generate_ray_differential(
            CameraSample sample, SampledWavelengths& lambda) const;

        static OrthographicCamera* create(const ParameterDictionary& parameters,
            const CameraTransform& camera_transform, Film film,
            Medium medium, const FileLoc* loc,
            Allocator alloc = {});

        LOQUAT_CPU_GPU
        SampledSpectrum importance(const Ray& ray, SampledWavelengths& lambda,
            Point2f* pRaster2 = nullptr) const
        {
            LOG_FATAL("importance() unimplemented for OrthographicCamera");
            return {};
        }

        LOQUAT_CPU_GPU
        void importance_PDF(const Ray& ray, Float* pdf_position,
            Float* pdf_direction) const
        {
            LOG_FATAL("importance_PDF() unimplemented for OrthographicCamera");
        }

        LOQUAT_CPU_GPU
        pstd::optional<CameraWiSample> sample_light_incoming(
            const Interaction& ref, Point2f u, SampledWavelengths& lambda) const
        {
            LOG_FATAL("sample_light_incoming() unimplemented for OrthographicCamera");
            return {};
        }

        std::string to_string() const;

    private:
        Vec3f dx_camera;
        Vec3f dy_camera;
    };

    // PerspectiveCamera Definition
    class PerspectiveCamera : public ProjectiveCamera
    {
    public:
        // PerspectiveCamera Public Methods
        PerspectiveCamera(CameraBaseParameters base_parameters, Float fov,
            AABB2f screen_window, Float lens_radius, Float focalDist)
            : ProjectiveCamera(base_parameters, perspective(fov, 1e-2f, 1000.f), screen_window,
                lens_radius, focalDist) {
            // Compute differential changes in origin for perspective camera rays
            dx_camera =
                camera_from_raster(Point3f(1, 0, 0)) - camera_from_raster(Point3f(0, 0, 0));
            dy_camera =
                camera_from_raster(Point3f(0, 1, 0)) - camera_from_raster(Point3f(0, 0, 0));

            // Compute _cosTotalWidth_ for perspective camera
            Point2f radius = Point2f(film.get_filter().get_radius());
            Point3f pCorner(-radius.x, -radius.y, 0.f);
            Vec3f wCornerCamera = normalize(Vec3f(camera_from_raster(pCorner)));
            cos_total_width = wCornerCamera.z;
            LOG_ASSERT(.9999 * cos_total_width < std::cos(radians(fov / 2)));

            // Compute image plane area at $z=1$ for _PerspectiveCamera_
            Point2i res = film.get_full_resolution();
            Point3f min = camera_from_raster(Point3f(0, 0, 0));
            Point3f max = camera_from_raster(Point3f(res.x, res.y, 0));
            min /= min.z;
            max /= max.z;
            A = std::abs((max.x - min.x) * (max.y - min.y));

            // Compute minimum differentials for _PerspectiveCamera_
            find_minimum_differentials(this);
        }

        PerspectiveCamera() = default;

        static PerspectiveCamera* create(const ParameterDictionary& parameters,
            const CameraTransform& camera_transform, Film film,
            Medium medium, const FileLoc* loc,
            Allocator alloc = {});

        LOQUAT_CPU_GPU
        pstd::optional<CameraRay> generate_ray(CameraSample sample,
            SampledWavelengths& lambda) const;

        LOQUAT_CPU_GPU
        pstd::optional<CameraRayDifferential> generate_ray_differential(
            CameraSample sample, SampledWavelengths& lambda) const;

        LOQUAT_CPU_GPU
        SampledSpectrum importance(const Ray& ray, SampledWavelengths& lambda,
            Point2f* pRaster2 = nullptr) const;
        LOQUAT_CPU_GPU
        void importance_PDF(const Ray& ray, Float* pdf_position,
            Float* pdf_direction) const;
        LOQUAT_CPU_GPU
        pstd::optional<CameraWiSample> sample_light_incoming(
            const Interaction& ref, Point2f u, SampledWavelengths& lambda) const;

        std::string to_string() const;

    private:
        Vec3f dx_camera;
        Vec3f dy_camera;
        Float cos_total_width;
        Float A;
    };

    class SphericalCamera : public CameraBase
    {
    public:
        enum Mapping
        {
            EquiRectangular,
            EqualArea
        };

        SphericalCamera(CameraBaseParameters base_parameters, Mapping mapping)
            : CameraBase(base_parameters)
            , mapping(mapping)
        {
            // Compute minimum differentials for _SphericalCamera_
            find_minimum_differentials(this);
        }

        static SphericalCamera* create(const ParameterDictionary& parameters,
            const CameraTransform& camera_transform, Film film,
            Medium medium, const FileLoc* loc,
            Allocator alloc = {});

        LOQUAT_CPU_GPU
        pstd::optional<CameraRay> generate_ray(CameraSample sample,
            SampledWavelengths& lambda) const;

        LOQUAT_CPU_GPU
        pstd::optional<CameraRayDifferential> generate_ray_differential(
            CameraSample sample, SampledWavelengths& lambda) const
        {
            return CameraBase::generate_ray_differential(this, sample, lambda);
        }

        LOQUAT_CPU_GPU
        SampledSpectrum importance(const Ray& ray, SampledWavelengths& lambda,
            Point2f* pRaster2 = nullptr) const
        {
            LOG_FATAL("importance() unimplemented for SphericalCamera");
            return {};
        }

        LOQUAT_CPU_GPU
        void importance_PDF(const Ray& ray, Float* pdf_position,
            Float* pdf_direction) const
        {
            LOG_FATAL("importance_PDF() unimplemented for SphericalCamera");
        }

        LOQUAT_CPU_GPU
        pstd::optional<CameraWiSample> sample_light_incoming(
            const Interaction& ref, Point2f u, SampledWavelengths& lambda) const
        {
            LOG_FATAL("sample_light_incoming() unimplemented for SphericalCamera");
            return {};
        }

        std::string to_string() const;

    private:
        Mapping mapping;
    };

    struct ExitPupilSample
    {
        Point3f p_pupil;
        Float pdf;
    };

    class RealisticCamera : public CameraBase
    {
    public:
        RealisticCamera(CameraBaseParameters base_parameters,
            std::vector<Float>& lensParameters, Float focusDistance,
            Float apertureDiameter, Image aperture_image, Allocator alloc);

        static RealisticCamera* create(const ParameterDictionary& parameters,
            const CameraTransform& camera_transform, Film film,
            Medium medium, const FileLoc* loc,
            Allocator alloc = {});

        LOQUAT_CPU_GPU
        pstd::optional<CameraRay> generate_ray(CameraSample sample,
            SampledWavelengths& lambda) const;

        LOQUAT_CPU_GPU
        pstd::optional<CameraRayDifferential> generate_ray_differential(
            CameraSample sample, SampledWavelengths& lambda) const
        {
            return CameraBase::generate_ray_differential(this, sample, lambda);
        }

        LOQUAT_CPU_GPU
        SampledSpectrum importance(const Ray& ray, SampledWavelengths& lambda,
            Point2f* pRaster2 = nullptr) const
        {
            LOG_FATAL("importance() unimplemented for RealisticCamera");
            return {};
        }

        LOQUAT_CPU_GPU
        void importance_PDF(const Ray& ray, Float* pdf_position, Float* pdf_direction) const
        {
            LOG_FATAL("importance_PDF() unimplemented for RealisticCamera");
        }

        LOQUAT_CPU_GPU
        pstd::optional<CameraWiSample> sample_light_incoming(const Interaction& ref, Point2f u,
            SampledWavelengths& lambda) const
        {
            LOG_FATAL("sample_light_incoming() unimplemented for RealisticCamera");
            return {};
        }

        std::string to_string() const;

    private:
        struct LensElementInterface
        {
            Float curvature_radius;
            Float thickness;
            Float eta;
            Float aperture_radius;
            std::string to_string() const;
        };

        LOQUAT_CPU_GPU
        Float lens_rear_z() const { return element_interfaces.back().thickness; }

        LOQUAT_CPU_GPU
        Float lens_front_z() const
        {
            Float z_sum = 0;
            for (const LensElementInterface& element : element_interfaces)
            {
                z_sum += element.thickness;
            }
            return z_sum;
        }

        LOQUAT_CPU_GPU
        Float rear_element_radius() const
        {
            return element_interfaces.back().aperture_radius;
        }

        LOQUAT_CPU_GPU
        Float trace_lenses_from_film(const Ray& ray_camera, Ray* ray_out) const;

        LOQUAT_CPU_GPU
        static bool IntersectSphericalElement(Float radius, Float z_center,
            const Ray& ray,  Float* t, Normal3f* n)
        {
            // Compute _t0_ and _t1_ for ray--element intersection
            Point3f o = ray.origin - Vec3f(0, 0, z_center);
            Float A = ray.direction.x * ray.direction.x 
                    + ray.direction.y * ray.direction.y 
                    + ray.direction.z * ray.direction.z;
            Float B = 2 * (ray.direction.x * o.x 
                         + ray.direction.y * o.y 
                         + ray.direction.z * o.z);
            Float C = o.x * o.x + o.y * o.y + o.z * o.z - radius * radius;
            Float t0, t1;
            if (!quadratic(A, B, C, &t0, &t1))
            {
                return false;
            }

            // Select intersection $t$ based on ray direction and element curvature
            bool useCloserT = (ray.direction.z > 0) ^ (radius < 0);
            *t = useCloserT ? std::min(t0, t1) : std::max(t0, t1);
            if (*t < 0)
            {
                return false;
            }

            // Compute surface normal of element at ray intersection point
            *n = Normal3f(Vec3f(o + *t * ray.direction));
            *n = face_forward(normalize(*n), -ray.direction);

            return true;
        }

        LOQUAT_CPU_GPU
        Float trace_lenses_from_scene(const Ray& ray_camera, Ray* ray_out) const;

        void draw_lens_system() const;
        void draw_ray_path_from_file(const Ray& r, bool arrow, bool to_optical_intercept) const;
        void draw_ray_path_from_scene(const Ray& r, bool arrow, bool to_optical_intercept) const;

        static void compute_cardinal_points(Ray ray_in, Ray ray_out, Float* p, Float* f);
        void compute_thick_lens_approximation(Float pz[2], Float f[2]) const;
        Float focus_thick_lens(Float focusDistance);
        AABB2f bound_exit_pupil(Float film_x0, Float film_x1) const;
        void render_exit_pupil(Float sx, Float sy, const char* filename) const;

        LOQUAT_CPU_GPU
        pstd::optional<ExitPupilSample> simple_exit_pupil(Point2f pFilm, Point2f uLens) const;

        void test_exit_pupil_bounds() const;

        AABB2f physical_extent;
        pstd::vector<LensElementInterface> element_interfaces;
        Image aperture_image;
        pstd::vector<AABB2f> exit_pupil_bounds;
    };

    LOQUAT_CPU_GPU
    inline pstd::optional<CameraRay> Camera::generate_ray(CameraSample sample,
        SampledWavelengths& lambda) const {
        auto generate = [&](auto ptr) { return ptr->generate_ray(sample, lambda); };
        return dispatch(generate);
    }

    LOQUAT_CPU_GPU
    inline Film Camera::get_film() const
    {
        auto getfilm = [&](auto ptr) { return ptr->get_film(); };
        return dispatch(getfilm);
    }

    LOQUAT_CPU_GPU
    inline Float Camera::sample_time(Float u) const
    {
        auto sample = [&](auto ptr) { return ptr->sample_time(u); };
        return dispatch(sample);
    }

    LOQUAT_CPU_GPU
    inline const CameraTransform& Camera::get_camera_transform() const
    {
        auto gtc = [&](auto ptr) -> const CameraTransform& {
            return ptr->get_camera_transform();
            };
        return dispatch(gtc);
    }

    LOQUAT_CPU_GPU
    inline void Camera::approximate_dp_dxy(Point3f p, Normal3f n, Float time,
        int samples_per_pixel, Vec3f* dpdx,
        Vec3f* dpdy) const
    {
        if constexpr (all_inherit_from<CameraBase>(Camera::Types())) {
            return ((const CameraBase*)pointer())
                ->approximate_dp_dxy(p, n, time, samples_per_pixel, dpdx, dpdy);
        }
        else
        {
            auto approx = [&](auto ptr) {
                return ptr->approximate_dp_dxy(p, n, time, samples_per_pixel, dpdx, dpdy);
                };
            return dispatch(approx);
        }
    }
}