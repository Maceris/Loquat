// pbrt is Copyright(c) 1998-2020 Matt Pharr, Wenzel Jakob, and Greg Humphreys.
// The pbrt source code is licensed under the Apache License, Version 2.0.
// SPDX: Apache-2.0

// This file has been modified from the original, original notice is above.

#pragma once

#include <string>
#include <vector>

#include "main/loquat.h"
#include "pbr/util/tagged_pointer.h"

#include "pbr/math/rng.h"
#include "pbr/util/pstd.h"
#include "pbr/util/spectrum.h"
#include "pbr/util/tagged_pointer.h"

namespace loquat
{
	
	struct PhaseFunctionSample
	{
		Float probability;
		Vec3f incoming;
		Float pdf;
	};

	class HGPhaseFunction;

	class PhaseFunction : TaggedPointer<HGPhaseFunction>
	{
	public:
		using TaggedPointer::TaggedPointer;

		[[nodiscard]]
		std::string to_string() const;

		LOQUAT_CPU_GPU
		[[nodiscard]]
		inline Float phase(Vec3f outgoing, Vec3f incoming) const;

		LOQUAT_CPU_GPU
		[[nodiscard]]
		inline pstd::optional<PhaseFunctionSample> sample_phase(Vec3f outgoing,
			Point2f sample_2D) const;

		LOQUAT_CPU_GPU
		[[nodiscard]]
		inline Float PDF(Vec3f outgoing, Vec3f incoming) const;
	};


	class HomogeneousMedium;
	class GridMedium;
	class RGBGridMedium;
	class CloudMedium;
	class NanoVDBMedium;

	struct MediumProperties;

	struct RayMajorantSegment {
		Float t_min;
		Float t_max;
		SampledSpectrum sigma_maj;

		[[nodiscard]]
		std::string to_string() const;
	};

	class HomogeneousMajorantIterator;
	class DDAMajorantIterator;

	class RayMajorantIterator : public TaggedPointer<
		HomogeneousMajorantIterator, DDAMajorantIterator>
	{
	public:
		using TaggedPointer::TaggedPointer;

		LOQUAT_CPU_GPU
		pstd::optional<RayMajorantSegment> next();

		[[nodiscard]]
		std::string to_string() const;
	};

	class Medium : public TaggedPointer<HomogeneousMedium, GridMedium, 
		RGBGridMedium, CloudMedium, NanoVDBMedium>
	{
	public:
		using TaggedPointer::TaggedPointer;

		[[nodiscard]]
		std::string to_string() const;

		LOQUAT_CPU_GPU
		bool is_emissive() const;

		LOQUAT_CPU_GPU
		[[nodiscard]]
		MediumProperties sample_point(Point3f p, 
			const SampledWavelengths& wavelengths) const;

		LOQUAT_CPU_GPU
		[[nodiscard]]
		RayMajorantIterator sample_ray(Ray ray, Float t_max,
			const SampledWavelengths& wavelengths,
			ScratchBuffer& buffer) const;
	};

	class MediumInterface
	{
	public:
		[[nodiscard]]
		std::string to_string() const;

		MediumInterface() = default;

		LOQUAT_CPU_GPU
		MediumInterface(Medium medium)
			: inside{ medium }
			, outside{ medium }
		{}

		LOQUAT_CPU_GPU
		MediumInterface(Medium inside, Medium outside)
			: inside{ inside }
			, outside{ outside }
		{}

		LOQUAT_CPU_GPU
		[[nodiscard]]
		bool is_medium_transition() const
		{
			return inside != outside;
		}

		Medium inside;
		Medium outside;
	};
}