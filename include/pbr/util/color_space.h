// pbrt is Copyright(c) 1998-2020 Matt Pharr, Wenzel Jakob, and Greg Humphreys.
// The pbrt source code is licensed under the Apache License, Version 2.0.
// SPDX: Apache-2.0

// This file has been modified from the original, original notice is above.

#pragma once

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
            return multiply<RGB>(RGB_from_XYZ, xyz);
        }
        LOQUAT_CPU_GPU
        XYZ to_XYZ(RGB rgb) const
        {
            return multiply<XYZ>(XYZ_from_RGB, rgb);
        }

        static const RGBColorSpace* get_named(std::string name);
        static const RGBColorSpace* lookup(Point2f r, Point2f g, Point2f b,
            Point2f w);

    private:
        const RGBToSpectrumTable* rgb_to_spectrum_table;
    };

}
//TODO(ches) finish this