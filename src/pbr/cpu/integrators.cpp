// pbrt is Copyright(c) 1998-2020 Matt Pharr, Wenzel Jakob, and Greg Humphreys.
// The pbrt source code is licensed under the Apache License, Version 2.0.
// SPDX: Apache-2.0

// This file has been modified from the original, original notice is above.

#include <algorithm>
#include <format>

#include "pbr/cpu/integrators.h"

#include "pbr/bsdf.h"
#include "pbr/bssrdf.h"
#include "pbr/cameras.h"
#include "pbr/films.h"
#include "pbr/filters.h"
#include "pbr/struct/interaction.h"
#include "pbr/lights.h"
#include "pbr/materials.h"
#include "pbr/media.h"
#include "pbr/options.h"
#include "pbr/samplers.h"
#include "pbr/shapes.h"
#include "pbr/math/blue_noise.h"
#include "pbr/math/hash.h"
#include "pbr/math/low_discrepancy.h"
#include "pbr/math/math.h"
#include "pbr/math/rng.h"
#include "pbr/struct/containers.h"
#include "pbr/struct/image.h"
#include "pbr/struct/parameter_dictionary.h"
#include "pbr/util/check.h"
#include "pbr/util/color.h"
#include "pbr/util/color_space.h"
#include "pbr/util/display.h"
#include "pbr/util/error.h"
#include "pbr/util/file.h"
#include "pbr/util/memory.h"
#include "pbr/util/parallel.h"
#include "pbr/util/progress_reporter.h"
#include "pbr/util/pstd.h"
#include "pbr/util/sampling.h"
#include "pbr/util/spectrum.h"
#include "pbr/util/stats.h"
#include "pbr/util/string.h"

namespace loquat
{
#if ENABLE_WIP_CODE

	STAT_COUNTER("Integrator/Camera rays traced", camera_ray_count);


	std::unique_ptr<RandomWalkIntegrator> RandomWalkIntegrator::create(
		const ParameterDictionary& parameters, Camera camera, Sampler sampler,
		Primitive aggregate, std::vector<Light> lights, const FileLoc* loc)
	{
		int maxDepth = parameters.get_one_int("maxdepth", 5);
		return std::make_unique<RandomWalkIntegrator>(maxDepth, camera,
			sampler, aggregate, lights);
	}

	std::string RandomWalkIntegrator::to_string() const
	{
		return std::format("[ RandomWalkIntegrator maxDepth: {} ]", max_depth);
	}

	Integrator::~Integrator() = default;

    void ImageTileIntegrator::render()
    {
        // Handle debug_start, if set
        if (!options->debug_start.empty())
        {
            std::vector<int> c = split_string_to_ints(options->debug_start, ',');
            if (c.empty())
            {
                LOG_FATAL(std::format("Didn't find integer values after --debugstart: {}",
                    options->debug_start));
            }
            if (c.size() != 3)
            {
                LOG_FATAL(std::format("Didn't find three integer values after --debugstart: {}",
                    options->debug_start));
            }

            Point2i p_pixel(c[0], c[1]);
            int sample_index = c[2];

            ScratchBuffer scratch_buffer(65536);
            Sampler tile_sampler = sampler_prototype.clone(Allocator());
            tile_sampler.start_pixel_sample(p_pixel, sample_index);

            evaluate_pixel_sample(p_pixel, sample_index, tile_sampler, scratch_buffer);

            return;
        }

        thread_local Point2i thread_pixel;
        thread_local int thread_sample_index;
        CheckCallbackScope _([&]() {
            return std::format("Rendering failed at pixel ({}, {}) sample {}. Debug with "
                "\"--debugstart {},{},{}\"\n",
                thread_pixel.x, thread_pixel.y, thread_sample_index,
                thread_pixel.x, thread_pixel.y, thread_sample_index);
            });

        // Declare common variables for rendering image in tiles
        ThreadLocal<ScratchBuffer> scratchBuffers([]() { return ScratchBuffer(); });

        ThreadLocal<Sampler> samplers([this]() { return sampler_prototype.clone(); });

        AABB2i pixel_bounds = camera.get_film().get_pixel_bounds();
        int spp = sampler_prototype.get_samples_per_pixel();
        ProgressReporter progress(int64_t(spp) * pixel_bounds.area(), "Rendering",
            options->quiet);

        int wave_start = 0;
        int wave_end = 1;
        int nextWaveSize = 1;

        if (options->record_pixel_statistics)
        {
            stats_enable_pixel_stats(pixel_bounds,
                remove_extension(camera.get_film().get_filename()));
        }

        // Handle MSE reference image, if provided
        pstd::optional<Image> reference_image;
        FILE* mse_out_file = nullptr;
        if (!options->mse_reference_image.empty())
        {
            auto mse = Image::read(options->mse_reference_image);
            reference_image = mse.image;

            AABB2i mse_pixel_bounds =
                mse.metadata.pixel_bounds
                ? *mse.metadata.pixel_bounds
                : AABB2i(Point2i(0, 0), reference_image->get_resolution());
            if (!inside(pixel_bounds, mse_pixel_bounds))
            {
                LOG_FATAL(std::format(
                    "Output image pixel bounds {} aren't inside the MSE "
                    "image's pixel bounds {}.",
                    pixel_bounds.to_string(), mse_pixel_bounds.to_string()));
            }

            // Transform the pixel_bounds of the image we're rendering to the
            // coordinate system with mse_pixel_bounds.min at the origin, which
            // in turn gives us the section of the MSE image to crop. (This is
            // complicated by the fact that Image doesn't support pixel
            // bounds...)
            AABB2i cropBounds(Point2i(pixel_bounds.min - mse_pixel_bounds.min),
                Point2i(pixel_bounds.max - mse_pixel_bounds.min));
            *reference_image = reference_image->crop(cropBounds);
            LOG_ASSERT(reference_image->get_resolution() == Point2i(pixel_bounds.diagonal()));

            mse_out_file = fopen_write(options->mse_reference_output);
            if (!mse_out_file)
            {
                LOG_FATAL(std::format("{}: {}", options->mse_reference_output, error_string()));
            }
        }

        // Connect to display server if needed
        if (!options->display_server.empty())
        {
            Film film = camera.get_film();
            display_dynamic(film.get_filename(), Point2i(pixel_bounds.diagonal()),
                { "R", "G", "B" },
                [&](AABB2i b, pstd::span<pstd::span<float>> displayValue) {
                    int index = 0;
                    for (Point2i p : b) {
                        RGB rgb = film.get_pixel_RGB(Point2i(pixel_bounds.min) + p,
                            2.f / (wave_start + wave_end));
                        for (int c = 0; c < 3; ++c)
                            displayValue[c][index] = rgb[c];
                        ++index;
                    }
                });
        }

        // Render image in waves
        while (wave_start < spp)
        {
            // Render current wave's image tiles in parallel
            parallel_for_2D(pixel_bounds, [&](AABB2i tileBounds) {
                // Render image tile given by _tileBounds_
                ScratchBuffer& scratch_buffer = scratchBuffers.get();
                Sampler& sampler = samplers.get();
                LOG_INFO(std::format("Starting image tile ({},{})-({},{}) wave_start {}, wave_end {}\n",
                    tileBounds.min.x, tileBounds.min.y, tileBounds.max.x,
                    tileBounds.max.y, wave_start, wave_end));
                for (Point2i p_pixel : tileBounds) {
                    stats_report_pixel_start(p_pixel);
                    thread_pixel = p_pixel;
                    // Render samples in pixel _pPixel_
                    for (int sample_index = wave_start; sample_index < wave_end; ++sample_index) {
                        thread_sample_index = sample_index;
                        sampler.start_pixel_sample(p_pixel, sample_index);
                        evaluate_pixel_sample(p_pixel, sample_index, sampler, scratch_buffer);
                        scratch_buffer.reset();
                    }

                    stats_report_pixel_end(p_pixel);
                }
                LOG_INFO(std::format("Finished image tile ({},{})-({},{})\n", tileBounds.min.x,
                    tileBounds.min.y, tileBounds.max.x, tileBounds.max.y));
                progress.update((wave_end - wave_start) * tileBounds.area());
                });

            // Update start and end wave
            wave_start = wave_end;
            wave_end = std::min(spp, wave_end + nextWaveSize);
            if (!reference_image)
            {
                nextWaveSize = std::min(2 * nextWaveSize, 64);
            }
            if (wave_start == spp)
            {
                progress.done();
            }

            // Optionally write current image to disk
            if (wave_start == spp || options->write_partial_images || reference_image)
            {
                LOG_INFO(std::format("Writing image with spp = {}", wave_start));
                ImageMetadata metadata;
                metadata.render_time_seconds = progress.elapsed_seconds();
                metadata.samples_per_pixel = wave_start;
                if (reference_image && mse_out_file != 0)
                {
                    ImageMetadata filmMetadata;
                    Image filmImage =
                        camera.get_film().get_image(&filmMetadata, 1.f / wave_start);
                    ImageChannelValues mse =
                        filmImage.MSE(filmImage.all_channels_desc(), *reference_image);
                    fprintf(mse_out_file, "%d, %.9g\n", wave_start, mse.average());
                    metadata.MSE = mse.average();
                    fflush(mse_out_file);
                }
                if (wave_start == spp || options->write_partial_images)
                {
                    camera.init_metadata(&metadata);
                    camera.get_film().write_image(metadata, 1.0f / wave_start);
                }
            }
        }

        if (mse_out_file)
        {
            fclose(mse_out_file);
        }
        disconnect_from_display_server();
        LOG_INFO("Rendering finished");
    }

    void RayIntegrator::evaluate_pixel_sample(Point2i pixel, int sample_index,
        Sampler sampler, ScratchBuffer& scratch_buffer)
    {
        // Sample wavelengths for the ray
        Float lu = sampler.get_1D();
        if (options->disable_wavelength_jitter)
        {
            lu = 0.5;
        }
        SampledWavelengths lambda = camera.get_film().sample_wavelengths(lu);

        // Initialize _CameraSample_ for current sample
        Filter filter = camera.get_film().get_filter();
        CameraSample cameraSample = get_camera_sample(sampler, pixel, filter);

        // Generate camera ray for current sample
        pstd::optional<CameraRayDifferential> cameraRay =
            camera.generate_ray_differential(cameraSample, lambda);

        // Trace _cameraRay_ if valid
        SampledSpectrum L(0.);
        VisibleSurface visibleSurface;
        if (cameraRay)
        {
            // Double check that the ray's direction is normalized.
            DCHECK_GT(length(cameraRay->ray.direction), 0.999f);
            DCHECK_LT(length(cameraRay->ray.direction), 1.001f);
            // Scale camera ray differentials based on image sampling rate
            Float rayDiffScale =
                std::max<Float>(.125f, 1 
                    / std::sqrt((Float)sampler.get_samples_per_pixel()));
            if (!options->disable_pixel_jitter)
            {
                cameraRay->ray.scale_differentials(rayDiffScale);
            }

            ++camera_ray_count;
            // Evaluate radiance along camera ray
            bool initializeVisibleSurface = 
                camera.get_film().uses_visible_surface();
            L = cameraRay->weight * light_incoming(cameraRay->ray, lambda,
                sampler, scratch_buffer,
                initializeVisibleSurface ? &visibleSurface : nullptr);

            // Issue warning if unexpected radiance value is returned
            if (L.has_NaNs())
            {
                LOG_ERROR(std::format("Not-a-number radiance value returned for pixel ({}, "
                    "{}), sample {}. Setting to black.",
                    pixel.x, pixel.y, sample_index));
                L = SampledSpectrum(0.f);
            }
            else if (is_inf(L.y(lambda)))
            {
                LOG_ERROR(std::format("Infinite radiance value returned for pixel ({}, {}), "
                    "sample {}. Setting to black.",
                    pixel.x, pixel.y, sample_index));
                L = SampledSpectrum(0.f);
            }

            LOG_INFO(std::format(
                "Camera sample: {} -> ray {} -> L = {}, visibleSurface {}\n",
                cameraSample.to_string(), cameraRay->ray.to_string(),
                L.to_string(),
                (visibleSurface ? visibleSurface.to_string() : "(none)")));
        }
        else
        {
            LOG_INFO(std::format("Camera sample: {} -> no ray generated", 
                cameraSample.to_string()));
        }
        // Add camera ray's contribution to image
        camera.get_film().add_sample(pixel, L, lambda, &visibleSurface,
            cameraSample.filterWeight);
    }

    STAT_COUNTER("Intersections/Regular ray intersection tests", intersection_test_count);
    STAT_COUNTER("Intersections/Shadow ray intersection tests", shadow_test_count);

    pstd::optional<ShapeIntersection> Integrator::intersect(const Ray& ray,
        Float t_max) const
    {
        ++intersection_test_count;
        DCHECK_NE(ray.direction, Vec3f(0, 0, 0));
        if (aggregate)
        {
            return aggregate.intersect(ray, t_max);
        }
        else
        {
            return {};
        }
    }

    bool Integrator::has_intersection(const Ray& ray, Float t_max) const
    {
        ++shadow_test_count;
        DCHECK_NE(ray.direction, Vec3f(0, 0, 0));
        if (aggregate)
        {
            return aggregate.has_intersection(ray, t_max);
        }
        else
        {
            return false;
        }
    }

    SampledSpectrum Integrator::transmittance(const Interaction& p0, const Interaction& p1,
        const SampledWavelengths& lambda) const
    {
        RNG rng(hash(p0.p()), hash(p1.p()));

        // :-(
        Ray ray =
            p0.is_surface_interaction() 
            ? p0.as_surface().spawn_ray_to(p1) 
            : p0.spawn_ray_to(p1);
        SampledSpectrum transmittance(1.f), inv_w(1.f);
        if (length_squared(ray.direction) == 0)
        {
            return transmittance;
        }

        while (true)
        {
            pstd::optional<ShapeIntersection> si = intersect(ray, 1 - SHADOW_EPSILON);
            // Handle opaque surface along ray's path
            if (si && si->interaction.material)
                return SampledSpectrum(0.0f);

            // Update transmittance for current ray segment
            if (ray.medium) {
                Point3f pExit = ray(si ? si->t_hit : (1 - SHADOW_EPSILON));
                ray.direction = pExit - ray.origin;

                SampledSpectrum T_maj =
                    sample_t_maj(ray, 1.f, rng.uniform<Float>(), rng, lambda,
                        [&](Point3f p, MediumProperties mp, SampledSpectrum sigma_maj,
                            SampledSpectrum T_maj) {
                                SampledSpectrum sigma_n =
                                    clamp_zero(sigma_maj - mp.sigma_a - mp.sigma_s);

                                // ratio-tracking: only evaluate null scattering
                                Float pr = T_maj[0] * sigma_maj[0];
                                transmittance *= T_maj * sigma_n / pr;
                                inv_w *= T_maj * sigma_maj / pr;

                                if (!transmittance || !inv_w)
                                {
                                    return false;
                                }

                                return true;
                        });
                transmittance *= T_maj / T_maj[0];
                inv_w *= T_maj / T_maj[0];
            }

            // Generate next ray segment or return final transmittance
            if (!si)
            {
                break;
            }
            ray = si->interaction.spawn_ray_to(p1);
        }
        LOG_INFO(string_printf("transmittance from %s to %s = %s", p0.point, p1.point, transmittance));
        return transmittance / inv_w.average();
    }

    std::string Integrator::to_string() const
    {
        std::string s = string_printf("[ Integrator aggregate: %s lights[%s]: [ ",
            aggregate.to_string(), lights.size());
        for (const auto& l : lights)
            s += string_printf("%s, ", l.to_string());
        s += string_printf("] infiniteLights[%s]: [ ", infinite_lights.size());
        for (const auto& l : infinite_lights)
        {
            s += string_printf("%s, ", l.to_string());
        }
        return s + " ]";
    }

	//TODO(ches) fill this out

	[[nodiscard]]
	SampledSpectrum RandomWalkIntegrator::light_incoming_random_walk(
		RayDifferential ray,
		SampledWavelengths& lambda, Sampler sampler,
		ScratchBuffer& scratch_buffer, int depth) const
	{

		pstd::optional<ShapeIntersection> intersection = intersect(ray);

		if (!intersection)
		{
			SampledSpectrum result{ 0.0f };
			for (Light light : infinite_lights)
			{
				result += light.infinite_light_contribution(ray, lambda);
			}
			return result;
		}

		SurfaceInteraction& surface = intersection->interaction;

		Vec3f outgoing_direction = -ray.direction;
		SampledSpectrum emitted = surface.emitted_radiance(
			outgoing_direction, lambda);

		if (depth == max_depth)
		{
			return emitted;
		}

		BSDF bsdf = surface.get_BSDF(ray, lambda, camera, scratch_buffer,
			sampler);

		if (!bsdf)
		{
			return emitted;
		}

		Point2f u = sampler.get_2D();
		Vec3f wp = sample_uniform_sphere(u);

		SampledSpectrum fcos = bsdf.f(outgoing_direction, wp) *
			absolute_dot(wp, surface.shading.normal);

		if (!fcos)
		{
			return emitted;
		}

		ray = surface.spawn_ray(wp);
		return emitted + fcos * light_incoming_random_walk(ray, lambda,
			sampler, scratch_buffer, depth + 1) /
			(1 / (4 * PI));

	}
#endif
}