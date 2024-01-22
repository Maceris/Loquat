// pbrt is Copyright(c) 1998-2020 Matt Pharr, Wenzel Jakob, and Greg Humphreys.
// The pbrt source code is licensed under the Apache License, Version 2.0.
// SPDX: Apache-2.0

// This file has been modified from the original, original notice is above.

#pragma once

namespace loquat
{
    struct BSSRDFSample;
    struct BSSRDFProbeSegment;
    struct SubsurfaceInteraction;
    struct BSSRDFTable;

	class TabulatedBSSRDF;

    /// <summary>
    /// Bidirectional surface scattering distribution function.
    /// 
    /// http://www.graphics.stanford.edu/papers/bssrdf/bssrdf.pdf
    /// </summary>
    class BSSRDF : public TaggedPointer<TabulatedBSSRDF>
    {
    public:
        // BSSRDF Interface
        using TaggedPointer::TaggedPointer;

        inline std::optional<BSSRDFProbeSegment> sample(Float sample_1D,
            Point2f sample_2D) const noexcept;

        inline BSSRDFSample probe_intersection_to_sample(
            const SubsurfaceInteraction& interaction,
            ScratchBuffer& scratch_buffer) const noexcept;
    };
}