// pbrt is Copyright(c) 1998-2020 Matt Pharr, Wenzel Jakob, and Greg Humphreys.
// The pbrt source code is licensed under the Apache License, Version 2.0.
// SPDX: Apache-2.0

// This file has been modified from the original, original notice is above.

#pragma once

#include <atomic>
#include <format>
#include <map>
#include <string>
#include <thread>
#include <vector>

#include "pbr/bsdf.h"
#include "pbr/base/bxdf.h"
#include "pbr/base/camera.h"
#include "pbr/base/film.h"
#include "pbr/math/sampling.h"
#include "pbr/math/transform.h"
#include "pbr/math/vector_math.h"
#include "pbr/util/color.h"
#include "pbr/util/color_space.h"
#include "pbr/util/parallel.h"
#include "pbr/util/pstd.h"
#include "pbr/util/spectrum.h"

namespace loquat
{
	class PixelSensor
	{
    public:
        static PixelSensor* create(const ParameterDictionary& parameters,
            const RGBColorSpace* color_space, Float exposure_time,
            const FileLoc* loc, Allocator allocatoratoratorator);

        static PixelSensor* create_default(Allocator allocatoratoratorator = {});

        PixelSensor(Spectrum r, Spectrum g, Spectrum b, 
            const RGBColorSpace* output_color_space, 
            Spectrum sensor_illumination, Float imaging_ratio, Allocator allocatoratoratorator)
            : r_bar(r, allocatoratoratorator)
            , g_bar(g, allocatoratoratorator)
            , b_bar(b, allocatoratoratorator)
            , imaging_ratio(imaging_ratio)
        {
            // Compute XYZ from camera RGB matrix
            // Compute _rgbCamera_ values for training swatches
            Float rgbCamera[swatch_reflectance_count][3];
            for (int i = 0; i < swatch_reflectance_count; ++i) {
                RGB rgb = project_reflectance<RGB>(swatch_reflectances[i],
                    sensor_illumination, &r_bar, &g_bar, &b_bar);
                for (int c = 0; c < 3; ++c)
                {
                    rgbCamera[i][c] = rgb[c];
                }
            }

            // Compute _xyzOutput_ values for training swatches
            Float xyzOutput[24][3];
            Float sensorWhiteG = inner_product(sensor_illumination, &g_bar);
            Float sensorWhiteY = inner_product(sensor_illumination, 
                &Spectra::Y());
            for (size_t i = 0; i < swatch_reflectance_count; ++i)
            {
                Spectrum s = swatch_reflectances[i];
                XYZ xyz =
                    project_reflectance<XYZ>(s,
                        &output_color_space->illuminant,
                        &Spectra::X(),
                        &Spectra::Y(),
                        &Spectra::Z()) *
                    (sensorWhiteY / sensorWhiteG);
                for (int c = 0; c < 3; ++c)
                {
                    xyzOutput[i][c] = xyz[c];
                }
            }

            // Initialize _XYZ_from_sensor_RGB_ using linear least squares
            pstd::optional<SquareMatrix<3>> m =
                linear_least_squares(rgbCamera, xyzOutput, 
                    swatch_reflectance_count);
            if (!m)
            {
                LOG_FATAL("Sensor XYZ from RGB matrix could not be solved.");
            }
            XYZ_from_sensor_RGB = *m;
        }

        PixelSensor(const RGBColorSpace* output_color_space,
            Spectrum sensor_illumination, Float imaging_ratio,
            Allocator allocatoratoratorator)
            : r_bar(&Spectra::X(), allocatoratoratorator)
            , g_bar(&Spectra::Y(), allocatoratoratorator)
            , b_bar(&Spectra::Z(), allocatoratoratorator)
            , imaging_ratio(imaging_ratio)
        {
            // Compute white balancing matrix for XYZ _PixelSensor_
            if (sensor_illumination)
            {
                Point2f sourceWhite = SpectrumToXYZ(sensor_illumination).xy();
                Point2f targetWhite = output_color_space->w;
                XYZ_from_sensor_RGB = white_balance(sourceWhite, targetWhite);
            }
        }

        LOQUAT_CPU_GPU
        RGB to_sensor_RGB(SampledSpectrum L,
            const SampledWavelengths& wavelengths) const
        {
            L = safe_divide(L, wavelengths.PDF());
            return imaging_ratio * RGB(
                (r_bar.sample(wavelengths) * L).average(),
                (g_bar.sample(wavelengths) * L).average(),
                (b_bar.sample(wavelengths) * L).average());
        }

        SquareMatrix<3> XYZ_from_sensor_RGB;

    private:
        template <typename Triplet>
        static Triplet project_reflectance(Spectrum r, Spectrum illumination,
            Spectrum b1, Spectrum b2, Spectrum b3);

        DenselySampledSpectrum r_bar;
        DenselySampledSpectrum g_bar;
        DenselySampledSpectrum b_bar;
        Float imaging_ratio;
        static constexpr int swatch_reflectance_count = 24;
        static Spectrum swatch_reflectances[swatch_reflectance_count];
	};

	class VisibleSurface
	{
    public:
        LOQUAT_CPU_GPU
        VisibleSurface(const SurfaceInteraction& interaction,
            SampledSpectrum albedo, const SampledWavelengths& wavelengths);

        LOQUAT_CPU_GPU
        operator bool() const
        {
            return set;
        }

        VisibleSurface() = default;

        [[nodiscard]]
        std::string to_string() const;

        Point3f p;
        Normal3f normal;
        Normal3f shading_normal;
        Point2f uv;
        Float time = 0;
        Vec3f dpdx;
        Vec3f dpdy;
        SampledSpectrum albedo;
        bool set = false;
	};

	struct FilmBaseParameters
	{
        FilmBaseParameters(const ParameterDictionary& parameters,
            Filter filter, const PixelSensor* sensor, const FileLoc* loc);
        FilmBaseParameters(Point2i full_resolution, AABB2i pixel_bounds,
            Filter filter, Float diagonal, const PixelSensor* sensor,
            std::string filename)
            : full_resolution{ full_resolution }
            , pixel_bounds{ pixel_bounds }
            , filter{ filter }
            , diagonal{ diagonal }
            , sensor{ sensor }
            , filename{ filename }
        {}

        Point2i full_resolution;
        AABB2i pixel_bounds;
        Filter filter;
        Float diagonal;
        const PixelSensor* sensor;
        std::string filename;
	};

	class FilmBase
	{
    public:
        FilmBase(FilmBaseParameters p)
            : full_resolution{ p.full_resolution }
            , pixel_bounds{ p.pixel_bounds }
            , filter{ p.filter }
            , diagonal{ p.diagonal * .001f }
            , sensor{ p.sensor }
            , filename{ p.filename }
        {
            LOG_ASSERT(!pixel_bounds.is_empty());
            LOG_ASSERT(pixel_bounds.min.x >= 0);
            LOG_ASSERT(pixel_bounds.max.x <= full_resolution.x);
            LOG_ASSERT(pixel_bounds.min.y >= 0);
            LOG_ASSERT(pixel_bounds.max.y <= full_resolution.y);
            LOG_INFO(std::format("created film with full resolution ({}, {}), pixel_bounds {}",
                full_resolution.x, full_resolution.y , pixel_bounds.to_string()));
        }

        LOQUAT_CPU_GPU
        Point2i get_full_resolution() const
        {
            return full_resolution;
        }

        LOQUAT_CPU_GPU
        AABB2i get_pixel_bounds() const
        {
            return pixel_bounds;
        }

        LOQUAT_CPU_GPU
        Float get_diagonal() const
        {
            return diagonal;
        }

        LOQUAT_CPU_GPU
        Filter get_filter() const
        {
            return filter;
        }

        LOQUAT_CPU_GPU
        const PixelSensor* get_pixel_sensor() const
        {
            return sensor;
        }

        std::string get_filename() const
        {
            return filename;
        }

        LOQUAT_CPU_GPU
        SampledWavelengths sample_wavelengths(Float sample_1D) const
        {
            return SampledWavelengths::sample_visible(sample_1D);
        }

        LOQUAT_CPU_GPU
        AABB2f sample_bounds() const;

        std::string base_to_string() const;

    protected:
        Point2i full_resolution;
        AABB2i pixel_bounds;
        Filter filter;
        Float diagonal;
        const PixelSensor* sensor;
        std::string filename;
	};

	class RGBFilm : public FilmBase
	{
    public:
        LOQUAT_CPU_GPU
        bool uses_visible_surface() const
        {
            return false;
        }

        LOQUAT_CPU_GPU
        void add_sample(Point2i film_point, SampledSpectrum L,
            const SampledWavelengths& wavelengths, const VisibleSurface*,
            Float weight)
        {
            // Convert sample radiance to _PixelSensor_ RGB
            RGB rgb = sensor->to_sensor_RGB(L, wavelengths);

            // Optionally clamp sensor RGB value
            Float m = std::max({ rgb.r, rgb.g, rgb.b });
            if (m > max_component_value)
            {
                rgb *= max_component_value / m;
            }

            LOG_ASSERT(inside_exclusive(film_point, pixel_bounds));
            // Update pixel values with filtered sample contribution
            Pixel& pixel = pixels[film_point];
            for (int c = 0; c < 3; ++c)
            {
                pixel.rgb_sum[c] += weight * rgb[c];
            }
            pixel.weight_sum += weight;
        }

        LOQUAT_CPU_GPU
        RGB get_pixel_RGB(Point2i p, Float splat_scale = 1) const
        {
            const Pixel& pixel = pixels[p];
            RGB rgb(pixel.rgb_sum[0], pixel.rgb_sum[1], pixel.rgb_sum[2]);
            // Normalize _rgb_ with weight sum
            Float weight_sum = pixel.weight_sum;
            if (weight_sum != 0)
                rgb /= weight_sum;

            // Add splat value at pixel
            for (int c = 0; c < 3; ++c)
            {
                rgb[c] += splat_scale * pixel.rgb_splat[c] / filter_integral;
            }

            // Convert _rgb_ to output RGB color space
            rgb = output_RGB_from_sensor_RGB * Vec3f(rgb);

            return rgb;
        }

        RGBFilm(FilmBaseParameters p, const RGBColorSpace* color_space,
            Float max_component_value = INFINITY, bool write_FP16 = true,
            Allocator allocatoratorator = {});

        static RGBFilm* create(const ParameterDictionary& parameters,
            Float exposure_time, Filter filter,
            const RGBColorSpace* color_space,
            const FileLoc* loc, Allocator allocatoratorator);

        LOQUAT_CPU_GPU
        void add_splat(Point2f p, SampledSpectrum v,
            const SampledWavelengths& wavelengths);

        void write_image(ImageMetadata metadata, Float splat_scale = 1);
        Image get_image(ImageMetadata* metadata, Float splat_scale = 1);

        [[nodiscard]]
        std::string to_string() const;

        LOQUAT_CPU_GPU
        RGB to_output_RGB(SampledSpectrum L,
            const SampledWavelengths& wavelengths) const
        {
            RGB sensorRGB = sensor->to_sensor_RGB(L, wavelengths);
            return output_RGB_from_sensor_RGB * Vec3f(sensorRGB);
        }

        LOQUAT_CPU_GPU
        void reset_pixel(Point2i p)
        {
            memset(&pixels[p], 0, sizeof(Pixel));
        }

    private:
        struct Pixel
        {
            Pixel() = default;
            double rgb_sum[3] = { 0.0, 0.0, 0.0 };
            double weight_sum = 0.0;
            AtomicDouble rgb_splat[3];
        };

        const RGBColorSpace* color_space;
        Float max_component_value;
        bool write_FP16;
        Float filter_integral;
        SquareMatrix<3> output_RGB_from_sensor_RGB;
        Array2D<Pixel> pixels;
	};

	class GBufferFilm : public FilmBase
	{
    public:
        GBufferFilm(FilmBaseParameters parameters,
            const AnimatedTransform& output_from_render,
            bool apply_inverse, const RGBColorSpace* color_space,
            Float max_component_value = INFINITY, bool write_FP16 = true,
            Allocator allocatorator = {});

        static GBufferFilm* create(const ParameterDictionary& parameters,
            Float exposure_time, const CameraTransform& camera_transform,
            Filter filter, const RGBColorSpace* color_space,
            const FileLoc* loc, Allocator allocatorator);

        LOQUAT_CPU_GPU
        void add_sample(Point2i film_point, SampledSpectrum L,
            const SampledWavelengths& wavelengths,
                const VisibleSurface* visible_surface, Float weight);

        LOQUAT_CPU_GPU
        void add_splat(Point2f p, SampledSpectrum v,
            const SampledWavelengths& wavelengths);

        LOQUAT_CPU_GPU
        RGB to_output_RGB(SampledSpectrum L,
            const SampledWavelengths& wavelengths) const
        {
            RGB cameraRGB = sensor->to_sensor_RGB(L, wavelengths);
            return output_RGB_from_sensor_RGB * Vec3f(cameraRGB);
        }

        LOQUAT_CPU_GPU
        bool uses_visible_surface() const
        {
            return true;
        }

        LOQUAT_CPU_GPU
        RGB get_pixel_RGB(Point2i p, Float splat_scale = 1) const
        {
            const Pixel& pixel = pixels[p];
            RGB rgb(pixel.rgb_sum[0], pixel.rgb_sum[1], pixel.rgb_sum[2]);

            // Normalize pixel with weight sum
            Float weight_sum = pixel.weight_sum;
            if (weight_sum != 0)
            {
                rgb /= weight_sum;
            }

            // Add splat value at pixel
            for (int c = 0; c < 3; ++c)
            {
                rgb[c] += splat_scale * pixel.rgb_splat[c] / filter_integral;
            }

            rgb = output_RGB_from_sensor_RGB * Vec3f(rgb);

            return rgb;
        }

        void write_image(ImageMetadata metadata, Float splat_scale = 1);
        Image get_image(ImageMetadata* metadata, Float splat_scale = 1);

        [[nodiscard]]
        std::string to_string() const;

        LOQUAT_CPU_GPU
        void reset_pixel(Point2i p)
        {
            memset(&pixels[p], 0, sizeof(Pixel));
        }

    private:
        struct Pixel
        {
            Pixel() = default;
            double rgb_sum[3] = { 0.0, 0.0, 0.0 };
            double weight_sum = 0.;
            double gBuffer_weight_sum = 0.;
            AtomicDouble rgb_splat[3];
            Point3f point_sum;
            Float dzdxSum = 0;
            Float dzdy_sum = 0;
            Normal3f normal_sum;
            Normal3f ns_sum;
            Point2f uv_sum;
            double rgb_albedo_sum[3] = { 0.0, 0.0, 0.0 };
            VarianceEstimator<Float> rgb_variance[3];
        };

        AnimatedTransform output_from_render;
        bool apply_inverse;
        Array2D<Pixel> pixels;
        const RGBColorSpace* color_space;
        Float max_component_value;
        bool write_FP16;
        Float filter_integral;
        SquareMatrix<3> output_RGB_from_sensor_RGB;
	};

	class SpectralFilm : public FilmBase
	{
    public:
        LOQUAT_CPU_GPU
        bool uses_visible_surface() const
        {
            return false;
        }

        LOQUAT_CPU_GPU
        SampledWavelengths sample_wavelengths(Float u) const
        {
            return SampledWavelengths::sample_uniform(u, lambda_min,
                lambda_max);
        }

        LOQUAT_CPU_GPU
        void add_sample(Point2i pFilm, SampledSpectrum L,
            const SampledWavelengths& lambda, const VisibleSurface*, 
            Float weight)
        {
            // Convert sample radiance to _PixelSensor_ RGB
            RGB rgb = sensor->to_sensor_RGB(L, lambda);

            // Optionally clamp sensor RGB value
            Float m = std::max({ rgb.r, rgb.g, rgb.b });
            if (m > max_component_value)
            {
                rgb *= max_component_value / m;
            }

            LOG_ASSERT(inside_exclusive(pFilm, pixel_bounds));
            // Update RGB fields in Pixel structure.
            Pixel& pixel = pixels[pFilm];
            for (int c = 0; c < 3; ++c)
            {
                pixel.rgb_sum[c] += weight * rgb[c];
            }
            pixel.rgb_weight_sum += weight;

            // Spectral processing starts here.
            // Optionally clamp spectral value. (TODO: for spectral should we
            // just clamp channels individually?)
            Float lm = L.max_component_value();
            if (lm > max_component_value)
            {
                L *= max_component_value / lm;
            }

            // The CIE_Y_integral factor effectively cancels out the effect of
            // the conversion of light sources to use photometric units for
            // specification.  We then do *not* divide by the PDF in |lambda|
            // but take advantage of the fact that we know that it is uniform
            // in sample_wavelengths(), the fact that the buckets all have the
            // same extend, and can then just average radiance in buckets
            // below.
            L *= weight * CIE_Y_INTEGRAL;

            // Accumulate contributions in spectral buckets.
            for (int i = 0; i < SPECTRUM_SAMPLE_COUNT; ++i)
            {
                int b = lambda_to_bucket(lambda[i]);
                pixel.bucket_sums[b] += L[i];
                pixel.weight_sums[b] += weight;
            }
        }

        LOQUAT_CPU_GPU
        RGB get_pixel_RGB(Point2i p, Float splat_scale = 1) const;

        SpectralFilm(FilmBaseParameters p, Float lambda_min, Float lambda_max,
            int bucket_count, const RGBColorSpace* color_space,
            Float max_component_value = INFINITY, bool write_FP16 = true,
            Allocator allocator = {});

        static SpectralFilm* create(const ParameterDictionary& parameters,
            Float exposure_time, Filter filter,
            const RGBColorSpace* color_space, const FileLoc* loc,
            Allocator allocator);

        LOQUAT_CPU_GPU
        void add_splat(Point2f p, SampledSpectrum v,
            const SampledWavelengths& lambda);

        void write_image(ImageMetadata metadata, Float splat_scale = 1);

        // Returns an image with both RGB and spectral components, following
        // the layout proposed in "An OpenEXR Layout for Sepctral Images" by
        // Fichet et al., https://jcgt.org/published/0010/03/01/.
        Image get_image(ImageMetadata* metadata, Float splat_scale = 1);

        [[nodiscard]]
        std::string to_string() const;

        LOQUAT_CPU_GPU
        RGB to_output_RGB(SampledSpectrum L,
            const SampledWavelengths& lambda) const
        {
            LOG_FATAL("to_output_RGB() is unimplemented. But that's ok since "
                "it's only used in the SPPM integrator, which is inherently "
                "very much based on RGB output.");
            return {};
        }

        LOQUAT_CPU_GPU void reset_pixel(Point2i p)
        {
            Pixel& pix = pixels[p];
            pix.rgb_sum[0] = pix.rgb_sum[1] = pix.rgb_sum[2] = 0.;
            pix.rgb_weight_sum = 0.;
            pix.rgb_splat[0] = pix.rgb_splat[1] = pix.rgb_splat[2] = 0.;
            memset(pix.bucket_sums, 0, bucket_count * sizeof(double));
            memset(pix.weight_sums, 0, bucket_count * sizeof(double));
            memset(pix.bucket_splats, 0, bucket_count * sizeof(AtomicDouble));
        }

    private:
        LOQUAT_CPU_GPU
        int lambda_to_bucket(Float lambda) const
        {
            int bucket = bucket_count * (lambda - lambda_min) 
                / (lambda_max - lambda_min);
            return clamp(bucket, 0, bucket_count - 1);
        }

        struct Pixel
        {
            Pixel() = default;
            // Continue to store RGB, both to include in the final image as
            // well as for previews during rendering.
            double rgb_sum[3] = { 0.0, 0.0, 0.0 };
            double rgb_weight_sum = 0.;
            AtomicDouble rgb_splat[3];
            // The following will all have bucket_count entries.
            double* bucket_sums, * weight_sums;
            AtomicDouble* bucket_splats;
        };

        const RGBColorSpace* color_space;
        Float lambda_min;
        Float lambda_max;
        int bucket_count;
        Float max_component_value;
        bool write_FP16;
        Float filter_integral;
        Array2D<Pixel> pixels;
        SquareMatrix<3> output_RGB_from_sensor_RGB;
	};


    LOQUAT_CPU_GPU
    inline SampledWavelengths Film::sample_wavelengths(Float u) const
    {
        auto sample = [&](auto ptr) { return ptr->sample_wavelengths(u); };
        return dispatch(sample);
    }

    LOQUAT_CPU_GPU
    inline AABB2f Film::sample_bounds() const
    {
        auto sb = [&](auto ptr) { return ptr->sample_bounds(); };
        return dispatch(sb);
    }

    LOQUAT_CPU_GPU
    inline AABB2i Film::get_pixel_bounds() const
    {
        auto pb = [&](auto ptr) { return ptr->get_pixel_bounds(); };
        return dispatch(pb);
    }

    LOQUAT_CPU_GPU
    inline Point2i Film::get_full_resolution() const
    {
        auto fr = [&](auto ptr) { return ptr->get_full_resolution(); };
        return dispatch(fr);
    }

    LOQUAT_CPU_GPU
    inline Float Film::get_diagonal() const
    {
        auto diag = [&](auto ptr) { return ptr->get_diagonal(); };
        return dispatch(diag);
    }

    LOQUAT_CPU_GPU
    inline Filter Film::get_filter() const
    {
        auto filter = [&](auto ptr) { return ptr->get_filter(); };
        return dispatch(filter);
    }

    LOQUAT_CPU_GPU
    inline bool Film::uses_visible_surface() const
    {
        auto uses = [&](auto ptr) { return ptr->uses_visible_surface(); };
        return dispatch(uses);
    }

    LOQUAT_CPU_GPU
    inline RGB Film::get_pixel_RGB(Point2i p, Float splat_scale) const
    {
        auto get = [&](auto ptr) { return ptr->get_pixel_RGB(p, splat_scale); };
        return dispatch(get);
    }

    LOQUAT_CPU_GPU
    inline RGB Film::to_output_RGB(SampledSpectrum L,
        const SampledWavelengths& lambda) const
    {
        auto out = [&](auto ptr) { return ptr->to_output_RGB(L, lambda); };
        return dispatch(out);
    }

    LOQUAT_CPU_GPU
    inline void Film::add_sample(Point2i pFilm, SampledSpectrum L,
            const SampledWavelengths& lambda,
            const VisibleSurface* visibleSurface, Float weight)
    {
        auto add = [&](auto ptr) {
            return ptr->add_sample(pFilm, L, lambda, visibleSurface, weight);
            };
        return dispatch(add);
    }

    LOQUAT_CPU_GPU
    inline const PixelSensor* Film::get_pixel_sensor() const
    {
        auto filter = [&](auto ptr) { return ptr->get_pixel_sensor(); };
        return dispatch(filter);
    }

    LOQUAT_CPU_GPU
    inline void Film::reset_pixel(Point2i p)
    {
        auto rp = [&](auto ptr) { ptr->reset_pixel(p); };
        return dispatch(rp);
    }
}
