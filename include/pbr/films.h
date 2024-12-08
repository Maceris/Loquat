// pbrt is Copyright(c) 1998-2020 Matt Pharr, Wenzel Jakob, and Greg Humphreys.
// The pbrt source code is licensed under the Apache License, Version 2.0.
// SPDX: Apache-2.0

// This file has been modified from the original, original notice is above.

#pragma once

#include <atomic>
#include <map>
#include <string>
#include <thread>
#include <vector>

#include "pbr/bsdf.h"
#include "pbr/base/bxdf.h"
#include "pbr/base/camera.h"
#include "pbr/base/film.h"
#include "pbr/math/transform.h"
#include "pbr/math/vector_math.h"
#include "pbr/util/color.h"
#include "pbr/util/color_space.h"
#include "pbr/util/parallel.h"
#include "pbr/util/pstd.h"
#include "pbr/util/sampling.h"
#include "pbr/util/spectrum.h"

namespace loquat
{
	class PixelSensor
	{
        static PixelSensor* create(const ParameterDictionary& parameters,
            const RGBColorSpace* color_space, Float exposure_time,
            const FileLoc* loc, Allocator allocator);

        static PixelSensor* create_default(Allocator allocator = {});

        PixelSensor(Spectrum r, Spectrum g, Spectrum b, 
            const RGBColorSpace* output_color_space, 
            Spectrum sensor_illumination, Float imaging_ratio, Allocator allocator)
            : r_bar(r, allocator)
            , g_bar(g, allocator)
            , b_bar(b, allocator)
            , imaging_ratio(imaging_ratio)
        {
            // Compute XYZ from camera RGB matrix
            // Compute _rgbCamera_ values for training swatches
            Float rgbCamera[swatch_reflectance_count][3];
            for (int i = 0; i < swatch_reflectance_count; ++i) {
                RGB rgb = project_reflectance<RGB>(swatch_reflectances[i],
                    sensor_illumination, &r_bar, &g_bar, &b_bar);
                for (int c = 0; c < 3; ++c)
                    rgbCamera[i][c] = rgb[c];
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
            Allocator allocator)
            : r_bar(&Spectra::X(), allocator)
            , g_bar(&Spectra::Y(), allocator)
            , b_bar(&Spectra::Z(), allocator)
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
            const SampledWavelengths& lambda) const
        {
            L = safe_divide(L, lambda.PDF());
            return imaging_ratio * RGB(
                (r_bar.sample(lambda) * L).average(),
                (g_bar.sample(lambda) * L).average(),
                (b_bar.sample(lambda) * L).average());
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

	};

	struct FilmBaseParameters
	{

	};

	class FilmBase
	{

	};

	class RGBFilm : public FilmBase
	{

	};

	class GBufferFilm : public FilmBase
	{

	};

	class SpectralFilm : public FilmBase
	{

	};
}
//TODO(ches) fill this out