// pbrt is Copyright(c) 1998-2020 Matt Pharr, Wenzel Jakob, and Greg Humphreys.
// The pbrt source code is licensed under the Apache License, Version 2.0.
// SPDX: Apache-2.0

// This file has been modified from the original, original notice is above.

#pragma once

#include "main/loquat.h"

#include "pbr/util/color.h"
#include "pbr/util/spectrum.h"
#include "pbr/math/math.h"
#include "pbr/math/vec.h"

namespace loquat
{
	class RGBColorSpace
	{
    public:
        RGBColorSpace(Point2f r, Point2f g, Point2f b, Spectrum illuminant,
            const RGBToSpectrumTable* rgb_to_spectrum_table,
            Allocator allocator);

        LOQUAT_CPU_GPU
        RGBSigmoidPolynomial to_RGB_coefficients(RGB rgb) const;

        static void init(Allocator allocator);

        Point2f r;
        Point2f g;
        Point2f b;
        Point2f w;
        DenselySampledSpectrum illuminant;
        SquareMatrix<3> XYZ_from_RGB;
        SquareMatrix<3> RGB_from_XYZ;

        static const RGBColorSpace* sRGB, * DCI_P3, * Rec2020, * ACES2065_1;

        LOQUAT_CPU_GPU
        bool operator==(const RGBColorSpace& cs) const
        {
            return (r == cs.r 
                && g == cs.g 
                && b == cs.b 
                && w == cs.w 
                && rgb_to_spectrum_table == cs.rgb_to_spectrum_table);
        }
        LOQUAT_CPU_GPU
        bool operator!=(const RGBColorSpace& cs) const
        {
            return (r != cs.r || g != cs.g || b != cs.b || w != cs.w ||
                rgb_to_spectrum_table != cs.rgb_to_spectrum_table);
        }

        [[nodiscard]]
        std::string to_string() const;

        LOQUAT_CPU_GPU
        RGB luminance_vector() const
        {
            return RGB(XYZ_from_RGB[1][0], XYZ_from_RGB[1][1], XYZ_from_RGB[1][2]);
        }

        LOQUAT_CPU_GPU
        RGB to_RGB(XYZ xyz) const
        {
            return mul<RGB>(RGB_from_XYZ, xyz);
        }
        LOQUAT_CPU_GPU
        XYZ to_XYZ(RGB rgb) const
        {
            return mul<XYZ>(XYZ_from_RGB, rgb);
        }

        static const RGBColorSpace* get_named(std::string name);
        static const RGBColorSpace* lookup(Point2f r, Point2f g, Point2f b,
            Point2f w);

    private:
        const RGBToSpectrumTable* rgb_to_spectrum_table;
    };

#ifdef LOQUAT_BUILD_GPU_RENDERER
    extern LOQUAT_CONST RGBColorSpace* RGBColorSpace_sRGB;
    extern LOQUAT_CONST RGBColorSpace* RGBColorSpace_DCI_P3;
    extern LOQUAT_CONST RGBColorSpace* RGBColorSpace_Rec2020;
    extern LOQUAT_CONST RGBColorSpace* RGBColorSpace_ACES2065_1;
#endif

    SquareMatrix<3> convert_RGB_color_space(const RGBColorSpace& from,
        const RGBColorSpace& to);
}
